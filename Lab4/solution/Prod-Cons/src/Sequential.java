import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Path;
import java.util.List;

public class Sequential {
    SortedLinkedList gradeList = new SortedLinkedList();

    public void execute(String[] args) {
        if (args.length != 2) {
            System.err.println("Parameters: <data_folder> <output_file> for the sequential version");
            System.exit(1);
        }

        String projectsGradesFolder = args[0];
        String outputFile = args[1];

        long startTime = System.nanoTime();

        var projectGradeFiles = Utils.getFilesInFolder(projectsGradesFolder);

        projectGradeFiles.forEach(path -> {
            try {
                readFile(path.toString());
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        });

        var snapshot = gradeList.snapshot();

        Utils.writeSnapshotToFile(snapshot, outputFile);

        double durationMs = (System.nanoTime() - startTime) / 1_000_000.0;
        System.out.printf(java.util.Locale.US, "Sequential: %.3f ms%n", durationMs);

    }

    private void readFile(String fileName) throws IOException {
        try(BufferedReader bufferedReader = new BufferedReader(new FileReader(Path.of(fileName).toFile()))) {
            List<String> lines = bufferedReader.lines().toList();
            for (String line : lines) {
                String[] parts = line.split(",");
                int studentId = Integer.parseInt(parts[0]);
                int projectGrade = Integer.parseInt(parts[1]);
                gradeList.addOrUpdate(studentId, projectGrade);
            }
        }
    }


}
