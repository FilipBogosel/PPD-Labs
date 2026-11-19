#include "utils.h"
#include <iostream>
#include <string>
#include <chrono>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <test_folder>\n";
        return 1;
    }

    std::string folder = argv[1];
    std::string num1_path = folder + "/Numar1.txt";
    std::string num2_path = folder + "/Numar2.txt";
    std::string num3_path = folder + "/Numar3.txt";

    // Start timing (including file I/O as requested by the assignment)
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<int> num1, num2;
    if (!read_number(num1_path, num1) || !read_number(num2_path, num2)) {
        return 1;
    }

    // Pad arrays to simplify the addition loop
    pad_arrays(num1, num2);
    size_t max_len = num1.size();
    
    std::vector<int> sum(max_len);
    int carry = 0;

    // Compute the sum digit by digit
    for (size_t i = 0; i < max_len; ++i) {
        int total = num1[i] + num2[i] + carry;
        sum[i] = total % 10;
        carry = total / 10;
    }

    // Append final carry if it exists
    if (carry > 0) {
        sum.push_back(carry);
    }

    if (!write_number(num3_path, sum)) {
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    // Extract test folder basename to output a clean CSV line for benchmarking
    size_t last_slash = folder.find_last_of("/\\");
    std::string test_name = (last_slash == std::string::npos) ? folder : folder.substr(last_slash + 1);
    
    std::cout << test_name << ",1," << duration_ms << "\n";

    return 0;
}
