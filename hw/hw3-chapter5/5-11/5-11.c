#include "mpi.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

const int ROOT = 0;
const int DUMMY_TAG = 87;
const int MAX_TIME_TAG = 777;
const int INT_COUNT = 1;
const int DOUBLE_COUNT = 2;
void assertm(bool ok, char msg[])
{
    if (!ok) {
        printf("Assertion failed: %s\n", msg);
        fflush(stdout);
        exit(1);
    }
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

    if (id == ROOT) {
        assertm(argc == 3, "Please give me your Sn n and precision d\n");
    }
    int n = atoi(argv[1]);
    int d = atoi(argv[2]);
    MPI_Bcast(&n, INT_COUNT, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&d, INT_COUNT, MPI_INT, ROOT, MPI_COMM_WORLD);

    double value = 0;
    double global_value = 0;

    for (double i = id + 1; i <= n; i += p) {
        value += 1.0 / i;
    }

    MPI_Reduce(&value, &global_value, DOUBLE_COUNT, MPI_DOUBLE, MPI_SUM, ROOT,
        MPI_COMM_WORLD);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();

    MPI_Reduce(&elapsed_time, &max_elapsed_time, DOUBLE_COUNT, MPI_DOUBLE, MPI_MAX, ROOT,
        MPI_COMM_WORLD);

    MPI_Finalize();
    
    // Print Result

    printf("id %d: Execution time %8.6f\n", id, elapsed_time);
    fflush(stdout);

    printf("id %d: Value: %13.11f\n", id, value);
    fflush(stdout);

    if (id == ROOT) {
        printf("Sn: %.*f\n", d, global_value);
        fflush(stdout);

        printf("Process number: %d\t, Max time: %8.6f\t, Answer: %.*f\n", p, max_elapsed_time, d, global_value);
        fflush(stdout);

        FILE* fp = fopen("./output.txt", "a");
        assertm(fp != NULL, "no output.txt\n");
        fprintf(fp, "Process number: %d\t, Max time: %8.6f\t, Answer: %.*f\n", p, max_elapsed_time, d, global_value);
        fclose(fp);

        fp = fopen("./time.csv", "a");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "%d,%8.6f\n", p, max_elapsed_time);
        fclose(fp);
    }

    return 0;
}