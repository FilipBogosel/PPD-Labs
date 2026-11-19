from pathlib import Path
import random
import sys

DEFAULT_NUM_STUDENTS = 10_000
DEFAULT_NUM_FILES = 10


def generate_unique_indexes(num_students: int) -> list[int]:
    min_grades = max(1, int(0.7 * num_students))
    number_of_elements = random.randint(min_grades, num_students)
    return random.sample(range(num_students), k=number_of_elements)


def generate_project_file(file_path: Path, num_students: int) -> int:
    indexes = generate_unique_indexes(num_students)
    lines = [f"{student_id},{random.randint(1, 100)}\n" for student_id in indexes]
    with file_path.open("w") as file:
        file.writelines(lines)
    return len(lines)


def main() -> None:
    num_students = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_NUM_STUDENTS
    num_files = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_NUM_FILES

    output_dir = Path(__file__).resolve().parent / "test_data"
    output_dir.mkdir(parents=True, exist_ok=True)

    total_entries = 0
    for i in range(1, num_files + 1):
        file_path = output_dir / f"project{i}.txt"
        total_entries += generate_project_file(file_path, num_students)

    print(f"Generated {num_files} files in {output_dir} for {num_students} students ({total_entries} total entries).")


if __name__ == "__main__":
    main()
