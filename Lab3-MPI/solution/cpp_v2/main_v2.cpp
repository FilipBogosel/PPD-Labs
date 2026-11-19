#include "utils.h"
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <mpi.h>

namespace {
    enum Tags {
        TAG_CARRY = 20,
        TAG_FINAL_OVERFLOW = 21
    };
}

static void master_prepare_data(const int rank, const int num_procs, const TestPaths& paths,
                                std::vector<int>& global_a, std::vector<int>& global_b, int& chunk_size) {
    if (rank != 0) return; // Only Master reads and prepares data

    if (!read_number(paths.num1, global_a) || !read_number(paths.num2, global_b)) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    pad_arrays(global_a, global_b);
    const int max_len = static_cast<int>(global_a.size());
    
    // MPI_Scatter requires chunks to be perfectly equal.
    // If the length isn't divisible by p, pad it with leading zeros.
    int rem = max_len % num_procs;
    if (rem != 0) {
        int padded_len = max_len + (num_procs - rem);
        global_a.resize(padded_len, 0);
        global_b.resize(padded_len, 0);
    }
    
    chunk_size = static_cast<int>(global_a.size()) / num_procs;
}

static int receive_carry_in(const int rank) {
    int carry_in = 0;
    // Rank 0 is the start of the number, so it receives no carry
    if (rank > 0) {
        MPI_Recv(&carry_in, 1, MPI_INT, rank - 1, TAG_CARRY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    return carry_in;
}

static int route_carry_and_overflow(int rank, int num_procs, int carry_out) {
    // 1. Pass the standard carry to the next sequential process
    if (rank < num_procs - 1) {
        MPI_Ssend(&carry_out, 1, MPI_INT, rank + 1, TAG_CARRY, MPI_COMM_WORLD);
    }
    
    // 2. The absolute last process holds the final potential overflow digit
    int final_overflow = 0;
    if (rank == num_procs - 1) {
        final_overflow = carry_out;
        if (num_procs > 1) {
            MPI_Ssend(&final_overflow, 1, MPI_INT, 0, TAG_FINAL_OVERFLOW, MPI_COMM_WORLD);
        }
    }
    
    // 3. Master receives this final overflow digit to append it at the end
    if (rank == 0 && num_procs > 1) {
        MPI_Recv(&final_overflow, 1, MPI_INT, num_procs - 1, TAG_FINAL_OVERFLOW, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    return final_overflow;
}

static void master_finalize_and_write(int rank, int num_procs, const TestPaths& paths,
                                      std::vector<int>& global_sum, int final_overflow,
                                      std::chrono::time_point<std::chrono::high_resolution_clock> start_time) {
    if (rank != 0) return; // Only Master writes

    if (final_overflow > 0) {
        global_sum.push_back(final_overflow);
    }
    
    // Trim any artificial trailing zeros we created during the Scatter padding phase
    // (In little-endian, trailing zeros in the array are leading zeros in the actual number)
    while (global_sum.size() > 1 && global_sum.back() == 0) {
        global_sum.pop_back();
    }
    
    if (!write_number(paths.num3, global_sum)) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    print_benchmark(paths.test_name, num_procs, duration_ms);
}

// =========================================================================
// Main Entry (Unified SPMD Architecture)
// =========================================================================

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (argc < 2) {
        if (rank == 0) std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <test_folder>\n";
        MPI_Finalize();
        return 1;
    }

    TestPaths paths = parse_test_paths(argv[1]);

    auto start_time = std::chrono::high_resolution_clock::now();

    // 1. Data Preparation
    std::vector<int> global_a, global_b;
    int chunk_size = 0;
    
    master_prepare_data(rank, num_procs, paths, global_a, global_b, chunk_size);
    
    // Broadcast the chunk size so all processes know how much memory to allocate
    MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2. Scatter Distribution
    std::vector<int> local_a(chunk_size), local_b(chunk_size), local_sum(chunk_size);
    
    MPI_Scatter(global_a.empty() ? nullptr : global_a.data(), chunk_size, MPI_INT, 
                local_a.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
                
    MPI_Scatter(global_b.empty() ? nullptr : global_b.data(), chunk_size, MPI_INT, 
                local_b.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // 3. Computation Pipeline
    int carry_in = receive_carry_in(rank);
    
    // Reuse the exact same clean math function from utils.cpp
    int carry_out = compute_sum_chunk(local_a, local_b, local_sum, carry_in);
    
    int final_overflow = route_carry_and_overflow(rank, num_procs, carry_out);

    // 4. Gather Collection
    std::vector<int> global_sum;
    if (rank == 0) global_sum.resize(chunk_size * num_procs);
    
    MPI_Gather(local_sum.data(), chunk_size, MPI_INT,
               global_sum.empty() ? nullptr : global_sum.data(), chunk_size, MPI_INT,
               0, MPI_COMM_WORLD);

    // 5. Output
    master_finalize_and_write(rank, num_procs, paths, global_sum, final_overflow, start_time);

    MPI_Finalize();
    return 0;
}
