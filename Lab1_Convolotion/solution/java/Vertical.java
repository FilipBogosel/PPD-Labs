public class Vertical {

    // Each worker computes a band of consecutive columns (all rows).
    private static class Worker extends Thread {
        private final ConvolutionUtils.Matrix matrix;
        private final ConvolutionUtils.Matrix kernel;
        private final ConvolutionUtils.Matrix result;
        private final int colStart;
        private final int colEnd;

        public Worker(ConvolutionUtils.Matrix matrix, ConvolutionUtils.Matrix kernel,
                      ConvolutionUtils.Matrix result, int colStart, int colEnd) {
            this.matrix = matrix;
            this.kernel = kernel;
            this.result = result;
            this.colStart = colStart;
            this.colEnd = colEnd;
        }

        @Override
        public void run() {
            ConvolutionUtils.convolveRegion(matrix, kernel, result, 0, matrix.rows, colStart, colEnd);
        }
    }

    public static void convolveVertical(ConvolutionUtils.Matrix matrix, ConvolutionUtils.Matrix kernel,
                                        ConvolutionUtils.Matrix result, int threadCount)
            throws InterruptedException {
        int m = matrix.cols;

        // Guard against requesting more threads than columns
        int activeThreads = Math.min(threadCount, m);
        Worker[] threads = new Worker[activeThreads];

        int sliceSize = m / activeThreads;
        int remainder = m % activeThreads;

        // The first "remainder" threads get one extra column, so the work is balanced
        int begin = 0;
        for (int t = 0; t < activeThreads; t++) {
            int extraColumn = (t < remainder) ? 1 : 0;
            int end = begin + sliceSize + extraColumn;
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
            System.err.println("Usage: java Vertical <input_folder> <output_file> <threads>");
            return;
        }

        String inputFolder = args[0];
        String outputFile = args[1];
        int threadCount = Integer.parseInt(args[2]);

        ConvolutionUtils.Matrix matrix = ConvolutionUtils.readMatrixF(inputFolder);
        ConvolutionUtils.Matrix kernel = ConvolutionUtils.readKernel(inputFolder);
        ConvolutionUtils.Matrix result = new ConvolutionUtils.Matrix(matrix.rows, matrix.cols);

        long time1 = System.nanoTime();
        convolveVertical(matrix, kernel, result, threadCount);
        long time2 = System.nanoTime();

        ConvolutionUtils.writeOutput(outputFile, result);

        /* Getting number of milliseconds as a double. */
        double msDouble = (time2 - time1) / 1_000_000.0;
        System.out.println(matrix.rows + "x" + matrix.cols + "," + threadCount + "," + msDouble);
    }
}
