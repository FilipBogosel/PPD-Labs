#ifndef CONVOLUTION_STATIC_H
#define CONVOLUTION_STATIC_H

#include <fstream>
#include <iostream>
#include <string>

// Maximum matrix dimensions based on the test cases
const int MAX_N = 10000;
const int MAX_M = 10000;
const int MAX_K = 5;

// Global static matrices
static int matrixF[MAX_N][MAX_M];
static int kernelC[MAX_K][MAX_K];
static int resultV[MAX_N][MAX_M];

// Current dimensions
static int n = 0;
static int m = 0;
static int k = 0;

// Keeps an index inside [0, size - 1]. This simulates the virtual border:
// an element outside the matrix is equal to the closest boundary element.
inline int clampIndex(int index, int size) {
    if (index < 0) {
        return 0;
    }
    if (index >= size) {
        return size - 1;
    }
    return index;
}

// Computes result(i, j) for rowStart <= i < rowEnd and colStart <= j < colEnd.
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

// File format: first line "rows cols", then the elements row by row.
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

// The convolution matrix is always named ConvolutionM.txt and sits next to MatrixF.txt.
inline std::string kernelFileFor(const std::string& matrixFile) {
    size_t slash = matrixFile.find_last_of('/');
    if (slash == std::string::npos) {
        return "ConvolutionM.txt";
    }
    return matrixFile.substr(0, slash + 1) + "ConvolutionM.txt";
}

inline bool readInput(const std::string& matrixFile) {
    if (!readMatrix(matrixFile, false) || !readMatrix(kernelFileFor(matrixFile), true)) {
        return false;
    }
    if (k % 2 == 0) {
        std::cerr << "Error: The convolution matrix must be k x k with k odd\n";
        return false;
    }
    return true;
}

#endif