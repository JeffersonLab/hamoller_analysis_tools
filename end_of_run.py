#!/usr/bin/env python3

import sys
import os
import glob
import re
import time
from datetime import datetime
from zoneinfo import ZoneInfo
import mysql.connector

def usage():
    print(f"Usage: {sys.argv[0]} <run_number>")
    sys.exit(1)


def variable_name(name):
    """
    Convert:

        FADC250_ALLCH_TET -> fadc_allch_tet
        FADC250_GAIN      -> fadc_gain
    """
    return name.replace("FADC250_", "FADC_", 1).lower()


def convert_value(value):
    """
    Convert a string to int or float.
    """
    try:
        if any(c in value.lower() for c in (".", "e")):
            return float(value)
        else:
            return int(value)
    except ValueError:
        return None


def parse_evio(filename):
    values = {}

    with open(filename, "rb") as f:

        # Only the first 33 lines contain the configuration text.
        for line_number in range(33):

            line = f.readline()

            if not line:
                break

            # Decode only this line.
            # Ignore bytes that aren't valid ASCII.
            text = line.decode("ascii", errors="ignore").strip()

            if not text:
                continue

            # We only care about FADC250_ variables.
            if not text.startswith("FADC250_"):
                continue

            parts = text.split()

            if len(parts) < 2:
                continue

            name = parts[0]

            # Convert variable name to desired Python dictionary name.
            key = variable_name(name)

            # ---------------------------------------------------------
            # ALLCH variables
            # ---------------------------------------------------------
            if name.startswith("FADC250_ALLCH_"):

                converted = []

                for value in parts[1:]:
                    numeric_value = convert_value(value)

                    if numeric_value is not None:
                        converted.append(numeric_value)

                if len(converted) != 16:
                    print(
                        f"Warning: {name} has {len(converted)} values "
                        f"instead of 16",
                        file=sys.stderr
                    )

                if not converted:
                    continue

                # -----------------------------------------------------
                # Create MariaDB VARCHAR(255) string.
                #
                # str() gives:
                #   20
                #   1.0
                #   272.538
                #
                # and join() gives:
                #   "20, 20, 20, 20, ..."
                # -----------------------------------------------------
                values[key] = ", ".join(str(v) for v in converted)

                # -----------------------------------------------------
                # Create child variable by removing "allch_".
                #
                # fadc_allch_tet -> fadc_tet
                # -----------------------------------------------------
                child_key = key.replace("fadc_allch_", "fadc_", 1)

                # If all 16 values are identical, use that value.
                # Otherwise use -1.
                if len(converted) == 16 and all(
                    v == converted[0] for v in converted
                ):
                    values[child_key] = converted[0]
                else:
                    values[child_key] = -1

            # ---------------------------------------------------------
            # MOLLER variable used to store trigger information
            # ---------------------------------------------------------
            elif name.startswith("FADC250_MOLLER"):

                converted = []
                print("Found MOLLER")
                for value in parts[1:]:
                    numeric_value = convert_value(value)

                    if numeric_value is not None:
                        converted.append(numeric_value)

                if len(converted) != 6:
                    print(
                        f"Warning: {name} has {len(converted)} values "
                        f"instead of 6",
                        file=sys.stderr
                    )

                if not converted:
                    continue
                values["fadc_l_offset"]   = converted[0]
                values["fadc_r_offset"]   = converted[1]
                values["fadc_l_sum_thr"]  = converted[2]
                values["fadc_r_sum_thr"]  = converted[3]
                values["fadc_disc_width"] = converted[4]
                values["fadc_disc_mode"]  = converted[5]


            # ---------------------------------------------------------
            # Ordinary single-value variable
            # ---------------------------------------------------------
            else:

                numeric_value = convert_value(parts[1])

                if numeric_value is None:
                    continue

                values[key] = numeric_value

    return values

# ============================================================
# Define your database connection credentials
# ============================================================

db_config = {
    "host": "halladb",
    "user": "hamoller_user",
    "password": "4YPHMLr7KY",
    "database": "hamoller_db"
}

# ============================================================
# Configuration
# ============================================================

# Log timestamps are assumed to be Eastern time.
# Change to ZoneInfo("UTC") if the log timestamps are UTC.
LOG_TIMEZONE = ZoneInfo("America/New_York")

TIMESTAMP_FORMAT = "%d%b%Y %H:%M:%S"

TRIG_NAMES = [
    "ps1",
    "ps2",
    "ps3",
    "ps4",
    "ps5",
    "ps6",
]

# ============================================================
# Helper function to dynamically generate and execute
# parameterized INSERT statements
# ============================================================

def insert_dict_to_table(cursor, table_name, data_dict):
    # Extract column names and create the placeholder string (%s, %s, ...)
    columns = ", ".join(data_dict.keys())
    placeholders = ", ".join(["%s"] * len(data_dict))
    
    # Construct the final SQL string
    sql = f"INSERT INTO {table_name} ({columns}) VALUES ({placeholders})"
    
    # Execute using the dictionary's values as tuple arguments
    cursor.execute(sql, tuple(data_dict.values()))


