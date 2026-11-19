import java.util.ArrayDeque;
import java.util.concurrent.Semaphore;
import java.util.concurrent.locks.ReentrantLock;

public class SemaphoreQueue {
    private final ArrayDeque<StudentGrade> projectGradesQueue = new ArrayDeque<>();
    private final ReentrantLock mutex = new ReentrantLock();
    private final Semaphore availableItems = new Semaphore(0);
    public static final StudentGrade POISON_PILL = new StudentGrade(-1, -1);

    public void put(Integer studentId, Integer gradeValue) {
        this.put(new StudentGrade(studentId, gradeValue));
    }

    public void put(StudentGrade studentGrade){
        mutex.lock();
        try{
            projectGradesQueue.addLast(studentGrade);
        } finally {
            mutex.unlock();
        }
        availableItems.release();
    }



    public StudentGrade take() {
        try {
            availableItems.acquire();
            mutex.lock();
            try{
                return projectGradesQueue.removeFirst();
            } finally {
                mutex.unlock();
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new RuntimeException(e);
        }
    }

}
