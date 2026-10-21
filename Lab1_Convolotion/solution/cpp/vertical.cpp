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

void convolveVertical(const Matrix& matrix, const Matrix& kernel, Matrix& result, int threadCount) {
    int m = matrix.cols;

    int activeThreads = std::min(threadCount, m);
    std::vector<std::thread> threads;

    int sliceSize = m / activeThreads;
    int remainder = m % activeThreads;

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
        std::cerr << "Usage: " << argv[0] << " <input_folder> <output_file> <threads>\n";
        return 1;
    }

    std::string inputFolder = argv[1];
    std::string outputFile = argv[2];
    int threadCount = std::stoi(argv[3]);

    Matrix matrix, kernel, result;
    if (!readInput(inputFolder, matrix, kernel, result)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    convolveVertical(matrix, kernel, result, threadCount);
    auto time2 = high_resolution_clock::now();

    writeOutput(outputFile, result);

    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << matrix.rows << "x" << matrix.cols << "," << threadCount << "," << msDouble.count() << "\n";

    freeMatrix(matrix);
    freeMatrix(kernel);
    freeMatrix(result);
    return 0;
}
