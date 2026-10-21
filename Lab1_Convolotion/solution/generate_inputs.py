import os
import random

# (n, m, k) for every test case required by the lab
TEST_CASES = [
    (10, 10, 3),
    (1000, 1000, 3),
    (10, 10000, 5),
    (10000, 10, 5),
    (10000, 10000, 5),
]


def write_matrix(filename, rows, cols, low, high):
    # First line: "rows cols", then one matrix row per line
    with open(filename, 'w') as f:
        f.write(f"{rows} {cols}\n")
        for _ in range(rows):
            row = [str(random.randint(low, high)) for _ in range(cols)]
            f.write(" ".join(row) + "\n")


def generate_test_case(n, m, k):
    folder = os.path.join("tests", f"{n}x{m}_k{k}")
    os.makedirs(folder, exist_ok=True)
    # F holds pixel-like values, C holds small weights
    write_matrix(os.path.join(folder, "MatrixF.txt"), n, m, 0, 255)
    write_matrix(os.path.join(folder, "ConvolutionM.txt"), k, k, -2, 2)
    print(f"Generated {folder} with n={n}, m={m}, k={k}")


if __name__ == "__main__":
    random.seed(42)  # Reproducible data
    for n, m, k in TEST_CASES:
        generate_test_case(n, m, k)
