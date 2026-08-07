#!/bin/bash

# Usage: ./start_of_run.sh <run_number>
# --------------------------------------------------------

# Check for run number argument
if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <run_number>"
    exit 1
fi
RUN_NUMBER=$1

# Take a snapshot of the configuration file and readout
# list file at the beginning of the run.
# --------------------------------------------------------
scp hapolmollervme:vme/cfg/hapolmoller.cfg ${HAMOLLER_RUNLOG_DIR}/hapolmoller_${RUN_NUMBER}.cfg
scp hapolmollervme:vme/rol/.moller_readout.c ${HAMOLLER_RUNLOG_DIR}/moller_readout_${RUN_NUMBER}.c
#Change permissions to prevent accidental overwriting
chmod 444 ${HAMOLLER_RUNLOG_DIR}/hapolmoller_${RUN_NUMBER}.cfg
chmod 444 ${HAMOLLER_RUNLOG_DIR}/moller_readout_${RUN_NUMBER}.c

# Query EPICS PVs and write PV name + value to output file
# --------------------------------------------------------

INPUT_FILE="epics/epics_pv_list.txt"
OUTPUT_FILE="${HAMOLLER_EPICS_DIR}/hamolpol_${RUN_NUMBER}.epics"

# Empty output file
if ! date +"Start of run time: %a %b %e %r %Z %Y" > "$OUTPUT_FILE" 2>/dev/null; then
    echo "Cannot open output file ${OUTPUT_FILE}. Permission denied." >&2
    exit 1
fi

# Loop over each PV (assumes first column in the file is PV)
while read -r line; do
    # Skip empty lines
    [[ -z "$line" ]] && continue

    # Get the PV name (first column)
    PV=$(echo "$line" | awk '{print $1}')

    # Query the PV with a 2-second timeout
    VALUE=$(caget -t "$PV" 2>/dev/null)
    
    # Check if caget failed
    if [[ $? -ne 0 || -z "$VALUE" ]]; then
        VALUE="timeout"
    fi

    # Check for specific Hall pass PVs and extract floating-point number
    case "$PV" in
        *MMSHLAPASS) pass_var="HallA_Npass" ;;
        *MMSHLBPASS) pass_var="HallB_Npass" ;;
        *MMSHLCPASS) pass_var="HallC_Npass" ;;
        *MMSHLDPASS) pass_var="HallD_Npass" ;;
        *) pass_var="" ;;
    esac

    # Write PV and value to output file
    echo -e "$PV\t$VALUE" >> "$OUTPUT_FILE"

    if [[ -n "$pass_var" ]]; then
        # Match any number sequence, optionally with a decimal point (e.g., "1", "5", "5.5")
        if [[ "$VALUE" =~ ([0-9]+(\.[0-9]+)?) ]]; then
            declare -g "$pass_var=${BASH_REMATCH[1]}"
        else
            declare -g "$pass_var=0"
        fi
        # Optional: Print to screen/log to verify extraction works
        echo -e "$pass_var = ${!pass_var}" >> "$OUTPUT_FILE"
    fi

done < "$INPUT_FILE"

#Change permissions so file never mistakenly overwritten
chmod 444 "$OUTPUT_FILE"

echo "Done. Results saved in $OUTPUT_FILE"
