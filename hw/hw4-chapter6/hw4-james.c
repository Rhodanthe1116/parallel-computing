#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))
#define MWIDTH 100
#define MHEIGHT 100

int** alloc_2d_int(int rows, int cols)
{
    int* data = (int*)malloc(rows * cols * sizeof(int));
    int** array = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++)
        array[i] = &(data[cols * i]);

    return array;
}

int main(int argc, char* argv[])
{
    MPI_Status Stat;
    MPI_Request req[2];
    int id, size;
    int m, n;
    int array[MWIDTH][MHEIGHT];
    int generations, printPerGen;
    int partitionWidth;
    double elapsed;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed = -MPI_Wtime();
    generations = atoi(argv[1]);
    printPerGen = atoi(argv[2]);

    if (id == 0) {
        FILE* f;
        if ((f = fopen("life_input.txt", "r")) == NULL)
            exit(1);

        if (fscanf(f, "%d%d", &m, &n) != 2)
            exit(1);

        for (int j = 0; j < m; j++)
            for (int i = 0; i < n; i++)
                if (fscanf(f, "%d", &array[i][j]) != 1)
                    exit(1);
        fclose(f);
        partitionWidth = m / (size - 1);

        printf("Oringinal array.\n");
        fflush(stdout);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                printf("%d ", array[i][j]);
                fflush(stdout);
            }
            printf("\n");
            fflush(stdout);
        }
    }

    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&partitionWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int** partition;
    partition = alloc_2d_int(partitionWidth, n);

    if (id == 0) {

        for (int z = 1; z < size; z++) {
            //printf("\nSend to pid %d.\n",z);
            fflush(stdout);
            for (int k = 0; k < partitionWidth; k++) {
                for (int l = 0; l < n; l++) {
                    partition[k][l] = array[k + ((z - 1) * partitionWidth)][l]; //cut a slice from the the board
                    //printf("%d ",partition[k][l]);
                    fflush(stdout);
                }
                //printf("\n");
                fflush(stdout);
            }
            MPI_Send(&(partition[0][0]), partitionWidth * n, MPI_INT, z, 1, MPI_COMM_WORLD); //and send it
            printf("PID %d Sended!\n", z);
            fflush(stdout);
        }

    } else {
        MPI_Recv(&(partition[0][0]), partitionWidth * n, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
        printf("PID %d received!\n", id);
        fflush(stdout);
    }

    /*
    // This part is for passing edge array data through processes.
    */
    int sendNext[n];
    int sendPrev[n];
    int getNext[n];
    int getPrev[n];

    for (int gen = 0; gen < generations; gen++) {
        if (id != 0) {
            // Other Processes: Low-edge send.
            if (id != size - 1) {
                for (int j = 0; j < n; j++) {
                    sendNext[j] = partition[partitionWidth - 1][j];
                }
                MPI_Send(&sendNext, n, MPI_INT, id + 1, 1, MPI_COMM_WORLD);
            }
            // Last process: Low-edge is NULL (0).
            else {
                for (int k = 0; k < n; k++) {
                    getNext[k] = 0;
                }
            }
            // Other processes: Up-edge receive.
            if (id != 1) {
                MPI_Recv(&getPrev, n, MPI_INT, id - 1, 1, MPI_COMM_WORLD, &Stat);
                for (int j = 0; j < n; j++) {
                    sendPrev[j] = partition[0][j];
                }
            }
            // First process: Up-edge is NULL (0).
            else {
                for (int k = 0; k < n; k++) {
                    getPrev[k] = 0;
                }
            }
            // Other processes: Low-edge receive.
            if (id != size - 1) {
                MPI_Recv(&getNext, n, MPI_INT, id + 1, 1, MPI_COMM_WORLD, &Stat);
            }
            // Other processes: Up-edge send.
            if (id != 1) {
                MPI_Send(&sendPrev, n, MPI_INT, id - 1, 1, MPI_COMM_WORLD);
            }

            /*
            // This part is for checking and write new conditions to each dot.
            */

            int sum = 0; // sum of neighbours
            int temp[partitionWidth][n];
            for (int x = 0; x < partitionWidth; x++) {
                for (int y = 0; y < n; y++) {
                    // First deal with corner cells.
                    if (x == 0 && y == 0) // upper-L
                        sum = partition[x + 1][y] + partition[x + 1][y + 1] + partition[0][y + 1] + getPrev[0] + getPrev[1];
                    else if (x == 0 && y == n - 1) // upper-R
                        sum = partition[x][y - 1] + partition[x + 1][y - 1] + partition[x + 1][y] + getPrev[n - 1] + getPrev[n - 2];
                    else if (x == partitionWidth - 1 && y == 0) // lower-L
                        sum = partition[x][y + 1] + partition[x - 1][y + 1] + partition[x - 1][y] + getNext[0] + getNext[1];
                    else if (x == partitionWidth - 1 && y == n - 1) // lower-R
                        sum = partition[x - 1][y] + partition[x - 1][y - 1] + partition[x][y - 1] + getNext[n - 1] + getNext[n - 2];
                    else // Other cells.
                    {
                        if (y == 0) // L-edge, except corner
                            sum = partition[x - 1][y] + partition[x - 1][y + 1] + partition[x][y + 1] + partition[x + 1][y + 1] + partition[x + 1][y];
                        else if (y == n - 1) // R-edge , except corner
                            sum = partition[x - 1][y] + partition[x - 1][y - 1] + partition[x][y - 1] + partition[x + 1][y - 1] + partition[x + 1][y];
                        else if (x == 0) // Up-edge, except corner
                            sum = partition[x][y - 1] + partition[x + 1][y - 1] + partition[x + 1][y] + partition[x + 1][y + 1] + partition[x][y + 1] + getPrev[y - 1] + getPrev[y] + getPrev[y + 1];
                        else if (x == partitionWidth - 1) // Low-edge, except corner
                            sum = partition[x - 1][y - 1] + partition[x - 1][y] + partition[x - 1][y + 1] + partition[x][y + 1] + partition[x][y - 1] + getNext[y - 1] + getNext[y] + getNext[y + 1];
                        else // General (finally, rest of the cells)
                            sum = partition[x - 1][y - 1] + partition[x - 1][y] + partition[x - 1][y + 1] + partition[x][y + 1] + partition[x + 1][y + 1] + partition[x + 1][y] + partition[x + 1][y - 1] + partition[x][y - 1];
                    }
                    // Write new condition of cells
                    if (partition[x][y] == 1 && (sum == 2 || sum == 3))
                        temp[x][y] = 1;
                    else if (partition[x][y] == 1 && sum > 3)
                        temp[x][y] = 0;
                    else if (partition[x][y] == 1 && sum < 1)
                        temp[x][y] = 0;
                    else if (partition[x][y] == 0 && sum == 3)
                        temp[x][y] = 1;
                    else
                        temp[x][y] = 0;
                }
            }
            // Write new data via temp.
            for (int x = 0; x < partitionWidth; x++) {
                for (int y = 0; y < n; y++) {
                    partition[x][y] = temp[x][y];
                }
            }
            MPI_Send(&(partition[0][0]), partitionWidth * n, MPI_INT, 0, 2, MPI_COMM_WORLD);
        } else {
            if ((gen+1) % printPerGen == 0) {
                if (id == 0) {
                    printf("\nGeneration %d result:\n", gen + 1);
                    fflush(stdout);
                    for (int i = 1; i < size; i++) {
                        MPI_Recv(&(partition[0][0]), partitionWidth * n, MPI_INT, i, 2, MPI_COMM_WORLD, &Stat); //receive all others'
                        for (int x = 0; x < partitionWidth; x++) {
                            for (int y = 0; y < n; y++) {
                                printf("%d ", partition[x][y]);
                                fflush(stdout);
                            }
                            printf("\n");
                            fflush(stdout);
                        }
                    }
                    printf("\n\n");
                    fflush(stdout);
                }
            }
        }

    } // generation ends.

    elapsed += MPI_Wtime();

    MPI_Finalize();
    if (id == 0) {
        printf("\n# Time elapsed: %f\n", elapsed);
        fflush(stdout);
    }
    return 0;
}