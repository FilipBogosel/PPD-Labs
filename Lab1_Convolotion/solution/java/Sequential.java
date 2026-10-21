public class Sequential {

    public static void main(String[] args) throws Exception {
        if (args.length != 3) {
            System.err.println("Usage: java Sequential <MatrixF_file> <output_file> <threads>");
            return;
        }

        String matrixFile = args[0];
        String outputFile = args[1];
        int threads = Integer.parseInt(args[2]);  // ignored, the sequential version uses only the main thread

        ConvolutionUtils.Matrix matrix = ConvolutionUtils.readMatrix(matrixFile);
        ConvolutionUtils.Matrix kernel = ConvolutionUtils.readKernel(matrixFile);
        ConvolutionUtils.Matrix result = new ConvolutionUtils.Matrix(matrix.rows, matrix.cols);

        long time1 = System.nanoTime();
        ConvolutionUtils.convolveRegion(matrix, kernel, result, 0, matrix.rows, 0, matrix.cols);
        long time2 = System.nanoTime();

        ConvolutionUtils.writeOutput(outputFile, result);

        /* Getting number of milliseconds as a double. */
        double msDouble = (time2 - time1) / 1_000_000.0;
        System.out.println(matrix.rows + "x" + matrix.cols + "," + threads + "," + msDouble);
    }
}
