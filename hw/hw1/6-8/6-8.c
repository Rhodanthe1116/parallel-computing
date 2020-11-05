/*
 *   Circuit Satisfiability, Version 3
 *
 *   This version of the program prints the total number of
 *   solutions and the execution time.
 *
 *   Programmed by Michael J. Quinn
 *
 *   Last modification: 3 September 2002
 */

#include "mpi.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define DATATYPE MPI_INT
const int ROOT = 0;

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
    /* Avoid TLE */
    alarm(3);

    /* Start timer */
    elapsed_time = -MPI_Wtime();

    // Start Here

    if (id == ROOT) {
        assertm(p == 2, "p must equal 2!\n");
        assertm(argc == 3, "Please give me your iterations t and n-byte n k\n");
    }
    int t = atoi(argv[1]);
    const int n = atoi(argv[2]);

    int DUMMY_TAG = 0;
    int* sent = (int*)malloc(n * sizeof(int) / 4);
    int* received = (int*)malloc(n * sizeof(int) / 4);
    const int count = n / 4;
    for (int i = 0; i < t; i++) {
        if (id == 0) {
            int dest_id = 1;

            MPI_Send(&sent[0], count, DATATYPE, dest_id, DUMMY_TAG, MPI_COMM_WORLD);
            MPI_Recv(&received[0], count, DATATYPE, dest_id, DUMMY_TAG, MPI_COMM_WORLD, &status);
        }
        if (id == 1) {
            int source_id = 0;

            MPI_Recv(&received[0], count, DATATYPE, source_id, DUMMY_TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&received[0], count, DATATYPE, source_id, DUMMY_TAG, MPI_COMM_WORLD);
        }
    }
    /* Stop timer */
    elapsed_time += MPI_Wtime();

    // meta
    if (id == 0) {
        printf("t = %d\n", t);
        fflush(stdout);
        printf("n-byte, n = %d\n", n);
        fflush(stdout);
        printf("Execution time %8.6f\n", elapsed_time);
        fflush(stdout);
        printf("Avg latency + (n / bandwith) = %8.10f\n", elapsed_time / (2 * t));
        fflush(stdout);
    }

    MPI_Finalize();

    return 0;
}