#include "utils.h"
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <mpi.h>

enum Tags {
    TAG_CHUNK_SIZE = 10,
    TAG_DIGITS_1 = 11,
    TAG_DIGITS_2 = 12,
    TAG_CARRY = 20,
    TAG_RESULT_SIZE = 30,
    TAG_RESULT_DATA = 31
};

// =========================================================================
// Master (Rank 0) Phase Helpers
// =========================================================================

static void async_dispatch_all(std::ifstream& file1, std::ifstream& file2, int n1, int n2, int max_len, int num_workers,
                               std::vector<int>& chunk_sizes,
                               std::vector<std::vector<int>>& buf_a_all,
                               std::vector<std::vector<int>>& buf_b_all,
                               std::vector<MPI_Request>& dispatch_reqs) {
    int d1_read = 0, d2_read = 0;

    for (int w = 1; w <= num_workers; ++w) {
        chunk_sizes[w] = get_worker_chunk_size(max_len, num_workers, w);

        dispatch_reqs.push_back(MPI_REQUEST_NULL);
        MPI_Isend(&chunk_sizes[w], 1, MPI_INT, w, TAG_CHUNK_SIZE, MPI_COMM_WORLD, &dispatch_reqs.back());

        if (chunk_sizes[w] > 0) {
            read_chunk_stream(file1, file2, chunk_sizes[w], n1, n2, d1_read, d2_read, buf_a_all[w], buf_b_all[w]);

            dispatch_reqs.push_back(MPI_REQUEST_NULL);
            MPI_Isend(buf_a_all[w].data(), chunk_sizes[w], MPI_INT, w, TAG_DIGITS_1, MPI_COMM_WORLD, &dispatch_reqs.back());

            dispatch_reqs.push_back(MPI_REQUEST_NULL);
            MPI_Isend(buf_b_all[w].data(), chunk_sizes[w], MPI_INT, w, TAG_DIGITS_2, MPI_COMM_WORLD, &dispatch_reqs.back());
        }
    }
}

static std::vector<int> async_gather_result_sizes(int num_workers) {
    std::vector<int> result_sizes(num_workers + 1, 0);
    std::vector<MPI_Request> size_reqs(num_workers + 1, MPI_REQUEST_NULL);

    for (int w = 1; w <= num_workers; ++w) {
        MPI_Irecv(&result_sizes[w], 1, MPI_INT, w, TAG_RESULT_SIZE, MPI_COMM_WORLD, &size_reqs[w]);
    }
    MPI_Waitall(num_workers, size_reqs.data() + 1, MPI_STATUSES_IGNORE);
    return result_sizes;
}

static void async_receive_and_stream_write(const std::string& out_path, int num_workers, const std::vector<int>& result_sizes) {
    std::vector<std::vector<int>> segment_results(num_workers + 1);
    std::vector<MPI_Request> data_reqs(num_workers + 1, MPI_REQUEST_NULL);

    // Post all non-blocking receives simultaneously
    for (int w = 1; w <= num_workers; ++w) {
        if (result_sizes[w] > 0) {
            segment_results[w].resize(result_sizes[w]);
            MPI_Irecv(segment_results[w].data(), result_sizes[w], MPI_INT, w, TAG_RESULT_DATA, MPI_COMM_WORLD, &data_reqs[w]);
        }
    }

    std::ofstream file3;
    std::streampos length_pos;
    if (!init_output_stream(out_path, file3, length_pos)) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    // Wait in strict rank order so chunks are streamed sequentially to disk
    int total_digits_written = 0;
    for (int w = 1; w <= num_workers; ++w) {
        if (result_sizes[w] > 0) {
            MPI_Wait(&data_reqs[w], MPI_STATUS_IGNORE);
            write_chunk_stream(file3, segment_results[w], total_digits_written);
        }
    }
    finalize_output_stream(file3, length_pos, total_digits_written);
}

