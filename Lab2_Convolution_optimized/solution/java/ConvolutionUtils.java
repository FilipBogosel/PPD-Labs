import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.StringTokenizer;

public class ConvolutionUtils {

    public static final String INPUT_FILE = "date.txt";
    public static final int KERNEL_SIZE = 3;

    public static class Input {
        public final int rows;
        public final int cols;
        public final int[] matrix;
        public final int[] kernel;

        public Input(int rows, int cols, int[] matrix, int[] kernel) {
            this.rows = rows;
            this.cols = cols;
            this.matrix = matrix;
            this.kernel = kernel;
        }
    }


    public static int clampIndex(int index, int size) {
        if (index < 0) {
            return 0;
        }
        if (index >= size) {
            return size - 1;
        }
        return index;
    }

    // Returns a copy of one matrix row 
    public static int[] copyRow(int[] matrix, int m, int row) {
        int[] copy = new int[m];
        System.arraycopy(matrix, row * m, copy, 0, m);
        return copy;
    }


    public static void convolveBandInPlace(int[] matrix, int m, int[] kernel,
                                           int startRow, int endRow,
                                           int[] topBoundary, int[] bottomBoundary) {
        int[] prevRow = topBoundary.clone();
        int[] currRow = new int[m];

        for (int i = startRow; i < endRow; i++) {
            int rowStart = i * m;

            System.arraycopy(matrix, rowStart, currRow, 0, m);

            int[] nextRow;
            int nextStart;
            if (i + 1 < endRow) {
                nextRow = matrix;
                nextStart = rowStart + m;
            } else {
                nextRow = bottomBoundary;
                nextStart = 0;
            }

            for (int j = 0; j < m; j++) {
                int left = clampIndex(j - 1, m);
                int right = clampIndex(j + 1, m);
                matrix[rowStart + j] =
                        prevRow[left] * kernel[0] + prevRow[j] * kernel[1] + prevRow[right] * kernel[2] +
                        currRow[left] * kernel[3] + currRow[j] * kernel[4] + currRow[right] * kernel[5] +
                        nextRow[nextStart + left] * kernel[6] + nextRow[nextStart + j] * kernel[7] +
                        nextRow[nextStart + right] * kernel[8];
            }

            // The original row i becomes the previous row of the next iteration.
            // Swapping is the same as copying currRow into prevRow, because currRow
            // is completely overwritten by the snapshot at the start of the next iteration.
            int[] swap = prevRow;
            prevRow = currRow;
            currRow = swap;
        }
    }

    public static Input readInput(String inputFolder) throws IOException {
        File file = new File(inputFolder, INPUT_FILE);
        try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
            StringTokenizer sizeLine = new StringTokenizer(reader.readLine());
            int n = Integer.parseInt(sizeLine.nextToken());
            int m = Integer.parseInt(sizeLine.nextToken());
            if (n <= 0 || m <= 0) {
                throw new IOException("Invalid matrix size in " + file.getPath());
            }

            int[] matrix = new int[n * m];
            for (int i = 0; i < n; i++) {
                StringTokenizer rowLine = new StringTokenizer(reader.readLine());
                for (int j = 0; j < m; j++) {
                    matrix[i * m + j] = Integer.parseInt(rowLine.nextToken());
                }
            }

            StringTokenizer kernelSizeLine = new StringTokenizer(reader.readLine());
            int kernelRows = Integer.parseInt(kernelSizeLine.nextToken());
            int kernelCols = Integer.parseInt(kernelSizeLine.nextToken());
            if (kernelRows != KERNEL_SIZE || kernelCols != KERNEL_SIZE) {
                throw new IOException("The convolution matrix must be 3 x 3");
            }

            int[] kernel = new int[KERNEL_SIZE * KERNEL_SIZE];
            for (int a = 0; a < KERNEL_SIZE; a++) {
                StringTokenizer kernelLine = new StringTokenizer(reader.readLine());
                for (int b = 0; b < KERNEL_SIZE; b++) {
                    kernel[a * KERNEL_SIZE + b] = Integer.parseInt(kernelLine.nextToken());
                }
            }
            return new Input(n, m, matrix, kernel);
        }
    }

    public static void writeOutput(String filename, int n, int m, int[] matrix) throws IOException {
        try (PrintWriter writer = new PrintWriter(filename)) {
            writer.println(n + " " + m);
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < m; j++) {
                    if (j > 0) {
                        writer.print(' ');
                    }
                    writer.print(matrix[i * m + j]);
                }
                writer.println();
            }
        }
    }
}
