import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

public class Utils {
    public static List<Path> getFilesInFolder(String folderPath) {
        try (var stream = Files.list(Path.of(folderPath))) {
            return stream
                    .filter((file) ->
                            Files.isRegularFile(file) && file.toString().endsWith(".txt"))
                    .toList();
        }
        catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public static void writeSnapshotToFile(List<StudentGrade> snapshot, String outputFile) {
        try (var writer = Files.newBufferedWriter(Path.of(outputFile))) {
            for (var studentGrade : snapshot) {
                writer.write(studentGrade.studentId() + "," + studentGrade.totalGrade()/10 );
                writer.newLine();
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }
}
