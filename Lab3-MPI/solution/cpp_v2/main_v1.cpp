#include "utils.h"
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <mpi.h>

// Tags for MPI communication
enum Tags {
    TAG_CHUNK_SIZE = 10,
    TAG_DIGITS_1 = 11,
    TAG_DIGITS_2 = 12,
    TAG_CARRY = 20,
    TAG_RESULT_SIZE = 30,
    TAG_RESULT_DATA = 31
};

// =========================================================================
// Shared Communication Helpers
// =========================================================================

void send_chunk_to_worker(int worker_id, int chunk_size, const std::vector<int>& buf_a, const std::vector<int>& buf_b) {
    MPI_Ssend(&chunk_size, 1, MPI_INT, worker_id, TAG_CHUNK_SIZE, MPI_COMM_WORLD);
    if (chunk_size > 0) {
        MPI_Ssend(buf_a.data(), chunk_size, MPI_INT, worker_id, TAG_DIGITS_1, MPI_COMM_WORLD);
        MPI_Ssend(buf_b.data(), chunk_size, MPI_INT, worker_id, TAG_DIGITS_2, MPI_COMM_WORLD);
    }
}

void receive_chunk_from_master(int& chunk_size, std::vector<int>& local_a, std::vector<int>& local_b) {
    MPI_Recv(&chunk_size, 1, MPI_INT, 0, TAG_CHUNK_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local_a.resize(chunk_size);
    local_b.resize(chunk_size);
    if (chunk_size > 0) {
        MPI_Recv(local_a.data(), chunk_size, MPI_INT, 0, TAG_DIGITS_1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(local_b.data(), chunk_size, MPI_INT, 0, TAG_DIGITS_2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void send_result_to_master(const std::vector<int>& local_sum) {
    int result_size = static_cast<int>(local_sum.size());
    MPI_Ssend(&result_size, 1, MPI_INT, 0, TAG_RESULT_SIZE, MPI_COMM_WORLD);
    if (result_size > 0) {
        MPI_Ssend(local_sum.data(), result_size, MPI_INT, 0, TAG_RESULT_DATA, MPI_COMM_WORLD);
    }
}

int receive_carry(int rank) {
    int carry_in = 0;
    if (rank > 1) {
        MPI_Recv(&carry_in, 1, MPI_INT, rank - 1, TAG_CARRY, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    return carry_in;
}

void send_carry(int carry, int rank, int num_workers) {
    if (rank < num_workers) {
        MPI_Ssend(&carry, 1, MPI_INT, rank + 1, TAG_CARRY, MPI_COMM_WORLD);
    }
}

// =========================================================================
// Process 0 (Master) Workflow - Streaming Edition
// =========================================================================

void master_workflow(const TestPaths& paths, int num_procs) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::ifstream file1, file2;
    int n1, n2, max_len;
    if (!open_input_files(paths.num1, paths.num2, file1, file2, n1, n2, max_len)) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    int num_workers = num_procs - 1;
    // Phase 1: Stream Read & Dispatch
    int d1_read = 0, d2_read = 0;
    for (int worker_id = 1; worker_id <= num_workers; ++worker_id) {
        int current_chunk_size = get_worker_chunk_size(max_len, num_workers, worker_id);
        
        std::vector<int> buf_a, buf_b;
        if (current_chunk_size > 0) {
            read_chunk_stream(file1, file2, current_chunk_size, n1, n2, d1_read, d2_read, buf_a, buf_b);
        }
        send_chunk_to_worker(worker_id, current_chunk_size, buf_a, buf_b);
    }
    file1.close();
    file2.close();

    // Phase 2: Stream Receive & Write
    std::ofstream file3;
    std::streampos length_pos;
    if (!init_output_stream(paths.num3, file3, length_pos)) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    int total_digits_written = 0;
    for (int w = 1; w <= num_workers; ++w) {
        int result_size = 0;
        MPI_Recv(&result_size, 1, MPI_INT, w, TAG_RESULT_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        if (result_size > 0) {
            std::vector<int> segment_result(result_size);
            MPI_Recv(segment_result.data(), result_size, MPI_INT, w, TAG_RESULT_DATA, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            write_chunk_stream(file3, segment_result, total_digits_written);
        }
    }
    finalize_output_stream(file3, length_pos, total_digits_written);

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    print_benchmark(paths.test_name, num_procs, duration_ms);
}

// =========================================================================
// Worker (Process > 0) Workflow
// =========================================================================

void worker_workflow(int rank, int num_workers) {
    int chunk_size = 0;
    std::vector<int> local_a, local_b;
    receive_chunk_from_master(chunk_size, local_a, local_b);
    
    int carry_in = receive_carry(rank);

    std::vector<int> local_sum(chunk_size);
    int carry_out = compute_sum_chunk(local_a, local_b, local_sum, carry_in);

    send_carry(carry_out, rank, num_workers);

    if (rank == num_workers && carry_out > 0) {
        local_sum.push_back(carry_out);
    }

    send_result_to_master(local_sum);
}

// =========================================================================
// Main Entry
// =========================================================================

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 2) {
        if (rank == 0) std::cerr << "Error: Variant 1 requires at least 2 processes.\n";
        MPI_Finalize();
        return 1;
    }
    if (argc < 2) {
        if (rank == 0) std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <test_folder>\n";
        MPI_Finalize();
        return 1;
    }

    TestPaths paths = parse_test_paths(argv[1]);

    if (rank == 0) {
        master_workflow(paths, num_procs);
    } else {
        worker_workflow(rank, num_procs - 1);
    }

    MPI_Finalize();
    return 0;
}
