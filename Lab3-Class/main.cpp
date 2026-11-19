// • produs vectori (element cu element) 
// 	• procesul 0 distribuie segmente aprox egale celorlalte procese si apoi reconstruieste vectorul rezultat
// 	• varianta scatter - gather pentru aceeasi provlema
// • Produs scalar 
// 	•  varianta 1 distributie cu MPI_Scatter si apoi sumele locale se trimit catre procesul 0 care apoi le aduna 
// 		• varianta 1.1 sumele locale se primesc folosind MPI_Recv (blocant)
// 		• varianta 1.2 - sumele locale se primesc cu MPI_IRecv
// 	• varianta 2 MPI_Scatter si apoi MPI_Reduce
// Presupunem N divizibil p
 
 
#include <algorithm>
#include <cstdlib> 
#include <mpi.h>
#include <cstdio>
#include <random>
 
using namespace std;
 
const int N=4;
 
 
 
void generate(int *arr, int min, int max)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> dist(0.0, 1.0);
 
    for(int i=0;i<N;i++)
    {
 
        arr[i] = int(dist(gen) * (max - min) + min);
    }
}
 
void print_arr(int *arr)
{
    for(int i=0; i<N;i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}
 
using namespace std;
 
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int chunk_size = N / (size-1);
 
    int *arr1;
    int *arr2;
    int *final_result;
 
    if(rank==0)
    {
        arr1 = new int[N];
        arr2 = new int[N];
        final_result = new int[N];
 
        generate(arr1, 5,10);
        generate(arr2, 5,10);
 
        print_arr(arr1);
        print_arr(arr2);
 
        // for(int i=1; i<size;i++)
        // {
        //     MPI_Send(arr1 + (i-1) * chunk_size, chunk_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        //     MPI_Send(arr2 + (i-1) * chunk_size, chunk_size, MPI_INT, i, 0, MPI_COMM_WORLD);
 
        // }
 
        // for(int i=1;i<size;i++)
        // {
        //     MPI_Recv(final_res + (i-1) * chunk_size, chunk_size, MPI_INT, i,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // }
 
        // print_arr(final_res);
 
    }
 
    int *partial_arr1 = new int[chunk_size];
    int *partial_arr2 = new int[chunk_size];
    int *partial_res= new int[chunk_size];
    int partial_sum =0;
 
    MPI_Scatter(arr1, chunk_size, MPI_INT, partial_arr1, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(arr2, chunk_size, MPI_INT, partial_arr2, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
 
 
    for(int i=0; i< chunk_size;i++)
    {
        partial_res[i] = partial_arr1[i] * partial_arr2[i];
 
    }
 
    int final_sum = 0;
 
    MPI_Reduce(partial_res, &final_sum , chunk_size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
 
    if(rank==0)
    {
        printf("%d " , final_sum);
    }
 
 
 
    // if(rank!=0)
    // {
    //     MPI_Send(&partial_sum, 1, MPI_INT, 0 ,0 ,MPI_COMM_WORLD);
    // }
 
 
    // MPI_Gather(partial_res, chunk_size, MPI_INT, final_result, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
 
 
    // if(rank==0)
    // {   int final_sum =partial_sum;
 
 
    //     int partial_sum_received;
 
    //     for(int i=1; i<size;i++)
    //     {
 
    //         MPI_Recv(&partial_sum_received, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //         final_sum += partial_sum_received;
    //     }
 
    //     printf("%d\n" , final_sum);
    //     // print_arr(final_result);
    // }
 
    if(rank!=0)
    {
        // int *partial_arr1 = new int[chunk_size];
        // int *partial_arr2 = new int[chunk_size];
        // int *partial_res= new int[chunk_size];
 
        // MPI_Recv(partial_arr1, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // MPI_Recv(partial_arr2, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
 
        // for(int i=0;i<chunk_size;i++)
        // {
        //     partial_res[i] = partial_arr1[i] * partial_arr2[i];
        // }
 
        // MPI_Send(partial_res, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
 
    }
 
 
 
    MPI_Finalize();
 
 
    return 0;
}