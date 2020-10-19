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

#define datatype MPI_LONG_DOUBLE

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

    int n = 1e2;
    int tag = 0;
    int sent;
    int received;

    for (int i = 0; i < n; i++) {
        sent = i;
        if (id == 0) {
            int destId = 1;

            MPI_Send(&sent, 1, datatype, destId, tag, MPI_COMM_WORLD);
            MPI_Recv(&received, 1, datatype, destId, tag, MPI_COMM_WORLD, &status);
        }
        if (id == 1) {
            int sourceId = 0;

            MPI_Recv(&received, 1, datatype, sourceId, tag, MPI_COMM_WORLD, &status);
            // printf("Received %d\n", received);
            // fflush(stdout);
            MPI_Send(&received, 1, datatype, sourceId, tag, MPI_COMM_WORLD);
        }
    }
    /* Stop timer */
    elapsed_time += MPI_Wtime();

    // meta
    if (id == 0) {
        printf("n = %d\n", n);
        fflush(stdout);
        printf("bandwith = %d\n", sizeof(sent));
        fflush(stdout);
        printf("Execution time %8.6f\n", elapsed_time);
        fflush(stdout);
        printf("Avg latency + (n / bandwith) = %8.10f\n", elapsed_time / (2 * n));
        fflush(stdout);
    }

    MPI_Finalize();

    return 0;
}