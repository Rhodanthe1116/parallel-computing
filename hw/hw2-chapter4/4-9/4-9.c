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

    int n = 1e6;
    int local_max_gap = 0;
    int global_max_gap = 0;
    const int NOT_YET = -1;
    int preprime = NOT_YET;

    int block_size = n / p;
    if (id == ROOT) {
        assertm(n % p == 0, "Please let p can divide n~~~~~~~~~~~~~~~`");
        assertm(block_size % 2 == 0, "Please let n / p be even~~~~~~~~~~~~~~~`");
    }
    int start_num = id * block_size + 1;
    int end_num = (id + 1) * block_size + 1;

    for (int num = start_num; preprime <= end_num; num += 2) {
        if (is_prime(num)) {
            if (preprime == NOT_YET) {
                preprime = num;
                continue;
            }
            int gap = num - preprime;
            if (gap > local_max_gap) {
                local_max_gap = gap;
            }
            preprime = num;
        }
    }

    MPI_Reduce(&local_max_gap, &global_max_gap, 1, MPI_INT, MPI_MAX, ROOT,
        MPI_COMM_WORLD);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    MPI_Reduce(&elapsed_time, &max_elapsed_time, 2, MPI_DOUBLE, MPI_MAX, ROOT,
        MPI_COMM_WORLD);

    MPI_Finalize();

    printf("id %d: local_max_gap : %d\n", id, local_max_gap);
    fflush(stdout);

    printf("id %d: Execution time %8.6f\n", id, elapsed_time);
    fflush(stdout);

    // Print Result
    if (id == ROOT) {
        printf("answer : %d\n", global_max_gap);
        fflush(stdout);
        printf("Process number: %d\t, Max time: %8.6f\t, Answer: %d\n", p, max_elapsed_time, global_max_gap);
        fflush(stdout);

        FILE* fp = fopen("./output.txt", "a");
        assertm(fp != NULL, "no output.txt\n");
        fprintf(fp, "Process number: %d\t, Max time: %8.6f\t, Answer: %d\n", p, max_elapsed_time, global_max_gap);
        fclose(fp);

        fp = fopen("./time.csv", "a");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "%d,%8.6f\n", p, max_elapsed_time);
        fclose(fp);
    }

    return 0;
}