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

    double max_elapsed_time = 0;
    int n = 1e6;
    int count = 0;
    int global_count = 0;

    int block_size = n / p;
    if (id == ROOT) {
        assertm(n % p == 0, "Please let p can divide n~~~~~~~~~~~~~~~`");
        assertm(block_size % 2 == 0, "Please let n / p be even~~~~~~~~~~~~~~~`");
    }
    int start_num = id * block_size + 1;
    int end_num = (id + 1) * block_size + 1;
    bool prenum_is_prime = is_prime(start_num - 2);

    for (int num = start_num; num < end_num; num += 2) {
        bool num_is_prime = is_prime(num);
        if (prenum_is_prime && num_is_prime) {
            // printf("%d %d\n", num - 2, num);
            count += 1;
        }
        prenum_is_prime = num_is_prime;
    }

    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, ROOT,
        MPI_COMM_WORLD);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    MPI_Reduce(&elapsed_time, &max_elapsed_time, 2, MPI_DOUBLE, MPI_MAX, ROOT,
        MPI_COMM_WORLD);

    MPI_Finalize();

    printf("id %d: Execution time %8.6f\n", id, elapsed_time);
    fflush(stdout);

    printf("id %d: count : %d\n", id, count);
    fflush(stdout);
    // Print Result
    if (id == ROOT) {
        printf("Process number: %d\t, Max time: %8.6f\t, Answer: %d\n", p, max_elapsed_time, global_count);
        fflush(stdout);

        FILE* fp = fopen("./output.txt", "a");
        assertm(fp != NULL, "no output.txt\n");
        fprintf(fp, "Process number: %d\t, Max time: %8.6f\t, Answer: %d\n", p, max_elapsed_time, global_count);
        fclose(fp);

        fp = fopen("./time.csv", "a");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "%d,%8.6f\n", p, max_elapsed_time);
        fclose(fp);
    }

    return 0;
}