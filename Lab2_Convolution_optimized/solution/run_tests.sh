#!/usr/bin/env bash
# Runs the lab test cases. Every variant is executed RUNS times on the same input file.
# After each run, output.txt is compared with the sequential result (expected_output.txt)
# and the timing line "n x m,threads,ms" is appended to a CSV file in results/.
#
# Usage (from the solution folder, after running generate_inputs.py):
#   ./run_tests.sh                    -> all test cases
#   ./run_tests.sh tests/1000x1000    -> only the given test case folders

RUNS=10
THREAD_COUNTS="2 4 8 16"

echo "Compiling..."
mkdir -p bin/java results
g++ -O3 -std=c++17 cpp/sequential.cpp -o bin/sequential || exit 1
g++ -O3 -std=c++17 cpp/parallel.cpp -o bin/parallel || exit 1
javac -d bin/java java/ConvolutionUtils.java java/Sequential.java java/Parallel.java || exit 1

# run_and_verify <csv_file> <test_folder> <command...>
run_and_verify() {
    local csv_file=$1
    local folder=$2
    shift 2
    for ((run = 1; run <= RUNS; run++)); do
        local result
        result=$("$@") || { echo "Run failed: $*"; exit 1; }
        if ! cmp -s "$folder/output.txt" "$folder/expected_output.txt"; then
            echo "Verification FAILED: $* (run $run)"
            exit 1
        fi
        echo "$result" | tail -n 1 >> "$csv_file"
    done
    echo "  OK ($RUNS runs verified): $*"
}

# run_test_case <test_folder>
run_test_case() {
    local folder=${1%/}
    local name
    name=$(basename "$folder")
    local output="$folder/output.txt"

    # The 10x10 case is only required with p = 2
    local thread_counts=$THREAD_COUNTS
    if [ "$name" == "10x10" ]; then
        thread_counts="2"
    fi

    echo "Test case $name"
    rm -f results/*_"$name".csv

    # Reference result, computed once with the sequential C++ program
    bin/sequential "$folder" "$folder/expected_output.txt" 1 > /dev/null || exit 1

    run_and_verify "results/cpp_sequential_$name.csv" "$folder" bin/sequential "$folder" "$output" 1
    run_and_verify "results/java_sequential_$name.csv" "$folder" java -cp bin/java Sequential "$folder" "$output" 1

    for threads in $thread_counts; do
        run_and_verify "results/cpp_parallel_$name.csv" "$folder" bin/parallel "$folder" "$output" "$threads"
        run_and_verify "results/java_parallel_$name.csv" "$folder" java -cp bin/java Parallel "$folder" "$output" "$threads"
    done

    for csv_file in results/*_"$name".csv; do
        echo "$csv_file"
        compute_averages "$csv_file"
    done
}

if [ "$#" -gt 0 ]; then
    folders=("$@")
else
    folders=(tests/10x10 tests/1000x1000 tests/10000x10000)
fi

for folder in "${folders[@]}"; do
    run_test_case "$folder"
done
