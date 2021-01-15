#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[])
{
    int niter = 100000;
    double s = 2;
    double d = 0.3;
    int id, p;
    double x, y, z;
    int count = 0;
    double dist;
    double v;
    int rcount;
    int rtotal;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    double elapsed = -MPI_Wtime();
    
    int total = niter / p;
    for (int i = 0; i < total; i++) {
        srand48(time(NULL) + id);
        x = (double)random() / (RAND_MAX + s);
        y = (double)random() / (RAND_MAX + s);
        z = (double)random() / (RAND_MAX + s);
        double sq = sqrt((x * x) + (y * y) + (z * z));
        double pre = (x + y + z) / (sqrt(3) * sq);
        dist = sin(acos(pre)) * sq;
        if (dist >= (d / 2.0)) {
            ++count;
        }
    }
    MPI_Reduce(&count, &rcount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&total, &rtotal, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed += MPI_Wtime();
    if (id == 0) {
        v = ((double)rcount / (double)rtotal) * s * s * s;
        printf("V: %f  %i / %d\n", v, rcount, rtotal);
        printf("[BENCHMARK] Processes = %d, Time = %f sec.\n", p, elapsed);
    }

    MPI_Finalize();

    return 0;
}