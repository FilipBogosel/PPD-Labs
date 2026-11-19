import java.io.BufferedReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

public class Parallel {
    private final SortedLinkedList gradeList = new SortedLinkedList();
    private final SemaphoreQueue queue = new SemaphoreQueue();

    public void execute(String[] args) {
        if (args.length != 4) {
            System.err.println("Parameters: <data_folder> <output_file> <p> <p_r> for the parallel version");
            System.exit(1);
        }

        String projectsGradesFolder = args[0];
        String outputFile = args[1];
        int totalThreads = Integer.parseInt(args[2]);
        int readerThreadsCount = Integer.parseInt(args[3]);
        int workerThreadsCount = totalThreads - readerThreadsCount;

        if (readerThreadsCount < 1 || workerThreadsCount < 1) {
            System.err.println("Error: Require p_r >= 1 and p > p_r (at least 1 worker thread).");
            System.exit(1);
        }

        long startTime = System.nanoTime();

        List<Path> projectGradeFiles = Utils.getFilesInFolder(projectsGradesFolder);
        AtomicInteger nextFileIndex = new AtomicInteger(0);

        List<Thread> workers = startWorkers(workerThreadsCount);
        List<Thread> helperReaders = startHelperReaders(readerThreadsCount - 1, projectGradeFiles, nextFileIndex);

        // The main thread acts as one of the p_r reader threads
        runReaderTask(projectGradeFiles, nextFileIndex);

        joinAll(helperReaders);
        sendPoisonPills(workerThreadsCount);
        joinAll(workers);

        Utils.writeSnapshotToFile(gradeList.snapshot(), outputFile);

        double durationMs = (System.nanoTime() - startTime) / 1_000_000.0;
        System.out.printf("Parallel (p=%d, p_r=%d): %.3f ms%n", totalThreads, readerThreadsCount, durationMs);
    }

    private List<Thread> startWorkers(int workerCount) {
        List<Thread> workers = new ArrayList<>(workerCount);
        for (int i = 0; i < workerCount; i++) {
            Thread worker = new Thread(this::runWorkerTask, "Worker-" + i);
            worker.start();
            workers.add(worker);
        }
        return workers;
    }

    private List<Thread> startHelperReaders(int helperReaderCount, List<Path> files, AtomicInteger nextFileIndex) {
        List<Thread> readers = new ArrayList<>(helperReaderCount);
        for (int i = 0; i < helperReaderCount; i++) {
            Thread reader = new Thread(() -> runReaderTask(files, nextFileIndex), "Reader-" + i);
            reader.start();
            readers.add(reader);
        }
        return readers;
    }

    private void runReaderTask(List<Path> files, AtomicInteger nextFileIndex) {
        while (true) {
            int fileIndex = nextFileIndex.getAndIncrement();
            if (fileIndex >= files.size()) {
                return;
            }
            readFileIntoQueue(files.get(fileIndex));
        }
    }

    private void readFileIntoQueue(Path filePath) {
        try (BufferedReader reader = Files.newBufferedReader(filePath)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.isBlank()) {
                    continue;
                }
                String[] parts = line.split(",");
                int studentId = Integer.parseInt(parts[0].trim());
                int projectGrade = Integer.parseInt(parts[1].trim());
                queue.put(studentId, projectGrade);
            }
        } catch (IOException e) {
            throw new RuntimeException("Failed to read file: " + filePath, e);
        }
    }

    private void runWorkerTask() {
        while (true) {
            StudentGrade item = queue.take();
            if (item.equals(SemaphoreQueue.POISON_PILL)) {
                return;
            }
            gradeList.addOrUpdate(item.studentId(), item.totalGrade());
        }
    }

    private void sendPoisonPills(int workerCount) {
        for (int i = 0; i < workerCount; i++) {
            queue.put(SemaphoreQueue.POISON_PILL);
        }
    }

    private void joinAll(List<Thread> threads) {
        for (Thread thread : threads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                throw new RuntimeException("Interrupted while waiting for thread: " + thread.getName(), e);
            }
        }
    }
}
