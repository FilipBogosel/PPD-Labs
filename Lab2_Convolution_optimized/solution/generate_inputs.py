import os
import random

# (n, m) for every test case required by the lab, the kernel is always 3 x 3
TEST_CASES = [
    (10, 10),
    (1000, 1000),
    (10000, 10000),
]

KERNEL_SIZE = 3


def generate_test_case(n, m):
    folder = os.path.join("tests", f"{n}x{m}")
    os.makedirs(folder, exist_ok=True)
    with open(os.path.join(folder, "date.txt"), 'w') as f:
        # Matrix F holds pixel-like values: first line "N M", then one matrix row per line
        f.write(f"{n} {m}\n")
        for _ in range(n):
            row = [str(random.randint(0, 255)) for _ in range(m)]
            f.write(" ".join(row) + "\n")
        # Convolution matrix holds small weights: first line "3 3", then one kernel row per line
        f.write(f"{KERNEL_SIZE} {KERNEL_SIZE}\n")
        for _ in range(KERNEL_SIZE):
            row = [str(random.randint(-2, 2)) for _ in range(KERNEL_SIZE)]
            f.write(" ".join(row) + "\n")
    print(f"Generated {folder} with n={n}, m={m}")


if __name__ == "__main__":
    random.seed(42)  # Reproducible data
    for n, m in TEST_CASES:
        generate_test_case(n, m)
