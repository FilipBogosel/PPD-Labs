#!/usr/bin/env bash
# Runs the Lab 3 MPI test cases. Every variant is executed RUNS times.
# After each run, Numar3.txt is compared with expected_Numar3.txt (or sequential baseline)
# and the timing line "<test_name>,<p>,<time_ms>" is appended to a CSV file in results/.
#
# Usage (from solution folder):
#   ./run_tests.sh                    -> runs all test cases across all variants
#   ./run_tests.sh tests/test1_16     -> runs only the given test folder

RUNS=10

# Ensure test cases are generated
if [ ! -d "tests/test1_16" ] || [ ! -d "tests/test2_10k_10k" ] || [ ! -d "tests/test3_100_100k" ]; then
    echo "Test directories not found. Generating inputs..."
    python3 generate_inputs.py || exit 1
fi

echo "Compiling C++ and MPI binaries..."
mkdir -p bin results

# Find mpicxx
MPICXX="mpicxx"
if command -v /opt/homebrew/bin/mpicxx &> /dev/null; then
    MPICXX="/opt/homebrew/bin/mpicxx"
fi

MPIRUN="mpirun --oversubscribe"
if command -v /opt/homebrew/bin/mpirun &> /dev/null; then
    MPIRUN="/opt/homebrew/bin/mpirun --oversubscribe"
fi

# Variant 0 (Sequential baseline)
g++ -O3 -std=c++17 cpp_v2/main_v0.cpp cpp_v2/utils.cpp -o bin/variant0_seq || exit 1

# Variant 1 (Standard / Synchronous)
$MPICXX -O3 -std=c++17 cpp_v2/main_v1.cpp cpp_v2/utils.cpp -o bin/variant1_blocking || exit 1

# Variant 1.1 (Speculative Optimization - 2 Bonus Points)
$MPICXX -O3 -std=c++17 cpp_v2/main_v1.1.cpp cpp_v2/utils.cpp -o bin/variant1_1_opt || exit 1

# Variant 2 (Scatter / Gather)
$MPICXX -O3 -std=c++17 cpp_v2/main_v2.cpp cpp_v2/utils.cpp -o bin/variant2_scatter || exit 1

# Variant 3 (Asynchronous non-blocking)
$MPICXX -O3 -std=c++17 cpp_v2/main_v3.cpp cpp_v2/utils.cpp -o bin/variant3_async || exit 1

echo "Compilation successful."

# run_with_timeout <seconds> <command...>
run_with_timeout() {
    local timeout_sec=$1
    shift
    python3 -c "import subprocess, sys; res = subprocess.run(sys.argv[2:], timeout=float(sys.argv[1]), capture_output=True, text=True); print(res.stdout, end=''); sys.exit(res.returncode)" "$timeout_sec" "$@"
}

# run_and_verify <csv_file> <test_folder> <expected_file> <command...>
run_and_verify() {
    local csv_file=$1
    local folder=$2
    local expected_file=$3
    shift 3

    for ((run = 1; run <= RUNS; run++)); do
        local result
        # 60s timeout protects against MPI deadlocks
        result=$(run_with_timeout 60 "$@")
        local exit_code=$?

        if [ $exit_code -ne 0 ]; then
            echo "Run failed or timed out (deadlock?): $* (run $run, exit $exit_code)"
            exit 1
        fi

        if ! diff -w "$folder/Numar3.txt" "$expected_file" > /dev/null; then
            echo "Verification FAILED: output does not match $expected_file on run $run!"
            echo "Command was: $*"
            exit 1
        fi

        # Extract only the timing line matching <name>,<p>,<ms>
        local timing_line
        timing_line=$(echo "$result" | grep -E "^[a-zA-Z0-9_]+,[0-9]+,[0-9.]+$" | tail -n 1)
        if [ -n "$timing_line" ]; then
            echo "$timing_line" >> "$csv_file"
        fi
    done
    echo "  OK ($RUNS runs verified): $*"
}

# run_test_case <test_folder>
run_test_case() {
    local folder=${1%/}
    local name
    name=$(basename "$folder")
    local expected="$folder/expected_Numar3.txt"

    echo "=========================================================="
    echo "Running Test Case: $name"
    echo "=========================================================="

    rm -f results/*_"$name".csv

    # 1. Variant 0 (Sequential baseline)
    run_and_verify "results/var0_seq_$name.csv" "$folder" "$expected" bin/variant0_seq "$folder"

    # Define process counts required by spec
    local procs_var1_3="5 9 17"
    local procs_var2="4 8 16"

    # For test1_16, specification tests with p=5 for Var 1&3, and p=4 for Var 2
    if [ "$name" == "test1_16" ]; then
        procs_var1_3="5"
        procs_var2="4"
    fi

    # 2. Variant 1 (Standard Blocking)
    for p in $procs_var1_3; do
        run_and_verify "results/var1_blocking_$name.csv" "$folder" "$expected" $MPIRUN -np "$p" bin/variant1_blocking "$folder"
    done

    # 2b. Variant 1.1 (Speculative Optimization - Bonus)
    if [ -f bin/variant1_1_opt ]; then
        for p in $procs_var1_3; do
            run_and_verify "results/var1_1_opt_$name.csv" "$folder" "$expected" $MPIRUN -np "$p" bin/variant1_1_opt "$folder"
        done
    fi

    # 3. Variant 2 (Scatter / Gather)
    for p in $procs_var2; do
        run_and_verify "results/var2_scatter_$name.csv" "$folder" "$expected" $MPIRUN -np "$p" bin/variant2_scatter "$folder"
    done

    # 4. Variant 3 (Asynchronous Isend/Irecv)
    for p in $procs_var1_3; do
        run_and_verify "results/var3_async_$name.csv" "$folder" "$expected" $MPIRUN -np "$p" bin/variant3_async "$folder"
    done

    echo ""
    echo "Summary Averages for $name:"
    for csv_file in results/*_"$name".csv; do
        echo "----------------------------------------------------------"
        echo "File: $csv_file"
        if command -v compute_averages &> /dev/null; then
            compute_averages "$csv_file"
        else
            awk -F',' 'NF==3 { sum[$1" ("$2" procs)"] += $3; count[$1" ("$2" procs)"]++ } END { for (k in sum) printf "%-30s -> Average: %.4f ms (from %d runs)\n", k, sum[k]/count[k], count[k] }' "$csv_file"
        fi
    done
    echo ""
}

# Determine target folders
if [ "$#" -gt 0 ]; then
    folders=("$@")
else
    folders=(tests/test1_16 tests/test2_10k_10k tests/test3_100_100k)
fi

for folder in "${folders[@]}"; do
    run_test_case "$folder"
done
