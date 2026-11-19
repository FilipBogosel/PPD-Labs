#!/usr/bin/env bash
# Runs the Lab 4 Producer-Consumer benchmark suite.
# Executes the sequential implementation and parallel implementations
# (p = 4, 8, 16 with p_r = 2 and p_r = 3), each RUNS times (default 10).
# Verifies Results.txt against the sequential baseline on every run,
# records CSV timings in results/, calls compute_averages, and prints T_seq / T_par.
#
# Usage (from anywhere):
#   ./run_tests.sh                         -> uses Lab4/solution/test_data
#   ./run_tests.sh /path/to/test_data      -> uses the given data folder
#   RUNS=10 ./run_tests.sh                 -> override number of runs

set -u

export PATH="$HOME/.local/bin:$PATH"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUNS="${RUNS:-10}"

RAW_DATA_DIR="${1:-$SCRIPT_DIR/solution/test_data}"
DATA_DIR="${RAW_DATA_DIR%/}"
DATASET_NAME="$(basename "$DATA_DIR")"

SRC_DIR="$SCRIPT_DIR/solution/Prod-Cons/src"
BIN_DIR="$SCRIPT_DIR/bin"
RESULTS_DIR="$SCRIPT_DIR/results"
OUTPUT_FILE="$SCRIPT_DIR/Results.txt"
EXPECTED_FILE="$RESULTS_DIR/expected_Results.txt"

THREAD_COUNTS="4 8 16"
READER_COUNTS="2 3"

# Check / generate dataset
has_project_files() {
    local dir=$1
    local f
    for f in "$dir"/project*.txt; do
        [ -e "$f" ] && return 0
    done
    return 1
}

if [ ! -d "$DATA_DIR" ] || ! has_project_files "$DATA_DIR"; then
    if [ "$DATA_DIR" == "$SCRIPT_DIR/solution/test_data" ]; then
        echo "Test data not found in $DATA_DIR. Generating inputs..."
        mkdir -p "$DATA_DIR"
        (cd "$SCRIPT_DIR/solution" && python3 generate_data.py) || {
            echo "Error: Failed to generate test data."
            exit 1
        }
    else
        echo "Error: Test data folder missing or contains no project*.txt files: $DATA_DIR"
        exit 1
    fi
fi

# Compile sources into Lab4/bin (keeps Prod-Cons/out untouched)
echo "Compiling Java sources into $BIN_DIR..."
mkdir -p "$BIN_DIR" "$RESULTS_DIR"
javac -d "$BIN_DIR" "$SRC_DIR"/*.java || {
    echo "Error: Compilation failed."
    exit 1
}
echo "Compilation successful."

# Clean previous CSVs for this dataset
SEQ_CSV="$RESULTS_DIR/sequential_${DATASET_NAME}.csv"
PR2_CSV="$RESULTS_DIR/parallel_pr2_${DATASET_NAME}.csv"
PR3_CSV="$RESULTS_DIR/parallel_pr3_${DATASET_NAME}.csv"
rm -f "$SEQ_CSV" "$PR2_CSV" "$PR3_CSV" "$EXPECTED_FILE" "$OUTPUT_FILE"

# run_with_timeout <seconds> <command...>
run_with_timeout() {
    local timeout_sec=$1
    shift
    python3 -c '
import subprocess, sys
timeout_sec = float(sys.argv[1])
cmd = sys.argv[2:]
try:
    res = subprocess.run(cmd, timeout=timeout_sec, capture_output=True, text=True)
    if res.stdout:
        print(res.stdout, end="")
    if res.stderr:
        print(res.stderr, end="", file=sys.stderr)
    sys.exit(res.returncode)
except subprocess.TimeoutExpired:
    print(f"Timed out after {timeout_sec}s (possible deadlock): {cmd}", file=sys.stderr)
    sys.exit(124)
' "$timeout_sec" "$@"
}

# Reference sequential baseline
echo "Generating reference baseline ($EXPECTED_FILE)..."
if ! run_with_timeout 30 java -cp "$BIN_DIR" Main "$DATA_DIR" "$EXPECTED_FILE" > /dev/null; then
    echo "Error: Sequential baseline generation failed."
    exit 1
fi

# run_and_verify <csv_file> <p_label> <command...>
run_and_verify() {
    local csv_file=$1
    local p_label=$2
    shift 2

    for ((run = 1; run <= RUNS; run++)); do
        local result
        result=$(run_with_timeout 30 "$@")
        local exit_code=$?
        if [ $exit_code -ne 0 ]; then
            echo "Error: Run failed or timed out (run $run, exit $exit_code): $*"
            exit 1
        fi

        if ! cmp -s "$OUTPUT_FILE" "$EXPECTED_FILE"; then
            echo "Verification FAILED on run $run: $OUTPUT_FILE differs from $EXPECTED_FILE"
            echo "Command: $*"
            exit 1
        fi

        local ms
        ms=$(echo "$result" | sed -nE 's/.*:[[:space:]]*([0-9]+([.,][0-9]+)?)[[:space:]]*ms.*/\1/p' | tail -n 1 | tr ',' '.')
        if [ -z "$ms" ]; then
            echo "Error: Could not extract timing from output on run $run:"
            echo "$result"
            exit 1
        fi

        echo "${DATASET_NAME},${p_label},${ms}" >> "$csv_file"
    done
    echo "  OK ($RUNS runs verified): $*"
}

