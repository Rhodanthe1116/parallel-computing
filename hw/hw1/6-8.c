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
#include <stdio.h>

#define DATATYPE MPI_DOUBLE

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
    // Start Here
    assert(p == 2);

    /* Start timer */
    elapsed_time = -MPI_Wtime();

    int t = 1e2;
    int DUMMY_TAG = 0;
    double sent;
    double received;
    int size = sizeof(sent) / 4;

    for (int i = 0; i < t; i++) {
        sent = i;
        if (id == 0) {
            int destId = 1;
            
            MPI_Send(&sent, size, DATATYPE, destId, DUMMY_TAG, MPI_COMM_WORLD);
            MPI_Recv(&received, size, DATATYPE, destId, DUMMY_TAG, MPI_COMM_WORLD, &status);
        }
        if (id == 1) {
            int sourceId = 0;

            MPI_Recv(&received, size, DATATYPE, sourceId, DUMMY_TAG, MPI_COMM_WORLD, &status);
            MPI_Send(&received, size, DATATYPE, sourceId, DUMMY_TAG, MPI_COMM_WORLD);
        }
    }
    /* Stop timer */
    elapsed_time += MPI_Wtime();

    // meta
    if (id == 0) {
        printf("t = %d\n", t);
        fflush(stdout);
        printf("n-byte, n = %d\n", sizeof(sent));
        fflush(stdout);
        printf("Execution time %8.6f\n", elapsed_time);
        fflush(stdout);
        printf("Avg latency + (n / bandwith) = %8.10f\n", elapsed_time / (2 * t));
        fflush(stdout);
    }

    MPI_Finalize();

    return 0;
}