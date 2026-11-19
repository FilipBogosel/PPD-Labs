public class Parallel {

    private static class Worker extends Thread {
        private final int[] matrix;
        private final int m;
        private final int[] kernel;
        private final int startRow;
        private final int endRow;
        private final int[] topBoundary;
        private final int[] bottomBoundary;

        public Worker(int[] matrix, int m, int[] kernel, int startRow, int endRow,
                      int[] topBoundary, int[] bottomBoundary) {
            this.matrix = matrix;
            this.m = m;
            this.kernel = kernel;
            this.startRow = startRow;
            this.endRow = endRow;
            this.topBoundary = topBoundary;
            this.bottomBoundary = bottomBoundary;
        }

        @Override
        public void run() {
            ConvolutionUtils.convolveBandInPlace(matrix, m, kernel, startRow, endRow, topBoundary, bottomBoundary);
        }
    }

    public static void convolveParallel(int[] matrix, int n, int m, int[] kernel, int threadCount)
            throws InterruptedException {
        int activeThreads = Math.min(threadCount, n);
        int sliceSize = n / activeThreads;
        int remainder = n % activeThreads;

        Worker[] threads = new Worker[activeThreads];

    
        int begin = 0;
        for (int t = 0; t < activeThreads; t++) {
            int extraRow = (t < remainder) ? 1 : 0;
            int end = begin + sliceSize + extraRow;
            int[] topBoundary = ConvolutionUtils.copyRow(matrix, m, ConvolutionUtils.clampIndex(begin - 1, n));
            int[] bottomBoundary = ConvolutionUtils.copyRow(matrix, m, ConvolutionUtils.clampIndex(end, n));
            threads[t] = new Worker(matrix, m, kernel, begin, end, topBoundary, bottomBoundary);
            begin = end;
        }

        
        for (int t = 0; t < activeThreads; t++) {
            threads[t].start();
        }

        for (int t = 0; t < activeThreads; t++) {
            threads[t].join();
        }
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 3) {
            System.err.println("Usage: java Parallel <input_folder> <output_file> <threads>");
            return;
        }

        String inputFolder = args[0];
        String outputFile = args[1];
        int threadCount = Integer.parseInt(args[2]);
        if (threadCount < 1) {
            System.err.println("Error: The number of threads must be at least 1");
            return;
        }

        ConvolutionUtils.Input input = ConvolutionUtils.readInput(inputFolder);
        int n = input.rows;
        int m = input.cols;

        long time1 = System.nanoTime();
        convolveParallel(input.matrix, n, m, input.kernel, threadCount);
        long time2 = System.nanoTime();

        ConvolutionUtils.writeOutput(outputFile, n, m, input.matrix);

        double msDouble = (time2 - time1) / 1_000_000.0;
        System.out.println(n + "x" + m + "," + threadCount + "," + msDouble);
    }
}