# ============================================================
# Timestamp handling
# ============================================================

def unix_time(timestamp_string):
    """
    Convert a timestamp such as:

        17Aug2026 09:39:00

    to Unix time.
    """

    dt = datetime.strptime(
        timestamp_string,
        TIMESTAMP_FORMAT
    )

    dt = dt.replace(tzinfo=LOG_TIMEZONE)

    return int(dt.timestamp())


def timestamp_before(text, phrase):
    """
    Find the timestamp associated with a line containing
    the requested phrase.

    Example:

        17Aug2026 09:39:00: sms_production INFO:  Go succeeded.

    Returns the Unix timestamp corresponding to
    17Aug2026 09:39:00.
    """

    pattern = re.compile(
        r"(\d{2}[A-Za-z]{3}\d{4} \d{2}:\d{2}:\d{2}):"
        r"[^\n]*?"
        + re.escape(phrase)
    )

    match = pattern.search(text)

    if not match:
        raise RuntimeError(
            f'Could not find timestamp associated with "{phrase}"'
        )

    return unix_time(match.group(1))


# ============================================================
# Trigger prescales
# ============================================================

def get_trigger_prescales(text):
    """
    Find the ps1..ps6 values.

    Expected format is something like:

        ps1=0,ps2=-1,ps3=-1,ps4=-1,ps5=-1,ps6=0

    The values may contain whitespace.
    """

    # Find the first line containing ps assignments.
    for line in text.splitlines():

        if "ps1" not in line:
            continue

        if not any(name in line for name in TRIG_NAMES):
            continue

        result = {}

        for name in TRIG_NAMES:

            match = re.search(
                rf"\b{name}\s*=\s*(-?\d+)",
                line
            )

            if match:
                result[name] = int(match.group(1))

        if len(result) == 6:
            return result

    raise RuntimeError(
        "Could not find ps1 through ps6 trigger prescales"
    )


# ============================================================
# Block and buffer levels
# ============================================================

def get_levels(text):
    """
    Find:

        Block Level        = 25
        Block Buffer Level = 20 -Broadcast- (Busy Enabled)
    """

    match = re.search(
        r"Block Level\s*=\s*(-?\d+)",
        text,
        re.IGNORECASE
    )

    if not match:
        raise RuntimeError(
            "Could not find Block Level"
        )

    block_level = int(match.group(1))

    match = re.search(
        r"Block Buffer Level\s*=\s*(-?\d+)",
        text,
        re.IGNORECASE
    )

    if not match:
        raise RuntimeError(
            "Could not find Block Buffer Level"
        )

    buffer_level = int(match.group(1))

    return block_level, buffer_level



# ============================================================
# Find the log file
# ============================================================

def find_log_file(run_number):
    """
    Find:

        ${CODA_LOGS}/<run_number>*.log

    Exactly one matching file is required.
    """

    coda_logs = os.environ.get("CODA_LOGS")

    if not coda_logs:
        raise RuntimeError(
            "CODA_LOGS environment variable is not set"
        )

    pattern = os.path.join(
        coda_logs,
        f"{run_number}*.log"
    )

    matches = glob.glob(pattern)

    if not matches:
        raise RuntimeError(
            f"No log file found matching:\n  {pattern}"
        )

    if len(matches) > 1:

        message = (
            f"Multiple log files found for run {run_number}:\n"
        )

        for filename in matches:
            message += f"  {filename}\n"

        raise RuntimeError(message)

    return matches[0]


# ============================================================
# Verify run number
# ============================================================

def verify_run_number(text, run_number):
    """
    Verify that the selected log actually contains the
    requested run number.

    Looks for something such as:

        prestarting,run 557, type 1
    """

    pattern = re.compile(
        rf"\brun\s+{re.escape(run_number)}\b"
    )

    if not pattern.search(text):
        raise RuntimeError(
            f"Log file does not appear to contain run "
            f"{run_number}"
        )


# ============================================================
# Main
# ============================================================

