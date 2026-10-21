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

// Vertical decomposition: every thread computes a band of consecutive columns (all rows).
void convolveVertical(const Matrix& matrix, const Matrix& kernel, Matrix& result, int threadCount) {
    int m = matrix.cols;

    // Guard against requesting more threads than columns
    int activeThreads = std::min(threadCount, m);
    std::vector<std::thread> threads;

    int sliceSize = m / activeThreads;
    int remainder = m % activeThreads;

    // The first "remainder" threads get one extra column, so the work is balanced
    int begin = 0;
    for (int t = 0; t < activeThreads; ++t) {
        int extraColumn = (t < remainder) ? 1 : 0;
        int end = begin + sliceSize + extraColumn;
        threads.emplace_back(convolveRegion, std::cref(matrix), std::cref(kernel), std::ref(result),
                             0, matrix.rows, begin, end);
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

    Matrix matrix, kernel, result;
    if (!readInput(matrixFile, matrix, kernel, result)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    convolveVertical(matrix, kernel, result, threadCount);
    auto time2 = high_resolution_clock::now();

    writeOutput(outputFile, result);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << matrix.rows << "x" << matrix.cols << "," << threadCount << "," << msDouble.count() << "\n";

    freeMatrix(matrix);
    freeMatrix(kernel);
    freeMatrix(result);
    return 0;
}