static void master_workflow_async(const TestPaths& paths, int num_procs) {
    auto start_time = std::chrono::high_resolution_clock::now();

    std::ifstream file1, file2;
    int n1 = 0, n2 = 0, max_len = 0;
    if (!open_input_files(paths.num1, paths.num2, file1, file2, n1, n2, max_len)) {
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    int num_workers = num_procs - 1;
    std::vector<int> chunk_sizes(num_workers + 1, 0);
    std::vector<std::vector<int>> buf_a_all(num_workers + 1);
    std::vector<std::vector<int>> buf_b_all(num_workers + 1);
    std::vector<MPI_Request> dispatch_reqs;

    // 1. Stream-read inputs and dispatch asynchronously
    async_dispatch_all(file1, file2, n1, n2, max_len, num_workers, chunk_sizes, buf_a_all, buf_b_all, dispatch_reqs);
    file1.close();
    file2.close();

    // 2. Gather result sizes, then receive segments and stream to output file
    std::vector<int> result_sizes = async_gather_result_sizes(num_workers);
    async_receive_and_stream_write(paths.num3, num_workers, result_sizes);

    // 3. Ensure all outgoing buffers have cleared before destroying them
    MPI_Waitall(static_cast<int>(dispatch_reqs.size()), dispatch_reqs.data(), MPI_STATUSES_IGNORE);

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    print_benchmark(paths.test_name, num_procs, duration_ms);
}

// =========================================================================
// Worker (Rank > 0) Phase Helpers
// =========================================================================

static void async_receive_inputs(int rank, int chunk_size, std::vector<int>& local_a, std::vector<int>& local_b, int& carry_in) {
    local_a.resize(chunk_size);
    local_b.resize(chunk_size);
    carry_in = 0;

    std::vector<MPI_Request> recv_reqs;
    if (chunk_size > 0) {
        recv_reqs.push_back(MPI_REQUEST_NULL);
        MPI_Irecv(local_a.data(), chunk_size, MPI_INT, 0, TAG_DIGITS_1, MPI_COMM_WORLD, &recv_reqs.back());

        recv_reqs.push_back(MPI_REQUEST_NULL);
        MPI_Irecv(local_b.data(), chunk_size, MPI_INT, 0, TAG_DIGITS_2, MPI_COMM_WORLD, &recv_reqs.back());
    }
    if (rank > 1) {
        recv_reqs.push_back(MPI_REQUEST_NULL);
        MPI_Irecv(&carry_in, 1, MPI_INT, rank - 1, TAG_CARRY, MPI_COMM_WORLD, &recv_reqs.back());
    }

    MPI_Waitall(static_cast<int>(recv_reqs.size()), recv_reqs.data(), MPI_STATUSES_IGNORE);
}

static void async_send_outputs(int rank, int num_workers, int carry_out, const std::vector<int>& local_sum) {
    std::vector<MPI_Request> send_reqs;

    if (rank < num_workers) {
        send_reqs.push_back(MPI_REQUEST_NULL);
        MPI_Isend(&carry_out, 1, MPI_INT, rank + 1, TAG_CARRY, MPI_COMM_WORLD, &send_reqs.back());
    }

    int result_size = static_cast<int>(local_sum.size());
    send_reqs.push_back(MPI_REQUEST_NULL);
    MPI_Isend(&result_size, 1, MPI_INT, 0, TAG_RESULT_SIZE, MPI_COMM_WORLD, &send_reqs.back());

    if (result_size > 0) {
        send_reqs.push_back(MPI_REQUEST_NULL);
        MPI_Isend(local_sum.data(), result_size, MPI_INT, 0, TAG_RESULT_DATA, MPI_COMM_WORLD, &send_reqs.back());
    }

    // Must complete before local_sum and result_size go out of scope
    MPI_Waitall(static_cast<int>(send_reqs.size()), send_reqs.data(), MPI_STATUSES_IGNORE);
}

static void worker_workflow_async(int rank, int num_workers) {
    int chunk_size = 0;
    MPI_Recv(&chunk_size, 1, MPI_INT, 0, TAG_CHUNK_SIZE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    std::vector<int> local_a, local_b;
    int carry_in = 0;
    async_receive_inputs(rank, chunk_size, local_a, local_b, carry_in);

    std::vector<int> local_sum(chunk_size);
    int carry_out = compute_sum_chunk(local_a, local_b, local_sum, carry_in);
    if (rank == num_workers && carry_out > 0) {
        local_sum.push_back(carry_out);
    }

    async_send_outputs(rank, num_workers, carry_out, local_sum);
}

// =========================================================================
// Main Entry
// =========================================================================

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank = 0, num_procs = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 2) {
        if (rank == 0) std::cerr << "Error: Variant 3 requires at least 2 processes.\n";
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
        master_workflow_async(paths, num_procs);
    } else {
        worker_workflow_async(rank, num_procs - 1);
    }

    MPI_Finalize();
    return 0;
}