#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../convolution_static.h"

using std::chrono::duration;
using std::chrono::high_resolution_clock;

// Vertical decomposition: every thread computes a band of consecutive columns (all rows).
void convolveVerticalStatic(int threadCount) {
    int cols = m;

    // Guard against requesting more threads than columns
    int activeThreads = std::min(threadCount, cols);
    std::vector<std::thread> threads;

    int sliceSize = cols / activeThreads;
    int remainder = cols % activeThreads;

    // The first "remainder" threads get one extra column, so the work is balanced
    int begin = 0;
    for (int t = 0; t < activeThreads; ++t) {
        int extraColumn = (t < remainder) ? 1 : 0;
        int end = begin + sliceSize + extraColumn;
        threads.emplace_back(convolveRegionStatic, 0, n, begin, end);
        begin = end;
    }

    for (std::thread& thread : threads) {
        thread.join();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <MatrixF_file> <output_file> <threads>\n";
        return 1;
    }

    std::string matrixFile = argv[1];
    std::string outputFile = argv[2];
    int threadCount = std::stoi(argv[3]);

    if (!readInput(matrixFile)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    convolveVerticalStatic(threadCount);
    auto time2 = high_resolution_clock::now();

    writeOutput(outputFile);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << n << "x" << m << "," << threadCount << "," << msDouble.count() << "\n";

    return 0;
}