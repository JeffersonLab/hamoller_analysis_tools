#!/bin/bash
# --------------------------------------------------------
# Query EPICS PVs and write PV name + value to output file
# Usage: ./query_epics_run.sh <run_number>
# --------------------------------------------------------

# Check for run number argument
if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <run_number>"
    exit 1
fi

RUN_NUMBER=$1
INPUT_FILE="epics/epics_pv_list.txt"
OUTPUT_FILE="${HAMOLLER_EPICS_DIR}/hamolpol_${RUN_NUMBER}.epics"

# Empty output file
> "$OUTPUT_FILE"

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

    # Write PV and value to output file
    echo -e "$PV\t$VALUE" >> "$OUTPUT_FILE"

done < "$INPUT_FILE"

echo "Done. Results saved in $OUTPUT_FILE"
