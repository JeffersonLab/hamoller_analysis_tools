#!/usr/bin/env python3

import sys
import os
import glob
import re
import time
from datetime import datetime
from zoneinfo import ZoneInfo
import mysql.connector

#Define your database connection credentials
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
# FADC table
# ============================================================

def get_fadc_table(text):
    """
    Extract the first complete 16-channel FADC configuration table.

    Table format:

       Ch| MODE | PL(ns) | PTW(ns) | NSB(ns) | NSA(ns) |
          NP | NPED | MAXPED | NSAT |

    Channel number (Ch) is not stored.

    The remaining columns are mapped to:

        MODE   -> fadc_allch_mode
        PL     -> fadc_allch_w_offset
        PTW    -> fadc_allch_w_width
        NSB    -> fadc_allch_nsb
        NSA    -> fadc_allch_nsa
        NP     -> fadc_allch_npeak
        NPED   -> fadc_allch_nped
        MAXPED -> fadc_allch_maxped
        NSAT   -> fadc_allch_nsat

    The logging system sometimes inserts a timestamp in the
    middle of a table row. For example:

        11| 9 | 808 |     24
    17Aug2026 09:39:47: ROC1 INFO: 0 | 12 | 40 | 4 ...

    This is actually:

        11| 9 | 808 | 240 | 12 | 40 | 4 ...

    The timestamp insertion is removed before parsing.
    """
    # --------------------------------------------------------
    # Locate the FADC table header.
    # --------------------------------------------------------

    header_re = re.compile(
        r"Ch\|\s*MODE\s*\|\s*PL\(ns\)\s*\|\s*PTW\(ns\)"
        r"\s*\|\s*NSB\(ns\)\s*\|\s*NSA\(ns\)"
        r"\s*\|\s*NP\s*\|\s*NPED\s*\|\s*MAXPED\s*\|\s*NSAT",
        re.IGNORECASE
    )

    header_match = header_re.search(text)

    # In case a timestamp in the middle of the header causes trouble
    # check for a match on the first half of the header...
    if not header_match:
        header_re = re.compile(
            r"Ch\|\s*MODE\s*\|\s*PL\(ns\)\s*\|\s*PTW\(ns\)"
            r"\s*\|\s*NSB\(ns\)\s*\|\s*NSA\(ns\)",
            re.IGNORECASE
        )
    header_match = header_re.search(text)
    # and if it still fails check for the other half of the header.
    # It can't miss both just due to a time stamp insertion.
    if not header_match:
        header_re = re.compile(
            r"\s*\|\s*NP\s*\|\s*NPED\s*\|\s*MAXPED\s*\|\s*NSAT",
            re.IGNORECASE
        )
    header_match = header_re.search(text)
        
    if not header_match:
        raise RuntimeError(
            "Could not find FADC table 1 header"
        )

    # Work on the text immediately following the header.
    table_section = text[
        header_match.end():
        header_match.end() + 20000
    ]

    # --------------------------------------------------------
    # Remove timestamp insertions.
    #
    # Specifically remove the newline followed by:
    #
    #   17Aug2026 09:39:47: ROC1 INFO:
    #
    # This joins the broken pieces of a table row back together.
    # --------------------------------------------------------

    table_section = re.sub(
        r"\n[ \t]*"
        r"\d{2}[A-Za-z]{3}\d{4} "
        r"\d{2}:\d{2}:\d{2}:"
        r"[^\n]*?INFO:\s*",
        "",
        table_section
    )

    # Handle a timestamp occurring at the very beginning.
    table_section = re.sub(
        r"^\s*"
        r"\d{2}[A-Za-z]{3}\d{4} "
        r"\d{2}:\d{2}:\d{2}:"
        r"[^\n]*?INFO:\s*",
        "",
        table_section
    )


    # --------------------------------------------------------
    # Find table rows.
    #
    # A valid row has:
    #
    #   channel
    #   MODE
    #   PL
    #   PTW
    #   NSB
    #   NSA
    #   NP
    #   NPED
    #   MAXPED
    #   NSAT
    #
    # i.e. 10 fields.
    # --------------------------------------------------------

    row_re = re.compile(
        r"^\s*(\d+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|",
        re.MULTILINE
    )

    rows = {}

    for match in row_re.finditer(table_section):

        channel = int(match.group(1))

        # Only accept channels 0 through 15.
        if channel < 0 or channel > 15:
            continue

        values = [
            match.group(i).strip()
            for i in range(2, 11)
        ]

        rows[channel] = values

        if len(rows) == 16:
            break

    # --------------------------------------------------------
    # Verify that every channel was found.
    # --------------------------------------------------------

    missing = [
        channel
        for channel in range(16)
        if channel not in rows
    ]

    if missing:
        raise RuntimeError(
            "FADC table is incomplete. "
            f"Missing channels: {missing}"
        )

    # --------------------------------------------------------
    # Map table columns to SQL variables.
    #
    # values[] contains:
    #
    #   0 = MODE
    #   1 = PL
    #   2 = PTW
    #   3 = NSB
    #   4 = NSA
    #   5 = NP
    #   6 = NPED
    #   7 = MAXPED
    #   8 = NSAT
    # --------------------------------------------------------

    columns = {
        "fadc_allch_mode":      0,
        "fadc_allch_w_offset":  1,
        "fadc_allch_w_width":   2,
        "fadc_allch_nsb":       3,
        "fadc_allch_nsa":       4,
        "fadc_allch_npeak":     5,
        "fadc_allch_nped":      6,
        "fadc_allch_maxped":    7,
        "fadc_allch_nsat":      8,
    }

    result = {}
    for sql_name, column_number in columns.items():
        result[sql_name] = ",".join(
            rows[channel][column_number]
            for channel in range(16)
        )

    return result
