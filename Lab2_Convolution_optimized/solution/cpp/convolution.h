#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::string INPUT_FILE = "date.txt";
const int KERNEL_SIZE = 3;

inline int clampIndex(int index, int size) {
    if (index < 0) {
        return 0;
    }
    if (index >= size) {
        return size - 1;
    }
    return index;
}

inline bool readInput(const std::string& inputFolder, int& n, int& m,
                      std::vector<int>& matrix, std::vector<int>& kernel) {
    std::string path = inputFolder + "/" + INPUT_FILE;
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open input file " << path << "\n";
        return false;
    }

    in >> n >> m;
    if (!in || n <= 0 || m <= 0) {
        std::cerr << "Error: Invalid matrix size in " << path << "\n";
        return false;
    }
    matrix.resize(static_cast<size_t>(n) * m);
    for (size_t i = 0; i < matrix.size(); ++i) {
        in >> matrix[i];
    }

    int kernelRows = 0;
    int kernelCols = 0;
    in >> kernelRows >> kernelCols;
    if (kernelRows != KERNEL_SIZE || kernelCols != KERNEL_SIZE) {
        std::cerr << "Error: The convolution matrix must be 3 x 3\n";
        return false;
    }
    kernel.resize(KERNEL_SIZE * KERNEL_SIZE);
    for (size_t i = 0; i < kernel.size(); ++i) {
        in >> kernel[i];
    }

    if (!in) {
        std::cerr << "Error: Malformed input file " << path << "\n";
        return false;
    }
    return true;
}

inline bool writeOutput(const std::string& filename, int n, int m, const std::vector<int>& matrix) {
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
            out << matrix[i * m + j];
        }
        out << "\n";
    }
    return true;
}

inline std::vector<int> copyRow(const std::vector<int>& matrix, int m, int row) {
    std::vector<int>::const_iterator first = matrix.begin() + row * m;
    return std::vector<int>(first, first + m);
}

inline void convolveBandInPlace(std::vector<int>& matrix, int m, const std::vector<int>& kernel,
                                int startRow, int endRow,
                                const std::vector<int>& topBoundary,
                                const std::vector<int>& bottomBoundary) {
    std::vector<int> prevRow = topBoundary;
    std::vector<int> currRow(m);

    for (int i = startRow; i < endRow; ++i) {
        int* row = &matrix[i * m];

        std::copy(row, row + m, currRow.begin());

        const int* nextRow = (i + 1 < endRow) ? &matrix[(i + 1) * m] : bottomBoundary.data();

        for (int j = 0; j < m; ++j) {
            int left = clampIndex(j - 1, m);
            int right = clampIndex(j + 1, m);
            row[j] = prevRow[left] * kernel[0] + prevRow[j] * kernel[1] + prevRow[right] * kernel[2] +
                     currRow[left] * kernel[3] + currRow[j] * kernel[4] + currRow[right] * kernel[5] +
                     nextRow[left] * kernel[6] + nextRow[j] * kernel[7] + nextRow[right] * kernel[8];
        }

        std::swap(prevRow, currRow);
    }
}

#endif
