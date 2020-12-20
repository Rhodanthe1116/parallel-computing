#include <assert.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))
#define fff fflush(stdout);

const int ROOT = 0;
const int HANDLE_INPUT = 0;
const int INT_COUNT = 1;
const int DOUBLE_COUNT = 2;

void assertm(bool ok, char msg[])
{
    if (!ok) {
        printf("Assertion failed: %s", msg);
        fflush(stdout);
        exit(1);
    }
};

int** alloc_2d_int(int n, int m)
{
    int* data = (int*)malloc(n * m * sizeof(int));
    int** a = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++)
        a[i] = &(data[m * i]);

    return a;
}

bool is_range_safe(int i, int n)
{
    if (0 <= i && i < n) {
        return true;
    }
    return false;
}

bool is_safe(int i, int j, int n, int m)
{
    if (0 <= i && i < n && 0 <= j && j < m) {
        return true;
    }
    return false;
}

int cal_neighber(int i, int j, int n, int m, int** arr)
{

    int sum = 0;
    int dirs[8][2] = {
        { -1, -1 },
        { -1, 0 },
        { -1, 1 },
        { 0, -1 },
        { 0, 1 },
        { 1, -1 },
        { 1, 0 },
        { 1, 1 }
    };
    for (int d = 0; d < 8; d++) {
        int x = i + dirs[d][0];
        int y = j + dirs[d][1];
        if (is_safe(x, y, n, m)) {
            sum += arr[x][y];
        }
    }
    return sum;
}
int main(int argc, char* argv[])
{
    double elapsed_time; /* Time to find, count solutions */
    double max_elapsed_time = 0;
    int id; /* Process rank */
    int p; /* Number of processes */
    MPI_Status status;
    MPI_Request req[2];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    if (id == 0) {
        printf("Processer number: %d\n\n", p), fff;
    }
    /* Avoid TLE */
    // alarm(2);

    /* Start timer */
    elapsed_time = -MPI_Wtime();

    /* Start Here */

    int n, m; /* Dimension of array */
    int** a; /* Input array */
    int iterations, print_per_gen;
    int partition_height;

    if (id == ROOT) {
        assertm(argc == 3, "Please give me your iterations j and print_per_iter k\n");
    }
    iterations = atoi(argv[1]);
    print_per_gen = atoi(argv[2]);

    if (id == ROOT) {
        FILE* f = fopen("./life_input.txt", "r");
        assertm(f != NULL, "Please prepare ./life_input.txt\n");

        if (fscanf(f, "%d%d", &n, &m) != 2) {
            printf("Please input n, m (array dimension)\n");
            exit(1);
        }
        printf("n = %d, m = %d\n", n, m);

        a = alloc_2d_int(n, m);
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (fscanf(f, "%d", &a[i][j]) != 1) {
                    printf("Input array error");
                    exit(1);
                }
            }
        }
        fclose(f);

        // printf("Oringinal array.\n"), fff;
        // for (int i = 0; i < n; i++) {
        //     for (int j = 0; j < m; j++) {
        //         printf("%d ", a[i][j]);
        //         fflush(stdout);
        //     }
        //     printf("\n");
        //     fflush(stdout);
        // }
    }
    /*
    if partition_height = 2
    ------
    ------

    ------
    ------

    ------
    ------
    */

    MPI_Bcast(&n, INT_COUNT, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&m, INT_COUNT, MPI_INT, ROOT, MPI_COMM_WORLD);
    partition_height = BLOCK_SIZE(id, p, n);
    /* Please give me local index... */
    int** partition = alloc_2d_int(partition_height, m);
    // printf("id %d: height = %d, m = %d\n", id, partition_height, m);
    if (id == ROOT) {

        for (int pi = 0; pi < p; pi++) {

            // prepare partition
            int others_height = BLOCK_SIZE(pi, p, n);
            /* Please give me local index... */
            int** others_partition = alloc_2d_int(others_height, m);
            for (int i = 0; i < others_height; i++) {
                for (int j = 0; j < m; j++) {
                    // cut a slice from the the board
                    others_partition[i][j] = a[BLOCK_LOW(pi, p, n) + i][j];
                }
            }

            // and send it.
            if (pi == ROOT) {
                partition = others_partition;
            } else if (pi != ROOT) {
                MPI_Send(&(others_partition[0][0]), others_height * m, MPI_INT, pi, HANDLE_INPUT, MPI_COMM_WORLD);
            }
            // printf("Sended to PID %d!\n", pi);
            // fflush(stdout);
        }
    }
    if (id != ROOT) {
        MPI_Recv(&(partition[0][0]), partition_height * m, MPI_INT, ROOT, HANDLE_INPUT, MPI_COMM_WORLD, &status);
    }
    // printf("PID %d received!\n", id);
    // fflush(stdout);

    // printf("Start computing.......\n\n");

    int row_to_next[m];
    int row_to_prev[m];
    int row_from_next[m];
    int row_from_prev[m];
    const int COMM = 1;
    for (int iteration = 0; iteration < iterations; iteration++) {
        // printf("iteration %d\n", iteration);
        // printf("Communacating...\n");
        /*
            This part is for passing edge array data through processes.
        */
        // Send to next
        if (is_range_safe(id + 1, p)) {
            for (int j = 0; j < m; j++) {
                row_to_next[j] = partition[partition_height - 1][j];
            }
            MPI_Send(&row_to_next, m, MPI_INT, id + 1, COMM, MPI_COMM_WORLD);
        }

        // Receive from prev
        if (is_range_safe(id - 1, p)) {
            MPI_Recv(&row_from_prev, m, MPI_INT, id - 1, COMM, MPI_COMM_WORLD, &status);
        } else {
            for (int j = 0; j < m; j++) {
                row_from_prev[j] = 0;
            }
        }

        // Send to prev
        if (is_range_safe(id - 1, p)) {
            for (int j = 0; j < m; j++) {
                row_to_prev[j] = partition[0][j];
            }
            MPI_Send(&row_to_prev, m, MPI_INT, id - 1, COMM, MPI_COMM_WORLD);
        }

        // Receive from next
        if (is_range_safe(id + 1, p)) {
            MPI_Recv(&row_from_next, m, MPI_INT, id + 1, COMM, MPI_COMM_WORLD, &status);
        } else {
            for (int j = 0; j < m; j++) {
                row_from_next[j] = 0;
            }
        }

        /*
            This part is for checking and write new conditions to each cell.
        */

        int sum = 0; // sum of neighbours
        int** next_partition = alloc_2d_int(partition_height, m);
        for (int i = 0; i < partition_height; i++) {
            for (int j = 0; j < m; j++) {
                // First deal with corner cells.

                sum = cal_neighber(i, j, partition_height, m, partition);
                bool is_top_edge = i == 0;
                if (is_top_edge) {
                    if (is_range_safe(j - 1, m))
                        sum += row_from_prev[j - 1];
                    if (is_range_safe(j, m))
                        sum += row_from_prev[j];
                    if (is_range_safe(j + 1, m))
                        sum += row_from_prev[j + 1];
                }
                bool is_bottom_edge = i == (partition_height - 1);
                if (is_bottom_edge) {
                    if (is_range_safe(j - 1, m))
                        sum += row_from_next[j - 1];
                    if (is_range_safe(j, m))
                        sum += row_from_next[j];
                    if (is_range_safe(j + 1, m))
                        sum += row_from_next[j + 1];
                }

                // Write new condition of cells
                if (partition[i][j] == 1 && (sum == 2 || sum == 3))
                    next_partition[i][j] = 1;
                else if (partition[i][j] == 1 && sum > 3)
                    next_partition[i][j] = 0;
                else if (partition[i][j] == 1 && sum < 1)
                    next_partition[i][j] = 0;
                else if (partition[i][j] == 0 && sum == 3)
                    next_partition[i][j] = 1;
                else
                    next_partition[i][j] = 0;
            }
        }

        // Update partition.
        partition = next_partition;

        // printf("Returning result..........\n\n");
        fflush(stdout);
        const int RETURN = 2;
        if ((iteration + 1) % print_per_gen == 0) {
            if (id != ROOT) {
                MPI_Send(&(partition[0][0]), partition_height * m, MPI_INT, 0,
                    RETURN, MPI_COMM_WORLD);
            }

            if (id == ROOT) {
                printf("\nGeneration %d result!!!!!!!!!!!!!!!:\n", iteration + 1);
                fflush(stdout);

                for (int pi = 0; pi < p; pi++) {
                    int others_height = BLOCK_SIZE(pi, p, n);
                    int** others_partition = alloc_2d_int(others_height, m);

                    if (pi == ROOT) {
                        others_partition = partition;
                    } else if (pi != ROOT) {
                        MPI_Recv(&(others_partition[0][0]), others_height * m,
                            MPI_INT, pi, RETURN, MPI_COMM_WORLD, &status);
                    }

                    for (int i = 0; i < others_height; i++) {
                        for (int j = 0; j < m; j++) {
                            printf("%d ", others_partition[i][j]);
                            fflush(stdout);
                        }
                        printf("\n");
                        fflush(stdout);
                    }
                }
            }
            printf("\n");
            fflush(stdout);
        }
        // iteration ends.
    }

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();

    MPI_Reduce(&elapsed_time, &max_elapsed_time, DOUBLE_COUNT, MPI_DOUBLE, MPI_MAX, ROOT,
        MPI_COMM_WORLD);

    MPI_Finalize();

    // Print Result

    // printf("id %d: Execution time %8.6f\n", id, elapsed_time);
    // fflush(stdout);

    // Print Result
    if (id == ROOT) {
        printf("%d) Execution time %8.6f\n", id, elapsed_time);
        fflush(stdout);

        printf("Process number: %d\t, Max time: %8.6f\n", p, max_elapsed_time);
        fflush(stdout);

        printf("------------------------------------------\n");
        fflush(stdout);

        FILE* fp = fopen("./time.txt", "a");
        assertm(fp != NULL, "no time.txt\n");
        fprintf(fp, "Process number: %d\t, Max time: %8.6f\n", p, max_elapsed_time);
        fclose(fp);
    }

    return 0;
}
