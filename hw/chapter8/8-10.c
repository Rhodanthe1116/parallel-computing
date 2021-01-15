/**
 * 
 * Write a program that implements matrix-vector multiplication based on a
 * checkerboard block decomposition of the matrix. The program should
 * read the matrix and the vector from an input hie and print the answer to
 * standard output. The names of the files containing the matrix and the
 * vector should be specihed as command-line arguments.
 *
 * 
 **/

/*
 *   Matrix-vector multiplication, Version 4
 *
 *   This program multiplies a matrix and a vector input from
 *   separate files. The result vector is printed to standard
 *   output.
 *
 *   Data distribution of matrix: checkerboard
 *   Data distribution of vector: blocked across procs in col 0
 *
 *   Programmed by Michael J. Quinn
 *
 *   Last modification by hw chang: 11 January 2020
 */

#include "./MyMPI.c"
#include <assert.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Change these two definitions when the matrix and vector
   element types change */

typedef double dtype;
#define mpitype MPI_DOUBLE

#define fff fflush(stdout);
const int ROOT = 0;

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
    dtype** a; /* First factor, a matrix */
    dtype* b; /* Second factor, a vector */
    int base;
    dtype* c_block; /* Partial product vector */
    dtype* c_sums;
    int cols;
    double max_seconds;
    double seconds; /* Elapsed time for matrix-vector multiply */
    dtype* storage; /* Matrix elements stored here */
    int grid_id;
    int grid_size[2]; /* Number of procs in each grid dimension */
    MPI_Comm grid_comm;
    MPI_Comm row_comm;
    MPI_Comm col_comm;
    dtype* tmp;
    int i, j; /* Loop indices */
    int id; /* Process ID number */
    int m; /* Rows in matrix */
    int n; /* Columns in matrix */
    int nprime; /* Elements in vector */
    int p; /* Number of processes */
    int rows; /* Number of rows on this process */
    int* recv_cnt;
    int* recv_disp;
    int* send_cnt;
    int* send_disp;
    int grid_coords[2];
    int periodic[2];
    int is_square;
    dtype* btrans;
    int send_els;
    int recv_els;
    int src;
    int dest;
    int coords[2];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    char* matrix_path = argv[1];
    char* vector_path = argv[2];

    if (id == ROOT) {
        printf("Program started.\n");

        assertm(argc == 3, "Please give me files.");

        printf("===== args =====\n"), fff;
        printf("Processer number: %d\n", p), fff;
        printf("matrix A path: %s\n", matrix_path), fff;
        printf("vector b path: %s\n", vector_path), fff;
        printf("===== args =====\n"), fff;
    }
    /* Avoid TLE */
    alarm(4);

    i = 1;
    while ((i * i) < p)
        i++;
    if (i * i == p)
        is_square = 1;
    else
        is_square = 0;

    grid_size[0] = grid_size[1] = 0;
    MPI_Dims_create(p, 2, grid_size);
    periodic[0] = periodic[1] = 0;
    MPI_Cart_create(MPI_COMM_WORLD, 2, grid_size, periodic, 1, &grid_comm);
    MPI_Comm_rank(grid_comm, &grid_id);
    MPI_Cart_coords(grid_comm, grid_id, 2, grid_coords);
    MPI_Comm_split(grid_comm, grid_coords[0], grid_coords[1], &row_comm);
    MPI_Comm_split(grid_comm, grid_coords[1], grid_coords[0], &col_comm);
    read_checkerboard_matrix(argv[1], (void*)&a,
        (void*)&storage, mpitype, &m, &n, grid_comm);
    rows = BLOCK_SIZE(grid_coords[0], grid_size[0], m);
    cols = BLOCK_SIZE(grid_coords[1], grid_size[1], n);

    /* Vector divided among processes in first column */

    if (grid_coords[1] == 0) {
        read_block_vector(vector_path, (void*)&b, mpitype, &nprime, col_comm);
    }

    c_block = (dtype*)malloc(rows * sizeof(dtype));
    c_sums = (dtype*)malloc(rows * sizeof(dtype));
    recv_els = BLOCK_SIZE(grid_coords[1], grid_size[1], n);
    btrans = (dtype*)malloc(recv_els * sizeof(dtype));
    tmp = (dtype*)malloc(n * sizeof(dtype));
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    seconds = -MPI_Wtime();

    /* Must get appropriate elements of b to procs */

    if (is_square) {

        /* Proc at (i,0) sends subvector to proc at (0,i) */
        /* Proc at (0,0) just does a copy. */

        if ((grid_coords[0] == 0) && (grid_coords[1] == 0)) {
            for (i = 0; i < recv_els; i++)
                btrans[i] = b[i];
        } else if ((grid_coords[0] > 0) && (grid_coords[1] == 0)) {
            send_els = BLOCK_SIZE(grid_coords[0], grid_size[0], n);
            coords[0] = 0;
            coords[1] = grid_coords[0];
            MPI_Cart_rank(grid_comm, coords, &dest);
            MPI_Send(b, send_els, mpitype, dest, 0, grid_comm);
        } else if ((grid_coords[1] > 0) && (grid_coords[0] == 0)) {
            coords[0] = grid_coords[1];
            coords[1] = 0;
            MPI_Cart_rank(grid_comm, coords, &src);
            MPI_Recv(btrans, recv_els, mpitype, src, 0, grid_comm, &status);
        }
    } else {
        /* Process at (0,0) gathers vector elements from procs in column 0 */

        if (grid_coords[1] == 0) {
            recv_cnt = (int*)malloc(grid_size[0] * sizeof(int));
            recv_disp = (int*)malloc(grid_size[0] * sizeof(int));
            for (i = 0; i < grid_size[0]; i++)
                recv_cnt[i] = BLOCK_SIZE(i, grid_size[0], n);
            recv_disp[0] = 0;
            for (i = 1; i < grid_size[0]; i++)
                recv_disp[i] = recv_disp[i - 1] + recv_cnt[i - 1];
            MPI_Gatherv(b, BLOCK_SIZE(grid_coords[0], grid_size[0], n),
                mpitype, tmp, recv_cnt, recv_disp, mpitype, 0, col_comm);
        }

        /* Process at (0,0) scatters vector elements to row 0 procs */

        if (grid_coords[0] == 0) {
            if (grid_size[1] > 1) {
                send_cnt = (int*)malloc(grid_size[1] * sizeof(int));
                send_disp = (int*)malloc(grid_size[1] * sizeof(int));
                for (i = 0; i < grid_size[1]; i++) {
                    send_cnt[i] = BLOCK_SIZE(i, grid_size[1], n);
                }
                send_disp[0] = 0;
                for (i = 1; i < grid_size[1]; i++) {
                    send_disp[i] = send_disp[i - 1] + send_cnt[i - 1];
                }
                MPI_Scatterv(tmp, send_cnt, send_disp, mpitype, btrans,
                    recv_els, mpitype, 0, row_comm);
            } else {
                for (i = 0; i < n; i++)
                    btrans[i] = tmp[i];
            }
        }
    }

    /* Row 0 procs broadcast their subvectors to procs in same column */
    MPI_Bcast(btrans, recv_els, mpitype, 0, col_comm);

    for (i = 0; i < rows; i++) {
        c_block[i] = 0.0;
        for (j = 0; j < cols; j++) {
            c_block[i] += a[i][j] * btrans[j];
        }
    }
    MPI_Reduce(c_block, c_sums, rows, mpitype, MPI_SUM, 0, row_comm);
    if (grid_coords[1] == 0) {
        print_block_vector(c_sums, mpitype, n, col_comm);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    seconds += MPI_Wtime();

    MPI_Reduce(&seconds, &max_seconds, 1, mpitype, MPI_MAX, 0,
        MPI_COMM_WORLD);
    if (!id) {
        printf("MV5) N = %d, Processes = %d, Time = %12.6f sec, ",
            n, p, max_seconds);
        printf("Mflop = %6.2f\n", 2 * n * n / (1000000.0 * max_seconds));

        FILE* fp = fopen("./time.txt", "a");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "N = %d\t, Process number: %d\t, Max time: %8.6f\n", n, p, max_seconds);
        fclose(fp);
    }
    MPI_Finalize();
    return 0;
}
