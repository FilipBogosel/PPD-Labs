#include <chrono>
#include <iostream>
#include <string>

#include "convolution.h"

using std::chrono::duration;
using std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <MatrixF_file> <output_file> <threads>\n";
        return 1;
    }

    std::string matrixFile = argv[1];
    std::string outputFile = argv[2];
    int threads = std::stoi(argv[3]);  // ignored, the sequential version uses only the main thread

    Matrix matrix, kernel, result;
    if (!readInput(matrixFile, matrix, kernel, result)) {
        return 1;
    }

    auto time1 = high_resolution_clock::now();
    convolveRegion(matrix, kernel, result, 0, matrix.rows, 0, matrix.cols);
    auto time2 = high_resolution_clock::now();

    writeOutput(outputFile, result);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> msDouble = time2 - time1;
    std::cout << matrix.rows << "x" << matrix.cols << "," << threads << "," << msDouble.count() << "\n";

    freeMatrix(matrix);
    freeMatrix(kernel);
    freeMatrix(result);
    return 0;
}
