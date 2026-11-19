#include "utils.h"
#include <iostream>
#include <algorithm>

// --- General & CLI Helpers ---
TestPaths parse_test_paths(const std::string& folder) {
    size_t last_slash = folder.find_last_of("/\\");
    std::string name = (last_slash == std::string::npos) ? folder : folder.substr(last_slash + 1);
    return {
        folder + "/Numar1.txt",
        folder + "/Numar2.txt",
        folder + "/Numar3.txt",
        name
    };
}

int get_worker_chunk_size(int total_len, int num_workers, int worker_id) {
    int base_chunk = total_len / num_workers;
    int remainder = total_len % num_workers;
    return base_chunk + (worker_id <= remainder ? 1 : 0);
}

void print_benchmark(const std::string& test_name, int num_procs, double duration_ms) {
    std::cout << test_name << "," << num_procs << "," << duration_ms << "\n";
}

// --- Existing memory-based utilities ---
bool read_number(const std::string& filename, std::vector<int>& digits) {
    std::ifstream file(filename);
    if (!file.is_open()) { std::cerr << "Error: Could not open " << filename << "\n"; return false; }
    int n; if (!(file >> n)) return false;
    digits.resize(n);
    for (int i = 0; i < n; ++i) { file >> digits[i]; }
    file.close(); return true;
}

bool write_number(const std::string& filename, const std::vector<int>& digits) {
    std::ofstream file(filename);
    if (!file.is_open()) { std::cerr << "Error: Could not open " << filename << "\n"; return false; }
    file << digits.size() << "\n";
    for (size_t i = 0; i < digits.size(); ++i) { file << digits[i] << (i + 1 == digits.size() ? "" : " "); }
    file << "\n"; file.close(); return true;
}

void pad_arrays(std::vector<int>& a, std::vector<int>& b) {
    size_t max_len = std::max(a.size(), b.size());
    a.resize(max_len, 0); b.resize(max_len, 0);
}

int compute_sum_chunk(const std::vector<int>& a, const std::vector<int>& b, std::vector<int>& result, int initial_carry) {
    int carry = initial_carry;
    for (size_t i = 0; i < a.size(); ++i) {
        const int total = a[i] + b[i] + carry;
        result[i] = total % 10;
        carry = total / 10;
    }
    return carry;
}

// --- New streaming utilities ---
bool open_input_files(const std::string& path1, const std::string& path2, std::ifstream& file1, std::ifstream& file2, int& n1, int& n2, int& max_len) {
    file1.open(path1);
    file2.open(path2);
    if (!file1.is_open() || !file2.is_open()) {
        std::cerr << "Error opening input files.\n";
        return false;
    }
    file1 >> n1;
    file2 >> n2;
    max_len = std::max(n1, n2);
    return true;
}

void read_chunk_stream(std::ifstream& file1, std::ifstream& file2, int chunk_size, int n1, int n2, int& d1_read, int& d2_read, std::vector<int>& buf_a, std::vector<int>& buf_b) {
    buf_a.assign(chunk_size, 0);
    buf_b.assign(chunk_size, 0);
    for (int i = 0; i < chunk_size; ++i) {
        if (d1_read < n1) { file1 >> buf_a[i]; d1_read++; }
        if (d2_read < n2) { file2 >> buf_b[i]; d2_read++; }
    }
}

bool init_output_stream(const std::string& path, std::ofstream& file3, std::streampos& length_pos) {
    file3.open(path);
    if (!file3.is_open()) {
        std::cerr << "Error opening output file.\n";
        return false;
    }
    length_pos = file3.tellp();
    file3 << "                    \n"; // 20 spaces reserved for the final total digit count
    return true;
}

void write_chunk_stream(std::ofstream& file3, const std::vector<int>& segment, int& total_written) {
    for (int digit : segment) {
        file3 << digit << " ";
        total_written++;
    }
}

void finalize_output_stream(std::ofstream& file3, std::streampos length_pos, int total_written) {
    file3.seekp(length_pos);
    file3 << total_written;
    file3.close();
}