echo ""
echo "=========================================================="
echo "Running Lab 4 Benchmarks on dataset: $DATASET_NAME ($RUNS runs each)"
echo "=========================================================="

# 1. Sequential implementation
run_and_verify "$SEQ_CSV" 1 java -cp "$BIN_DIR" Main "$DATA_DIR" "$OUTPUT_FILE"

# 2. Parallel implementation (p_r = 2, 3 and p = 4, 8, 16)
for pr in $READER_COUNTS; do
    csv_target="$RESULTS_DIR/parallel_pr${pr}_${DATASET_NAME}.csv"
    for p in $THREAD_COUNTS; do
        run_and_verify "$csv_target" "$p" java -cp "$BIN_DIR" Main "$DATA_DIR" "$OUTPUT_FILE" "$p" "$pr"
    done
done

echo ""
echo "=========================================================="
echo "Averages (via compute_averages)"
echo "=========================================================="

print_averages() {
    local label=$1
    local csv_file=$2
    echo "--- $label ($csv_file) ---"
    if command -v compute_averages &> /dev/null; then
        compute_averages "$csv_file"
    else
        awk -F',' 'NF==3 { key=$1" elements, "$2" threads"; sum[key]+=$3; count[key]++ } END { for (k in sum) printf "%-30s -> Average: %.4f ms (from %d runs)\n", k, sum[k]/count[k], count[k] }' "$csv_file"
    fi
}

print_averages "Sequential" "$SEQ_CSV"
print_averages "Parallel (p_r = 2)" "$PR2_CSV"
print_averages "Parallel (p_r = 3)" "$PR3_CSV"

echo ""
echo "=========================================================="
echo "Speedup Summary (T_sequential / T_parallel)"
echo "=========================================================="

awk -F',' '
    FILENAME == ARGV[1] && NF == 3 { seq_sum += $3; seq_cnt++ }
    FILENAME == ARGV[2] && NF == 3 { pr2_sum[$2] += $3; pr2_cnt[$2]++ }
    FILENAME == ARGV[3] && NF == 3 { pr3_sum[$2] += $3; pr3_cnt[$2]++ }
    END {
        if (seq_cnt == 0) {
            print "No sequential timings found."
            exit 1
        }
        t_seq = seq_sum / seq_cnt
        printf "%-20s | %-15s | %-15s\n", "Configuration", "Avg Time (ms)", "Speedup (T_s/T_p)"
        printf "%-20s-+-%-15s-+-%-17s\n", "--------------------", "---------------", "-----------------"
        printf "%-20s | %-15.4f | %-15s\n", "Sequential (p=1)", t_seq, "1.0000x"

        split("4 8 16", procs, " ")
        for (i = 1; i <= 3; i++) {
            p = procs[i]
            if (pr2_cnt[p] > 0) {
                t = pr2_sum[p] / pr2_cnt[p]
                printf "%-20s | %-15.4f | %-15s\n", "p=" p ", p_r=2", t, sprintf("%.4fx", t_seq / t)
            }
        }
        for (i = 1; i <= 3; i++) {
            p = procs[i]
            if (pr3_cnt[p] > 0) {
                t = pr3_sum[p] / pr3_cnt[p]
                printf "%-20s | %-15.4f | %-15s\n", "p=" p ", p_r=3", t, sprintf("%.4fx", t_seq / t)
            }
        }
    }
' "$SEQ_CSV" "$PR2_CSV" "$PR3_CSV"

echo ""
echo "Done. Final verified output written to: $OUTPUT_FILE"
