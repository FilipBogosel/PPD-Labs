#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include <fstream>
#include <iostream>
#include <string>

// Dynamically allocated matrix, stored row by row: element (i, j) is data[i * cols + j].
struct Matrix {
    int rows = 0;
    int cols = 0;
    int* data = nullptr;
};

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
inline void convolveRegion(const Matrix& matrix, const Matrix& kernel, Matrix& result,
                           int rowStart, int rowEnd, int colStart, int colEnd) {
    int n = matrix.rows;
    int m = matrix.cols;
    int k = kernel.rows;
    int radius = k / 2;

    for (int i = rowStart; i < rowEnd; ++i) {
        for (int j = colStart; j < colEnd; ++j) {
            int sum = 0;
            for (int a = 0; a < k; ++a) {
                int row = clampIndex(i + a - radius, n);
                for (int b = 0; b < k; ++b) {
                    int col = clampIndex(j + b - radius, m);
                    sum += matrix.data[row * m + col] * kernel.data[a * k + b];
                }
            }
            result.data[i * m + j] = sum;
        }
    }
}

// File format: first line "rows cols", then the elements row by row.
inline bool readMatrix(const std::string& filename, Matrix& matrix) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open input file " << filename << "\n";
        return false;
    }
    in >> matrix.rows >> matrix.cols;
    matrix.data = new int[matrix.rows * matrix.cols];
    for (int i = 0; i < matrix.rows * matrix.cols; ++i) {
        in >> matrix.data[i];
    }
    return true;
}

inline bool writeOutput(const std::string& filename, const Matrix& matrix) {
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open output file " << filename << "\n";
        return false;
    }
    out << matrix.rows << " " << matrix.cols << "\n";
    for (int i = 0; i < matrix.rows; ++i) {
        for (int j = 0; j < matrix.cols; ++j) {
            if (j > 0) {
                out << " ";
            }
            out << matrix.data[i * matrix.cols + j];
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

// Reads F and C and allocates the result matrix V with the same size as F.
inline bool readInput(const std::string& matrixFile, Matrix& matrix, Matrix& kernel, Matrix& result) {
    if (!readMatrix(matrixFile, matrix) || !readMatrix(kernelFileFor(matrixFile), kernel)) {
        return false;
    }
    if (kernel.rows != kernel.cols || kernel.rows % 2 == 0) {
        std::cerr << "Error: The convolution matrix must be k x k with k odd\n";
        return false;
    }
    result.rows = matrix.rows;
    result.cols = matrix.cols;
    result.data = new int[result.rows * result.cols];
    return true;
}

inline void freeMatrix(Matrix& matrix) {
    delete[] matrix.data;
    matrix.data = nullptr;
}

#endif
