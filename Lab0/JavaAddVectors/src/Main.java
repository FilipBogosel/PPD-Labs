import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Scanner;

public class Main {
    public static double[] addVectors(double[] a, double[] b) {
        double[] c = new double[a.length];
        for (int i = 0; i < a.length; ++i) {
            c[i] = a[i] + b[i];
        }
        return c;
    }

    public static double[][] readInput(String filename) throws IOException {
        try (Scanner scanner = new Scanner(new File(filename))) {
            int n = scanner.nextInt();
            double[] a = new double[n];
            double[] b = new double[n];

            for (int i = 0; i < n; i++) {
                a[i] = scanner.nextDouble();
            }
            for (int i = 0; i < n; i++) {
                b[i] = scanner.nextDouble();
            }

            return new double[][]{a, b};
        }
    }

    public static void writeOutput(String filename, int n, double[] c) throws IOException {
        try (PrintWriter writer = new PrintWriter(filename)) {
            writer.println(n);
            for (int i = 0; i < n; i++) {
                writer.printf("%.4f ", c[i]);
            }
            writer.println();
        }
    }


    public static void main(String[] args) throws Exception {
        if (args.length != 3) {
            System.err.println("Usage: java Main <input_file> <output_file> <threads>");
            return;
        }

        String inputFile = args[0];
        String outputFile = args[1];
        int threads = Integer.parseInt(args[2]);

        double[][] vectors = readInput(inputFile);
        double[] a = vectors[0];
        double[] b = vectors[1];
        int n = a.length;

        long time1 = System.nanoTime();
        double[] c = addVectors(a, b);
        long time2 = System.nanoTime();

        writeOutput(outputFile, n, c);

        /* Getting number of milliseconds as a double. */
        double msDouble = (time2 - time1) / 1_000_000.0;
        System.out.println(n + "," + threads + "," + msDouble);
    }
}