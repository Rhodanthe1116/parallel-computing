/* This program uses Simpson's Rule to compute pi. */
#include "mpi.h"
#include <stdio.h>

#define fff fflush(stdout)
const int n = 100000;
const int ROOT = 0;

bool isPrime(int n)
{
    if (n < 2)
        return false;
    for (int x = 2; x * x <= n; x++) {
        if (n % x == 0)
            return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    // Basic
    int id; /* Process rank */
    int p; /* Number of processes */
    double elapsed_time; /* Time to find, count solutions */
    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    /* Start timer */
    elapsed_time = -MPI_Wtime();

    double area = 0; /* Solutions found by this proc */
    double global_area = f(0) - f(n); /* Total number of solutions */

    for (int i = id; i <= n / 2; i += p) {
        area += 4.0 * f(2 * i - 1) + 2 * f(2 * i);
    }
    printf("%d) area : %13.11f\n", id, area);
    fflush(stdout);

    MPI_Reduce(&area, &global_area, 1, MPI_DOUBLE, MPI_SUM, ROOT,
        MPI_COMM_WORLD);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    printf("%d) Execution time %8.6f\n", id, elapsed_time);
    fflush(stdout);
    MPI_Finalize();

    // Print Result
    if (id == ROOT) {
        global_area /= (3.0 * n);
        printf("Approximation of pi : %13.11f\n", global_area);
        fflush(stdout);
    }

    return 0;
}