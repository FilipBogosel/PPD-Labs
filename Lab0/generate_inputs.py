import random

def generate_input_file(filename, n):
    with open(filename, 'w') as f:
        f.write(f"{n}\n")
        # Vector A
        a = [round(random.uniform(-1000.0, 1000.0), 4) for _ in range(n)]
        f.write(" ".join(map(str, a)) + "\n")
        # Vector B
        b = [round(random.uniform(-1000.0, 1000.0), 4) for _ in range(n)]
        f.write(" ".join(map(str, b)) + "\n")
    print(f"Generated {filename} with n={n}")

if __name__ == "__main__":
    random.seed(42)  # Reproducible data
    for size in [1000, 10000, 100000]:
        generate_input_file(f"input_{size}.txt", size)