def main():

    # --------------------------------------------------------
    # Command-line argument
    # --------------------------------------------------------

    if len(sys.argv) != 2:
        usage()

    run_number = sys.argv[1]

    # --------------------------------------------------------
    # Locate log file
    # --------------------------------------------------------

    try:
        filename = find_log_file(run_number)

    except Exception as error:

        print(
            f"ERROR: {error}",
            file=sys.stderr
        )

        sys.exit(1)

    print(
        f"Reading log: {filename}",
        file=sys.stderr
    )

    # --------------------------------------------------------
    # Read log
    # --------------------------------------------------------

    try:

        with open(
            filename,
            "r",
            encoding="utf-8",
            errors="replace"
        ) as logfile:

            text = logfile.read()

    except Exception as error:

        print(
            f"ERROR reading log file: {error}",
            file=sys.stderr
        )

        sys.exit(1)

    # --------------------------------------------------------
    # Verify run number
    # --------------------------------------------------------

    try:

        verify_run_number(
            text,
            run_number
        )

    except Exception as error:

        print(
            f"ERROR: {error}",
            file=sys.stderr
        )

        sys.exit(1)

    # --------------------------------------------------------
    # Extract values
    # --------------------------------------------------------

    try:

        # Trigger prescales
        trig = get_trigger_prescales(text)

        # Run start/end
        text = text.replace("StartRun", "Go")
        run_start_unix = timestamp_before(
            text,
            "Go succeeded"
        )
        run_start = time.ctime(run_start_unix)

        run_end_unix = timestamp_before(
            text,
            "End Executed"
        )
        run_end = time.ctime(run_end_unix)

        # TI levels
        block_level, buffer_level = get_levels(text)

        # FADC configuration
    except Exception as error:

        print(
            f"ERROR parsing log: {error}",
            file=sys.stderr
        )

        sys.exit(1)
    evio_dir = os.environ.get("HAMOLLER_EVIO_DIR")

    if not evio_dir:
        print(
            "Error: HAMOLLER_EVIO_DIR is not defined",
            file=sys.stderr
        )
        sys.exit(1)

    # Look for:
    #
    #   <anything><run_number>.evio.0
    #
    pattern = os.path.join(
        evio_dir,
        f"*{run_number}.evio.0"
    )

    matches = glob.glob(pattern)

    if not matches:
        print(
            f"Error: no EVIO file found matching {pattern}",
            file=sys.stderr
        )
        sys.exit(1)

    if len(matches) > 1:
        print(
            f"Warning: multiple files matched; using {matches[0]}",
            file=sys.stderr
        )

    filename = matches[0]

    print(f"Reading: {filename}", file=sys.stderr)

    values = parse_evio(filename)

    wanted = ["fadc_allch_mode",
              "fadc_allch_w_offset",
              "fadc_allch_w_width",
              "fadc_allch_nsa",
              "fadc_allch_nsb",
              "fadc_allch_npeak",
              "fadc_allch_nped",
              "fadc_allch_maxped",
              "fadc_allch_nsat",
              "fadc_allch_dac",
              "fadc_allch_tet",
              "fadc_allch_ped",
              "fadc_allch_gain",
              "fadc_mode",
              "fadc_w_offset",
              "fadc_w_width",
              "fadc_nsa",
              "fadc_nsb",
              "fadc_npeak",
              "fadc_nped",
              "fadc_maxped",
              "fadc_nsat",
              "fadc_dac",
              "fadc_tet",
              "fadc_ped",
              "fadc_gain",
              "fadc_trg_sel",
              "fadc_trg_width",
              "fadc_l_offset",
              "fadc_r_offset",
              "fadc_l_sum_thr",
              "fadc_r_sum_thr",
              "fadc_disc_width",
              "fadc_disc_mode"]
              
    fadc = {key:values[key] for key in wanted if key in values}
    
    # --------------------------------------------------------
    # Create SQL variables
    # --------------------------------------------------------

    variables = {

        "trig_ps1": trig["ps1"],
        "trig_ps2": trig["ps2"],
        "trig_ps3": trig["ps3"],
        "trig_ps4": trig["ps4"],
        "trig_ps5": trig["ps5"],
        "trig_ps6": trig["ps6"],

        "run_start_datetime": run_start,
        "run_start_unix": run_start_unix,
        "run_end_datetime": run_end,
        "run_end_unix": run_end_unix,

        "block_level": block_level,
        "buffer_level": buffer_level,
        **fadc
    }

    # --------------------------------------------------------
    # Output
    # --------------------------------------------------------

    for name, value in variables.items():
        print(f"{name}={value}")

    daq_data = {"run_number": run_number}
    run_data = {"run_number": run_number}

    for key, val in variables.items():
        if key.startswith("fadc_") or key.count("level")>0:
            daq_data[key] = val
        else:
            run_data[key] = val

        # Connect to the database and insert data
    try:
        connection = mysql.connector.connect(**db_config)
        cursor = connection.cursor()
    
        # Insert rows into DAQ_info and Run_info tables
        insert_dict_to_table(cursor, "Run_info", run_data)
        insert_dict_to_table(cursor, "DAQ_config", daq_data)
    
        # Commit the transaction to save changes permanently
        connection.commit()
        print(f"Successfully inserted data for run {run_number} into both tables.")

    except mysql.connector.Error as err:
        print(f"Database error occurred: {err}")
        if 'connection' in locals() and connection.is_connected():
            connection.rollback() # Roll back changes if any insert fails

    finally:
        # Safely close database assets
        if 'cursor' in locals():
            cursor.close()
        if 'connection' in locals() and connection.is_connected():
            connection.close()        

# ============================================================
# Entry point
# ============================================================

if __name__ == "__main__":
    main()

