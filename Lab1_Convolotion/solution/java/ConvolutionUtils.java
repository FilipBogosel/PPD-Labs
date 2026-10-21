import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;

public class ConvolutionUtils {

    // Matrix stored row by row: element (i, j) is data[i * cols + j].
    public static class Matrix {
        public final int rows;
        public final int cols;
        public final int[] data;

        public Matrix(int rows, int cols) {
            this.rows = rows;
            this.cols = cols;
            this.data = new int[rows * cols];
        }
    }

    // Keeps an index inside [0, size - 1]. This simulates the virtual border:
    // an element outside the matrix is equal to the closest boundary element.
    public static int clampIndex(int index, int size) {
        if (index < 0) {
            return 0;
        }
        if (index >= size) {
            return size - 1;
        }
        return index;
    }

    // Computes result(i, j) for rowStart <= i < rowEnd and colStart <= j < colEnd.
    public static void convolveRegion(Matrix matrix, Matrix kernel, Matrix result,
                                      int rowStart, int rowEnd, int colStart, int colEnd) {
        int n = matrix.rows;
        int m = matrix.cols;
        int k = kernel.rows;
        int radius = k / 2;

        for (int i = rowStart; i < rowEnd; i++) {
            for (int j = colStart; j < colEnd; j++) {
                int sum = 0;
                for (int a = 0; a < k; a++) {
                    int row = clampIndex(i + a - radius, n);
                    for (int b = 0; b < k; b++) {
                        int col = clampIndex(j + b - radius, m);
                        sum += matrix.data[row * m + col] * kernel.data[a * k + b];
                    }
                }
                result.data[i * m + j] = sum;
            }
        }
    }

    // File format: first line "rows cols", then one matrix row per line.
    public static Matrix readMatrix(String filename) throws IOException {
        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            String[] header = reader.readLine().trim().split("\\s+");
            Matrix matrix = new Matrix(Integer.parseInt(header[0]), Integer.parseInt(header[1]));

            for (int i = 0; i < matrix.rows; i++) {
                String[] values = reader.readLine().trim().split("\\s+");
                for (int j = 0; j < matrix.cols; j++) {
                    matrix.data[i * matrix.cols + j] = Integer.parseInt(values[j]);
                }
            }
            return matrix;
        }
    }

    public static void writeOutput(String filename, Matrix matrix) throws IOException {
        try (PrintWriter writer = new PrintWriter(filename)) {
            writer.println(matrix.rows + " " + matrix.cols);
            for (int i = 0; i < matrix.rows; i++) {
                for (int j = 0; j < matrix.cols; j++) {
                    if (j > 0) {
                        writer.print(' ');
                    }
                    writer.print(matrix.data[i * matrix.cols + j]);
                }
                writer.println();
            }
        }
    }

    // The convolution matrix is always named ConvolutionM.txt and sits next to MatrixF.txt.
    public static String kernelFileFor(String matrixFile) {
        return new File(new File(matrixFile).getParentFile(), "ConvolutionM.txt").getPath();
    }

    public static Matrix readKernel(String matrixFile) throws IOException {
        Matrix kernel = readMatrix(kernelFileFor(matrixFile));
        if (kernel.rows != kernel.cols || kernel.rows % 2 == 0) {
            throw new IOException("The convolution matrix must be k x k with k odd");
        }
        return kernel;
    }
}