# ============================================================
# FADC table 2
# ============================================================

def get_fadc_table2(text):
    """
    Extract the second complete 16-channel FADC configuration table.

    Table format:

       Ch| Readout - TET | Trigger - TET | GAIN   | PED     |DELAY|TRGMODE|INVERT|PLAYBACK|

    Channel number (Ch) is not stored.

    The remaining columns are mapped to:

        Readout - TET   -> fadc_allch_TET
        Trigger - TET      not recorded
        GAIN            -> fadc_allch_gain
        PED             -> fadc_allch_ped
        DELAY              not recorded
        TRGMODE            not recorded
        INVERT             not recorded
        PLAYBACK           not recorded

    The logging system sometimes inserts a timestamp in the
    middle of a table row. For example:

   7|            30 |            30 |  1.000 | 308.538 |   0 | PULSE |   0  |DISA
13Aug2026 09:37:03: ROC1 INFO: BLED

    This is actually:

   7|            30 |            30 |  1.000 | 308.538 |   0 | PULSE |   0  |DISABLED


    The timestamp insertion is removed before parsing.
    """

    # --------------------------------------------------------
    # Locate the FADC table header.
    # --------------------------------------------------------

    header_re = re.compile(
        r"Ch\|\s*Readout - TET\s*\|\s*Trigger - TET\s*\|\s*GAIN"
        r"\s*\|\s*PED\s*\|\s*DELAY"
        r"\s*\|\s*TRGMODE\s*\|\s*INVERT\s*\|\s*PLAYBACK",
        re.IGNORECASE
    )

    header_match = header_re.search(text)

    # In case a timestamp in the middle of the header causes trouble
    # check for a match on the first half of the header...
    if not header_match:
        header_re = re.compile(
            r"Ch\|\s*Readout - TET\s*\|\s*Trigger - TET\s*\|\s*GAIN"
            r"\s*\|\s*PED\s*\|\s*DELAY",
            re.IGNORECASE
        )
    header_match = header_re.search(text)
    # and if it still fails check for the other half of the header.
    # It can't miss both just due to a time stamp insertion.
    if not header_match:
        header_re = re.compile(
        r"\s*\|\s*TRGMODE\s*\|\s*INVERT\s*\|\s*PLAYBACK",
            re.IGNORECASE
        )
    header_match = header_re.search(text)

    if not header_match:
        raise RuntimeError(
            "Could not find FADC table 2 header"
        )

    # Work on the text immediately following the header.
    table_section = text[
        header_match.end():
        header_match.end() + 20000
    ]

    # --------------------------------------------------------
    # Remove timestamp insertions.
    #
    # Specifically remove the newline followed by:
    #
    #   17Aug2026 09:39:47: ROC1 INFO:
    #
    # This joins the broken pieces of a table row back together.
    # --------------------------------------------------------

    table_section = re.sub(
        r"\n[ \t]*"
        r"\d{2}[A-Za-z]{3}\d{4} "
        r"\d{2}:\d{2}:\d{2}:"
        r"[^\n]*?INFO:\s*",
        "",
        table_section
    )

    # Handle a timestamp occurring at the very beginning.
    table_section = re.sub(
        r"^\s*"
        r"\d{2}[A-Za-z]{3}\d{4} "
        r"\d{2}:\d{2}:\d{2}:"
        r"[^\n]*?INFO:\s*",
        "",
        table_section
    )

    # --------------------------------------------------------
    # Find table rows.
    #
    # A valid row has:
    #
    #   channel
    #   Readout - TET
    #   Trigger - TET
    #   GAIN
    #   PED
    #   DELAY
    #   TRGMODE
    #   INVERT
    #   PLAYBACK
    #
    # i.e. 9 fields.
    # --------------------------------------------------------

    row_re = re.compile(
        r"^\s*(\d+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|"
        r"\s*([^|]+)\|",
        re.MULTILINE
    )

    rows = {}

    for match in row_re.finditer(table_section):

        channel = int(match.group(1))

        # Only accept channels 0 through 15.
        if channel < 0 or channel > 15:
            continue

        values = [
            match.group(i).strip()
            for i in [1,3,4]
        ]

        rows[channel] = values

        if len(rows) == 16:
            break

    # --------------------------------------------------------
    # Verify that every channel was found.
    # --------------------------------------------------------

    missing = [
        channel
        for channel in range(16)
        if channel not in rows
    ]

    if missing:
        raise RuntimeError(
            "FADC table is incomplete. "
            f"Missing channels: {missing}"
        )

    # --------------------------------------------------------
    # Map table columns to SQL variables.
    #
    # values[] contains:
    #
    #   0 = Readout - TET
    #   1 = GAIN
    #   2 = PED
    #   3 = DELAY
    #   4 = TRGMODE
    # --------------------------------------------------------

    columns = {
        "fadc_allch_tet":      0,
        "fadc_allch_gain":     1,
        "fadc_allch_ped":      2,
    }

    result = {}
    for sql_name, column_number in columns.items():
        result[sql_name] = ",".join(
            rows[channel][column_number]
            for channel in range(16)
        )

    return result


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

        print(
            f"Usage: {sys.argv[0]} <run_number>",
            file=sys.stderr
        )

        sys.exit(1)

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
        fadc = get_fadc_table(text)
        fadc2 = get_fadc_table2(text)
        fadc_ch = {}
        for key, value_string in fadc.items():
            # 1. Generate the new key by removing "allch_"
            new_key = key.replace("allch_", "")
    
            # 2. Split the string by commas and convert each item to a float
            float_list = [float(x.strip()) for x in value_string.split(",")]
    
            # 3. Check if all items in the list match the very first item
            if all(val == float_list[0] for val in float_list):
                fadc_ch[new_key] = float_list[0]
            else:
                fadc_ch[new_key] = -1.0
        fadc2_ch = {}
        for key, value_string in fadc2.items():
            # 1. Generate the new key by removing "allch_"
            new_key = key.replace("allch_", "")
    
            # 2. Split the string by commas and convert each item to a float
            float_list = [float(x.strip()) for x in value_string.split(",")]
    
            # 3. Check if all items in the list match the very first item
            if all(val == float_list[0] for val in float_list):
                fadc2_ch[new_key] = float_list[0]
            else:
                fadc2_ch[new_key] = -1.0
    except Exception as error:

        print(
            f"ERROR parsing log: {error}",
            file=sys.stderr
        )

        sys.exit(1)

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

        **fadc,
        **fadc2,
        **fadc_ch,
        **fadc2_ch
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

