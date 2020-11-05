/* This program uses Simpson's Rule to compute pi. */
#include "mpi.h"
#include <stdbool.h>
#include <stdio.h>

#define fff fflush(stdout)
const int ROOT = 0;

void assertm(bool ok, char msg[])
{
    if (!ok) {
        printf("Assertion failed: %s\n", msg);
        fflush(stdout);
        exit(1);
    }
};

const int n = 50;

double f(int i)
{
    double x;
    x = (double)i / (double)n;
    return 4.0 / (1.0 + x * x);
}

int main(int argc, char* argv[])
{
    double elapsed_time; /* Time to find, count solutions */
    double max_elapsed_time = 0;
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

    /* Avoid TLE */
    alarm(3);

    // Start Here

    double area = 0; /* Solutions found by this proc */
    double global_area = f(0) - f(n);

    for (int i = id; i <= n / 2; i += p) {
        area += 4.0 * f(2 * i - 1) + 2 * f(2 * i);
    }

    MPI_Reduce(&area, &global_area, 1, MPI_DOUBLE, MPI_SUM, ROOT,
        MPI_COMM_WORLD);
        
    global_area /= (3.0 * n);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    MPI_Reduce(&elapsed_time, &max_elapsed_time, 2, MPI_DOUBLE, MPI_MAX, ROOT,
        MPI_COMM_WORLD);

    MPI_Finalize();

    printf("id %d: Execution time %8.6f\n", id, elapsed_time);
    fflush(stdout);

    printf("id %d: Area: %13.11f\n", id, area);
    fflush(stdout);

    // Print Result
    if (id == ROOT) {
        printf("Approximation of pi : %13.11f\n", global_area);
        fflush(stdout);

        printf("Process number: %d\t, Max time: %8.6f\t, Answer: %13.11f\n", p, max_elapsed_time, global_area);
        fflush(stdout);

        FILE* fp = fopen("./output.txt", "a");
        assertm(fp != NULL, "no output.txt\n");
        fprintf(fp, "Process number: %d\t, Max time: %8.6f\t, Answer: %13.11f\n", p, max_elapsed_time, global_area);
        fclose(fp);

        fp = fopen("./time.csv", "a");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "%d,%8.6f\n", p, max_elapsed_time);
        fclose(fp);
    }

    return 0;
}