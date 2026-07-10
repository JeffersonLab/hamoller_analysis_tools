#!/bin/bash

# Check arguments
if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 start_run [end_run]"
    exit 1
fi

start=$1
end=$2

# If only one argument → run single run
if [[ -z "$end" ]]; then
    echo "Processing run $start"
    root -l -b -q "buildLeftRightSumTree.C($start)"
else
    # Loop over range
    for (( run=start; run<=end; run++ )); do
	if ls "$HAMOLLER_ROOTFILE_DIR"/fadcV2_moller_analyzer_"$run"*.root 1>/dev/null 2>&1; then
            echo "Processing run $run"
            root -l -b -q "buildLeftRightSumTree.C($run)"
	else
	    echo "File not found for run $run"
	fi	
    done
fi
