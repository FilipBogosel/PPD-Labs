public class Horizontal {

    // Each worker computes a band of consecutive rows (all columns).
    private static class Worker extends Thread {
        private final ConvolutionUtils.Matrix matrix;
        private final ConvolutionUtils.Matrix kernel;
        private final ConvolutionUtils.Matrix result;
        private final int rowStart;
        private final int rowEnd;

        public Worker(ConvolutionUtils.Matrix matrix, ConvolutionUtils.Matrix kernel,
                      ConvolutionUtils.Matrix result, int rowStart, int rowEnd) {
            this.matrix = matrix;
            this.kernel = kernel;
            this.result = result;
            this.rowStart = rowStart;
            this.rowEnd = rowEnd;
        }

        @Override
        public void run() {
            ConvolutionUtils.convolveRegion(matrix, kernel, result, rowStart, rowEnd, 0, matrix.cols);
        }
    }

    public static void convolveHorizontal(ConvolutionUtils.Matrix matrix, ConvolutionUtils.Matrix kernel,
                                          ConvolutionUtils.Matrix result, int threadCount)
            throws InterruptedException {
        int n = matrix.rows;

        // Guard against requesting more threads than rows
        int activeThreads = Math.min(threadCount, n);
        Worker[] threads = new Worker[activeThreads];

        int sliceSize = n / activeThreads;
        int remainder = n % activeThreads;

        // The first "remainder" threads get one extra row, so the work is balanced
        int begin = 0;
        for (int t = 0; t < activeThreads; t++) {
            int extraRow = (t < remainder) ? 1 : 0;
            int end = begin + sliceSize + extraRow;
            threads[t] = new Worker(matrix, kernel, result, begin, end);
            threads[t].start();
            begin = end;
        }

        for (int t = 0; t < activeThreads; t++) {
            threads[t].join();
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 3) {
            System.err.println("Usage: java Horizontal <MatrixF_file> <output_file> <threads>");
            return;
        }

        String matrixFile = args[0];
        String outputFile = args[1];
        int threadCount = Integer.parseInt(args[2]);

        ConvolutionUtils.Matrix matrix = ConvolutionUtils.readMatrix(matrixFile);
        ConvolutionUtils.Matrix kernel = ConvolutionUtils.readKernel(matrixFile);
        ConvolutionUtils.Matrix result = new ConvolutionUtils.Matrix(matrix.rows, matrix.cols);

        long time1 = System.nanoTime();
        convolveHorizontal(matrix, kernel, result, threadCount);
        long time2 = System.nanoTime();

        ConvolutionUtils.writeOutput(outputFile, result);

        /* Getting number of milliseconds as a double. */
        double msDouble = (time2 - time1) / 1_000_000.0;
        System.out.println(matrix.rows + "x" + matrix.cols + "," + threadCount + "," + msDouble);
    }
}
