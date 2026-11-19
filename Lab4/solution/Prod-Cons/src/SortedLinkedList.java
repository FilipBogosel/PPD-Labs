import java.util.ArrayList;
import java.util.List;

public class SortedLinkedList {

    private static class Node {
        final int studentId;
        int totalGrade;
        Node next;

        Node(int studentId, int totalGrade, Node next) {
            this.studentId = studentId;
            this.totalGrade = totalGrade;
            this.next = next;
        }
    }

    // Sentinel node avoids edge cases when inserting before the first real student
    private final Node head;

    public SortedLinkedList() {
        this.head = new Node(Integer.MIN_VALUE, 0, null);
    }

    public synchronized void addOrUpdate(int studentId, int projectGrade) {
        Node previous = head;
        Node current = head.next;

        while (current != null && current.studentId < studentId) {
            previous = current;
            current = current.next;
        }

        if (current != null && current.studentId == studentId) {
            current.totalGrade += projectGrade;
        } else {
            previous.next = new Node(studentId, projectGrade, current);
        }
    }

    public synchronized List<StudentGrade> snapshot() {
        List<StudentGrade> results = new ArrayList<>();
        Node current = head.next;
        while (current != null) {
            results.add(new StudentGrade(current.studentId, current.totalGrade));
            current = current.next;
        }
        return results;
    }
}