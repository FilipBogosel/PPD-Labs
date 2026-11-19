#include <iostream>
#include <cstdlib>
#include <random>
#include <thread>
#include <vector>
#include <functional>
#include <chrono>

const int n = 1000;

void create_vector(double *vec, double min, double max)
{
    for (int i = 0; i < n; i++)
        vec[i] = (max - min) * ((double)rand() / (double)RAND_MAX) + min;
}

void calcul(double *results, int start, int end, double *vec1, double *vec2, int thread_id)
{
    double sum{};
    for (int i = start; i < end; i++)
    {
        sum += vec1[i] * vec2[i];
    }
    results[thread_id] = sum;
}

int main(int argc, char **argv)
{
    int p = atoi(argv[1]);
    double a[n] = {}, b[n] = {};
    create_vector(a, 0, 100);
    create_vector(b, 0, 100);

    auto t1_seq = std::chrono::high_resolution_clock::now();

    double seq_results[1] = {};
    calcul(seq_results, 0, n, a, b, 0);
    double seq_scalar = seq_results[0];

    auto t2_seq = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential result: " << seq_scalar << "\n";
    std::cout << "Sequential time:   "
              << std::chrono::duration_cast<std::chrono::microseconds>(t2_seq - t1_seq).count()
              << " us\n\n";

    auto t1_par = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads(p);
    std::vector<double> par_results(p, 0.0); // one partial sum per thread

    int chunk_size = n / p;
    int remainder  = n % p;
    int start = 0;

    for (int i = 0; i < p; i++)
    {
        int add = (i < remainder) ? 1 : 0;
        int end = start + chunk_size + add;
        threads[i] = std::thread(calcul, par_results.data(), start, end, a, b, i);
        start = end;
    }

    for (int i = 0; i < p; i++)
        threads[i].join();

    double par_scalar = 0.0;
    for (int i = 0; i < p; i++)
        par_scalar += par_results[i];

    auto t2_par = std::chrono::high_resolution_clock::now();
    std::cout << "Parallel result:   " << par_scalar << "\n";
    std::cout << "Parallel time:     "
              << std::chrono::duration_cast<std::chrono::microseconds>(t2_par - t1_par).count()
              << " us\n";

    return 0;
}
