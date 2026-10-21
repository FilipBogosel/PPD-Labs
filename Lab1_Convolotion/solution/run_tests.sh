#!/usr/bin/env bash
# Runs the lab test cases. Every variant is executed RUNS times on the same input files.
# After each run, output.txt is compared with the sequential result (expected_output.txt)
# and the timing line "n x m,threads,ms" is appended to a CSV file in results/.
#
# Usage (from the solution folder, after running generate_inputs.py):
#   ./run_tests.sh                      -> all test cases
#   ./run_tests.sh tests/1000x1000_k3   -> only the given test case folders

RUNS=10
THREAD_COUNTS="2 4 8 16"

echo "Compiling..."
mkdir -p bin/java results
# Dynamic C++
g++ -O3 -std=c++17 cpp/sequential.cpp -o bin/sequential || exit 1
g++ -O3 -std=c++17 cpp/horizontal.cpp -o bin/horizontal || exit 1
g++ -O3 -std=c++17 cpp/vertical.cpp -o bin/vertical || exit 1
# Static C++
g++ -O3 -std=c++17 cpp/static/sequential.cpp -o bin/seq_static || exit 1
g++ -O3 -std=c++17 cpp/static/horizontal.cpp -o bin/horiz_static || exit 1
g++ -O3 -std=c++17 cpp/static/vertical.cpp -o bin/vert_static || exit 1
# Java
javac -d bin/java java/*.java || exit 1

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
    local input="$folder"
    local output="$folder/output.txt"

    # The 10x10 case is only required with p = 4
    local thread_counts=$THREAD_COUNTS
    if [ "$name" == "10x10_k3" ]; then
        thread_counts="4"
    fi

    echo "Test case $name"
    rm -f results/*_"$name".csv

    # Reference result, computed once with the sequential C++ program
    bin/sequential "$input" "$folder/expected_output.txt" 1 > /dev/null || exit 1

    run_and_verify "results/cpp_sequential_$name.csv" "$folder" bin/sequential "$input" "$output" 1
    run_and_verify "results/cpp_static_sequential_$name.csv" "$folder" bin/seq_static "$input" "$output" 1
    run_and_verify "results/java_sequential_$name.csv" "$folder" java -cp bin/java Sequential "$input" "$output" 1

    for threads in $thread_counts; do
        # Dynamic C++
        run_and_verify "results/cpp_horizontal_$name.csv" "$folder" bin/horizontal "$input" "$output" "$threads"
        run_and_verify "results/cpp_vertical_$name.csv" "$folder" bin/vertical "$input" "$output" "$threads"
        # Static C++
        run_and_verify "results/cpp_static_horizontal_$name.csv" "$folder" bin/horiz_static "$input" "$output" "$threads"
        run_and_verify "results/cpp_static_vertical_$name.csv" "$folder" bin/vert_static "$input" "$output" "$threads"
        # Java
        run_and_verify "results/java_horizontal_$name.csv" "$folder" java -cp bin/java Horizontal "$input" "$output" "$threads"
        run_and_verify "results/java_vertical_$name.csv" "$folder" java -cp bin/java Vertical "$input" "$output" "$threads"
    done

    for csv_file in results/*_"$name".csv; do
        echo "$csv_file"
        compute_averages "$csv_file"
    done
}

if [ "$#" -gt 0 ]; then
    folders=("$@")
else
    folders=(tests/10x10_k3 tests/1000x1000_k3 tests/10x10000_k5 tests/10000x10_k5 tests/10000x10000_k5)
fi

for folder in "${folders[@]}"; do
    run_test_case "$folder"
done
