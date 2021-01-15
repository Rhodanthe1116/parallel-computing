#include <assert.h>
#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int ROOT = 0;
const int INT_COUNT = 1;
const int DOUBLE_COUNT = 2;
const int MAX_PERFECT_COUNT = 8;

void assertm(bool ok, char msg[])
{
    if (!ok) {
        printf("Assertion failed: %s", msg);
        fflush(stdout);
        exit(1);
    }
};

int main(int argc, char* argv[])
{
    double elapsed_time; /* Time to find, count solutions */
    int id, p;
    MPI_Comm worker_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Barrier(MPI_COMM_WORLD);

    /* Avoid TLE */
    // alarm(3);

    /* Start timer */
    elapsed_time = -MPI_Wtime();

    /* Start Here */
    int n = 100;
    double s = 2; // edge length: given by problem
    double d = 0.3; // diameter: given by problem
    double v; // volumn

    int global_remain = 0;
    int global_total = n * n * n;

    int remain = 0;
    int total = global_total / p;

    srand48(time(NULL) + id);

    double x, y, z;
    double distance;
    for (int i = 0; i < total; i++) {
        x = (double)random() / (RAND_MAX + s);
        y = (double)random() / (RAND_MAX + s);
        z = (double)random() / (RAND_MAX + s);
        double sq = sqrt((x * x) + (y * y) + (z * z));
        double pre = (x + y + z) / (sqrt(3) * sq);
        distance = sin(acos(pre)) * sq;

        if (distance >= (d / 2.0)) {
            remain += 1;
        }
    }
    MPI_Reduce(&remain, &global_remain, INT_COUNT, MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    MPI_Finalize();

    // Print Result
    if (id == ROOT) {
        double fraction = ((double)global_remain / (double)global_total);
        v = fraction * s * s * s;

        printf("n^3:                %d^3 = %d      \n", n, n*n*n);
        printf("remain volumn:      %f      \n", v);
        printf("original volumn:    %f      \n", s * s * s);
        printf("remain / original:  %i / %d = %.5f\n", global_remain, global_total, fraction);
        printf("\n");

        printf("Process number: %d\t Max time: %8.6f\n", p, elapsed_time);
        fflush(stdout);

        printf("------------------------------------------\n");
        fflush(stdout);

        FILE* fp = fopen("./time.txt", "a+");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "Process number: %d\t, Max time: %8.6f\n", p, elapsed_time);
        fclose(fp);
    }

    return 0;
}