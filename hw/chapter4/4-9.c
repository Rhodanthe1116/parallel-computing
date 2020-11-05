/* This program uses Simpson's Rule to compute pi. */
#include "mpi.h"
#include <stdio.h>
#include <stdbool.h>

#define fff fflush(stdout)
const int ROOT = 0;

bool is_prime(int n)
{
    if (n < 2)
        return false;
    for (int x = 2; x * x <= n; x++) {
        if (n % x == 0)
            return false;
    }
    return true;
};

int main(int argc, char* argv[])
{
    double elapsed_time; /* Time to find, count solutions */
    int id; /* Process rank */
    int p; /* Number of processes */
    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    if (id == 0) {
        printf("Processer number %d\n\n", p);
        fflush(stdout);
    }
    /* Start timer */
    elapsed_time = -MPI_Wtime();
    // Start Here

    int n = 1e3;
    int count = 0;
    int global_count = 0;
    bool prenum_is_prime = true;

    int block_size = n / p;
    int start_num = id * n / p + 1;
    int end_num = (id + 1) * n / p + 1;

    for (int num = start_num; num < end_num; num += 2) {
        bool num_is_prime = is_prime(num);
        if (prenum_is_prime && num_is_prime) {
            printf("%d %d", num - 2, num);
        }
    }

    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, ROOT,
        MPI_COMM_WORLD);

    printf("%d) count : %13.11f\n", id, count);
    fflush(stdout);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    printf("%d) Execution time %8.6f\n", id, elapsed_time);
    fflush(stdout);
    MPI_Finalize();

    // Print Result
    if (id == ROOT) {
        printf("answer : %d\n", global_count);
        fflush(stdout);
    }

    return 0;
}