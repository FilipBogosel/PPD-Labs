#!/usr/bin/env python3
import os
import random

def compute_expected_sum(digits1, digits2):
    """
    Computes digit-by-digit sum of two numbers where digits are in little-endian order
    (digits[0] is least significant).
    Returns list of digits representing the sum, without leading zeros (except for single 0).
    """
    n1 = len(digits1)
    n2 = len(digits2)
    max_len = max(n1, n2)
    
    result = []
    carry = 0
    for i in range(max_len):
        d1 = digits1[i] if i < n1 else 0
        d2 = digits2[i] if i < n2 else 0
        total = d1 + d2 + carry
        result.append(total % 10)
        carry = total // 10
        
    if carry > 0:
        result.append(carry)
        
    return result

def write_number_file(filepath, digits):
    """
    Writes a number file:
    Line 1: N (number of digits)
    Line 2: space-separated digits (little-endian: least significant digit at index 0)
    """
    with open(filepath, 'w') as f:
        f.write(f"{len(digits)}\n")
        f.write(" ".join(map(str, digits)) + "\n")

def generate_test_case(folder_name, digits1, digits2):
    folder_path = os.path.join("tests", folder_name)
    os.makedirs(folder_path, exist_ok=True)
    
    write_number_file(os.path.join(folder_path, "Numar1.txt"), digits1)
    write_number_file(os.path.join(folder_path, "Numar2.txt"), digits2)
    
    expected_sum = compute_expected_sum(digits1, digits2)
    write_number_file(os.path.join(folder_path, "expected_Numar3.txt"), expected_sum)
    
    print(f"Generated {folder_path} -> N1={len(digits1)}, N2={len(digits2)}, ExpectedSumDigits={len(expected_sum)}")

def main():
    random.seed(42)  # Fixed seed for reproducibility

    # Test 1: Fixed 16-digit test case specified in Lab 3 requirements
    # N1 = N2 = 16; Numar1 = "9999 4444 4444 9999", Numar2 = "9999 5555 5555 9999"
    # Digits in array: index 0 is least significant digit.
    t1_d1 = [9]*4 + [4]*4 + [4]*4 + [9]*4
    t1_d2 = [9]*4 + [5]*4 + [5]*4 + [9]*4
    generate_test_case("test1_16", t1_d1, t1_d2)

    # Test 2: Symmetric 10,000 digits random
    t2_d1 = [random.randint(0, 9) for _ in range(10000)]
    t2_d2 = [random.randint(0, 9) for _ in range(10000)]
    # Ensure most significant digit is non-zero
    t2_d1[-1] = random.randint(1, 9)
    t2_d2[-1] = random.randint(1, 9)
    generate_test_case("test2_10k_10k", t2_d1, t2_d2)

    # Test 3: Asymmetric N1 = 100, N2 = 100,000 digits random
    t3_d1 = [random.randint(0, 9) for _ in range(100)]
    t3_d2 = [random.randint(0, 9) for _ in range(100000)]
    t3_d1[-1] = random.randint(1, 9)
    t3_d2[-1] = random.randint(1, 9)
    generate_test_case("test3_100_100k", t3_d1, t3_d2)

if __name__ == "__main__":
    main()
