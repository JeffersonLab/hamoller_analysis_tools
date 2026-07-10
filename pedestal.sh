#!/bin/bash

DIR=${HAMOLLER_ROOTFILE_DIR}

if [ -z "$DIR" ]; then
    echo "HAMOLLER_ROOTFILE_DIR not set!"
    exit 1
fi

# -------------------------------
# MODE 1: pedestal.sh start end
# -------------------------------
if [ $# -eq 2 ]; then
    START=$1
    END=$2

    echo "Processing runs from $START to $END"

    for ((run=START; run<=END; run++)); do
        # Check if any file exists for this run
        files=(${DIR}/fadcV2_moller_analyzer_${run}*.root)

        if [ ! -e "${files[0]}" ]; then
            #echo "Run $run: no files found, skipping"
            continue
        fi

        echo "Run $run: processing"
        #root -l -b -q "pedestal.C($run)"
    done

    echo "Done."
    exit 0
fi

# ----------------------------------------
# MODE 2: pedestal.sh   (auto-discover runs)
# ----------------------------------------
if [ $# -eq 0 ]; then
    echo "Scanning directory for run numbers..."

    declare -A runs

    for file in "$DIR"/*.root; do
        fname=$(basename "$file")
	# Only keep files with correct prefix
	[[ $fname == fadcV2_moller_analyzer_* ]] || continue

	base="${fname%.root}"
	run="${base#fadcV2_moller_analyzer_}"
	run="${run%%_*}"
	if [ ! -z "$run" ]; then
            runs[$run]=1
        fi
	#echo "File $fname"
	#echo $run

    done

    for run in "${!runs[@]}"; do
        echo "Run $run: processing"
        root -l -b -q "pedestal.C($run)"
    done

    echo "Done."
    exit 0
fi

# -------------------------------
# Invalid usage
# -------------------------------
echo "Usage:"
echo "  $0 start end     # process range of runs"
echo "  $0               # auto-detect runs"
exit 1
