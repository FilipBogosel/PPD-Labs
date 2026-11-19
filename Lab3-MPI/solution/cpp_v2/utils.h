#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <fstream>

struct TestPaths {
    std::string num1;
    std::string num2;
    std::string num3;
    std::string test_name;
};

// =========================================================================
// General & CLI Helpers (Used by all variants)
// =========================================================================
TestPaths parse_test_paths(const std::string& folder);
int get_worker_chunk_size(int total_len, int num_workers, int worker_id);
void print_benchmark(const std::string& test_name, int num_procs, double duration_ms);

// =========================================================================
// Memory-based Array Operations (Used by V0 and V2)
// =========================================================================
bool read_number(const std::string& filename, std::vector<int>& digits);
bool write_number(const std::string& filename, const std::vector<int>& digits);
void pad_arrays(std::vector<int>& a, std::vector<int>& b);
int compute_sum_chunk(const std::vector<int>& a, const std::vector<int>& b, std::vector<int>& result, int initial_carry);

// =========================================================================
// Streaming File Operations (Used by V1, V1.1, and V3)
// =========================================================================

// Opens both input files and reads their total lengths (N1, N2).
bool open_input_files(const std::string& path1, const std::string& path2, 
                      std::ifstream& file1, std::ifstream& file2, 
                      int& n1, int& n2, int& max_len);

// Reads exactly 'chunk_size' digits from the streams, padding with zeros if EOF is reached.
void read_chunk_stream(std::ifstream& file1, std::ifstream& file2, 
                       int chunk_size, int n1, int n2, 
                       int& d1_read, int& d2_read, 
                       std::vector<int>& buf_a, std::vector<int>& buf_b);

// Opens output file and reserves 20 spaces at the top for the final digit count.
bool init_output_stream(const std::string& path, std::ofstream& file3, std::streampos& length_pos);

// Appends a received segment directly to the open output file.
void write_chunk_stream(std::ofstream& file3, const std::vector<int>& segment, int& total_written);

// Seeks back to the top of the file to write the true total length, then closes it.
void finalize_output_stream(std::ofstream& file3, std::streampos length_pos, int total_written);

#endif // UTILS_H
