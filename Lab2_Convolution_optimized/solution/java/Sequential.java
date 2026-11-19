public class Sequential {

    public static void main(String[] args) throws Exception {
        if (args.length != 3) {
            System.err.println("Usage: java Sequential <input_folder> <output_file> <threads>");
            return;
        }

        String inputFolder = args[0];
        String outputFile = args[1];

        ConvolutionUtils.Input input = ConvolutionUtils.readInput(inputFolder);
        int n = input.rows;
        int m = input.cols;

        long time1 = System.nanoTime();
        int[] topBoundary = ConvolutionUtils.copyRow(input.matrix, m, ConvolutionUtils.clampIndex(0 - 1, n));
        int[] bottomBoundary = ConvolutionUtils.copyRow(input.matrix, m, ConvolutionUtils.clampIndex(n, n));
        ConvolutionUtils.convolveBandInPlace(input.matrix, m, input.kernel, 0, n, topBoundary, bottomBoundary);
        long time2 = System.nanoTime();

        ConvolutionUtils.writeOutput(outputFile, n, m, input.matrix);

        double msDouble = (time2 - time1) / 1_000_000.0;
        System.out.println(n + "x" + m + "," + 1 + "," + msDouble);
    }
}
