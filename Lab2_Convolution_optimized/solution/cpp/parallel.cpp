#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "convolution.h"

using std::chrono::duration;
using std::chrono::high_resolution_clock;

// Horizontal decomposition: every thread convolves a band of consecutive rows in place.
void convolveParallel(std::vector<int>& matrix, int n, int m, const std::vector<int>& kernel, int threadCount) {
    // Guard against requesting more threads than rows
    int activeThreads = std::min(threadCount, n);
    int sliceSize = n / activeThreads;
    int remainder = n % activeThreads;

    std::vector<int> startRows(activeThreads);
    std::vector<int> endRows(activeThreads);
    std::vector<std::vector<int>> topBoundaries(activeThreads);
    std::vector<std::vector<int>> bottomBoundaries(activeThreads);


    int begin = 0;
    for (int t = 0; t < activeThreads; ++t) {
        int extraRow = (t < remainder) ? 1 : 0;
        int end = begin + sliceSize + extraRow;
        startRows[t] = begin;
        endRows[t] = end;
        topBoundaries[t] = copyRow(matrix, m, clampIndex(begin - 1, n));
        bottomBoundaries[t] = copyRow(matrix, m, clampIndex(end, n));
        begin = end;
    }

    std::vector<std::thread> threads;
    for (int t = 0; t < activeThreads; ++t) {
        threads.emplace_back(convolveBandInPlace, std::ref(matrix), m, std::cref(kernel),
                             startRows[t], endRows[t],
                             std::cref(topBoundaries[t]), std::cref(bottomBoundaries[t]));
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_folder> <output_file> <threads>\n";
        return 1;
    }

    std::string inputFolder = argv[1];
    std::string outputFile = argv[2];
    int threadCount = std::stoi(argv[3]);
    if (threadCount < 1) {
        std::cerr << "Error: The number of threads must be at least 1\n";
        return 1;
    }

    int n = 0;
    int m = 0;
    std::vector<int> matrix, kernel;
    if (!readInput(inputFolder, n, m, matrix, kernel)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    convolveParallel(matrix, n, m, kernel, threadCount);
    auto time2 = high_resolution_clock::now();

    if (!writeOutput(outputFile, n, m, matrix)) {
        return 1;
    }

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << n << "x" << m << "," << threadCount << "," << msDouble.count() << "\n";
    return 0;
}
