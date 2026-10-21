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

// Horizontal decomposition: every thread computes a band of consecutive rows (all columns).
void convolveHorizontal(const Matrix& matrix, const Matrix& kernel, Matrix& result, int threadCount) {
    int n = matrix.rows;

    // Guard against requesting more threads than rows
    int activeThreads = std::min(threadCount, n);
    std::vector<std::thread> threads;

    int sliceSize = n / activeThreads;
    int remainder = n % activeThreads;

    // The first "remainder" threads get one extra row, so the work is balanced
    int begin = 0;
    for (int t = 0; t < activeThreads; ++t) {
        int extraRow = (t < remainder) ? 1 : 0;
        int end = begin + sliceSize + extraRow;
        threads.emplace_back(convolveRegion, std::cref(matrix), std::cref(kernel), std::ref(result),
                             begin, end, 0, matrix.cols);
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
    convolveHorizontal(matrix, kernel, result, threadCount);
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
