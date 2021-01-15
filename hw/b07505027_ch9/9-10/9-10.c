#include <assert.h>
#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define FAIL_MSG 3
#define EMPTY_MSG 0
#define SUCCESS_MSG 1
#define TASK_NUMBER_MSG 2

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

bool is_prime(long long n)
{
    if (n < 2)
        return false;
    for (long long x = 2; x * x <= n; x += 1) {
        if (n % x == 0)
            return false;
    }
    return true;
};

void manager(int);
void worker(int, int);

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

    if (id == ROOT) {
        manager(p);
    } else {
        worker(p, id);
    }

    // Benchmark and Finalize
    /* Stop timer */
    elapsed_time += MPI_Wtime();
    MPI_Finalize();
    // Print Result
    if (id == ROOT) {
        printf("%d) Execution time %8.6f\n", id, elapsed_time);
        fflush(stdout);

        printf("Process number: %d\t, Max time: %8.6f\n", p, elapsed_time);
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

void manager(int p)
{
    int perfect_count = 0;
    int current_n = 0;
    int terminated = 0;
    int res_n;
    long long perfect;
    int src, tag;
    MPI_Status status;

    while (terminated < (p - 1)) {
        MPI_Recv(&res_n, INT_COUNT, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        if (tag == SUCCESS_MSG && perfect_count < MAX_PERFECT_COUNT) {
            perfect_count += 1;
            perfect = (pow(2, res_n) - 1) * (pow(2, res_n - 1));
            printf("n = %d, the %d perfect number is: %lld\n", res_n, perfect_count, perfect);
            fflush(stdout);
        }

        if (perfect_count < MAX_PERFECT_COUNT) {
            MPI_Send(&current_n, INT_COUNT, MPI_INT, src, TASK_NUMBER_MSG, MPI_COMM_WORLD);
            current_n += 1;
        } else {
            // printf("id %d terminated\n", src);
            // fflush(stdout);
            MPI_Send(NULL, 0, MPI_INT, src, TASK_NUMBER_MSG, MPI_COMM_WORLD);
            terminated += 1;
        }
    }

    printf("Verified n range: 1 ~ %d\n", current_n);
}

void worker(int p, int id)
{
    MPI_Status status;
    int data_size;
    int test_n;
    MPI_Send(NULL, 0, MPI_INT, 0, EMPTY_MSG, MPI_COMM_WORLD);
    while (true) {
        MPI_Probe(0, TASK_NUMBER_MSG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &data_size);
        fflush(stdout);
        if (!data_size)
            break;
        MPI_Recv(&test_n, INT_COUNT, MPI_INT, ROOT, TASK_NUMBER_MSG, MPI_COMM_WORLD, &status);

        // printf("%d) Calculating n = %d\n", id, test_n);
        // fflush(stdout);
        if (is_prime(pow(2, test_n) - 1)) {
            MPI_Send(&test_n, INT_COUNT, MPI_INT, ROOT, SUCCESS_MSG, MPI_COMM_WORLD);
        } else {
            test_n = -1;
            MPI_Send(&test_n, INT_COUNT, MPI_INT, ROOT, FAIL_MSG, MPI_COMM_WORLD);
        }
    }
}
