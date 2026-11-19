#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "convolution.h"

using std::chrono::duration;
using std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_folder> <output_file> <threads>\n";
        return 1;
    }

    std::string inputFolder = argv[1];
    std::string outputFile = argv[2];

    int n = 0;
    int m = 0;
    std::vector<int> matrix, kernel;
    if (!readInput(inputFolder, n, m, matrix, kernel)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    std::vector<int> topBoundary = copyRow(matrix, m, clampIndex(0 - 1, n));
    std::vector<int> bottomBoundary = copyRow(matrix, m, clampIndex(n, n));
    convolveBandInPlace(matrix, m, kernel, 0, n, topBoundary, bottomBoundary);
    auto time2 = high_resolution_clock::now();

    if (!writeOutput(outputFile, n, m, matrix)) {
        return 1;
    }

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << n << "x" << m << "," << 1 << "," << msDouble.count() << "\n";
    return 0;
}
