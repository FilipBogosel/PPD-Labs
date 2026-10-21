#ifndef CONVOLUTION_STATIC_H
#define CONVOLUTION_STATIC_H

#include <fstream>
#include <iostream>
#include <string>

const int MAX_N = 10000;
const int MAX_M = 10000;
const int MAX_K = 5;

static int matrixF[MAX_N][MAX_M];
static int kernelC[MAX_K][MAX_K];
static int resultV[MAX_N][MAX_M];

static int n = 0;
static int m = 0;
static int k = 0;

inline int clampIndex(int index, int size) {
    if (index < 0) {
        return 0;
    }
    if (index >= size) {
        return size - 1;
    }
    return index;
}

inline void convolveRegionStatic(int rowStart, int rowEnd, int colStart, int colEnd) {
    int radius = k / 2;

    for (int i = rowStart; i < rowEnd; ++i) {
        for (int j = colStart; j < colEnd; ++j) {
            int sum = 0;
            for (int a = 0; a < k; ++a) {
                int row = clampIndex(i + a - radius, n);
                for (int b = 0; b < k; ++b) {
                    int col = clampIndex(j + b - radius, m);
                    sum += matrixF[row][col] * kernelC[a][b];
                }
            }
            resultV[i][j] = sum;
        }
    }
}

inline bool readMatrix(const std::string& filename, bool isKernel) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open input file " << filename << "\n";
        return false;
    }

    if (isKernel) {
        in >> k >> k;
        for (int i = 0; i < k; ++i) {
            for (int j = 0; j < k; ++j) {
                in >> kernelC[i][j];
            }
        }
    } else {
        in >> n >> m;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                in >> matrixF[i][j];
            }
        }
    }
    return true;
}

inline bool writeOutput(const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open output file " << filename << "\n";
        return false;
    }
    out << n << " " << m << "\n";
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (j > 0) {
                out << " ";
            }
            out << resultV[i][j];
        }
        out << "\n";
    }
    return true;
}

const std::string MATRIX_FILE = "MatrixF.txt";
const std::string KERNEL_FILE = "ConvolutionM.txt";

inline bool readInput(const std::string& inputFolder) {
    if (!readMatrix(inputFolder + "/" + MATRIX_FILE, false) ||
        !readMatrix(inputFolder + "/" + KERNEL_FILE, true)) {
        return false;
    }
    if (k % 2 == 0) {
        std::cerr << "Error: The convolution matrix must be k x k with k odd\n";
        return false;
    }
    return true;
}

#endif