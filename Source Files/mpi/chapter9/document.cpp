#include <ftw.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define DICT_SIZE_MSG 0 /* Msg has dictionar y siz e */
#define FILE_NAME_MSG 1 /* Msg i s fil e name */
#define VECTOR_MSG 2 /* Msg i s profile */
#define EMPTY_MSG 3 /* Msg i s empty */
#define DIR_ARG 1 /* Director y argumen t */
#define DICT_ARG 2 /* Dictionar y argument */
#define RES_ARG 3 /* Result s argument */

typedef unsigned char uchar;

const int ROOT = 0;

int main(int argc, char* argv[])
{
    int id; /* Process rank */
    int p; /* Number o f processe s */
    MPI_Comm worker_comm; /* Workers-onl y communicato r */
    void manager(int, char**, int);
    void worker(int, char**, MPI_Comm);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 4) {
        if (id == ROOT) {
            printf("Program needs thre e arguments:\n");
            printf("%s <dir > <dict > <results>\n ", argv[0]);
        }
    } else if (p < 2) {
        printf("Program needs at least two processes\n");
    } else {
        if (id == ROOT) {
            MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, id, &worker_comm);
            manager(argc, argv, p);
        } else {
            MPI_Comm_split(MPI_COMM_WORLD, 0, id, &worker_comm);
            worker(argc, argv, worker_comm);
        }
    }
    MPI_Finalize();
    return 0;
}

void manager(int argc, char* argv[], int p)
{
    int assign_cnt; /* Docs assigne d so fa r */
    int* assigned; /* Document assignment s */
    uchar* buffer; /* Stor e profile vector s her e */
    int dict_size; /* Dictionar y entrie s */
    int file_cnt; /* Plaintex t file s foun d */
    char** file_name; /* Store s fil e (path ) names */
    int i;
    MPI_Request pending; /* Handl e to r rec v reques t */
    int src; /* Message sourc e proces s */
    MPI_Status status; /* Message statu s */
    int tag; /* Message ta g */
    int terminated; /* Count o f terminated proc s */
    uchar** vector; /* profile vecto r repositor y */

    void build_2d_array(int, int, uchar***);
    void get_names(char*, char***, int*);
    void write_proflies(char*, int, int, char**, uchar**);

    /* Put i n reques t t o receiv e dictionar y siz e */
    MPI_Irecv(&dict_size, 1, MPI_INT, MPI_ANY_SOURCE, DICT_SIZE_MSG,
        MPI_COMM_WORLD, &pending);

    /* Collec t th e names o f th e documents t o be profile d */
    get_names(argv[DIR_ARG], &file_name, &file_cnt);
    /* Wai t to r dictionar y siz e t o be receive d */
    MPI_Wait(&pending, &status);
    /* Set asid e butte r t o catc h profile s fro m worlter s */
    buffer = (uchar*)malloc(dict_size * sizeof(MPI_UNSIGNED_CHAR));
    /* Set aside 2-D array t o hold all profiles .
    Call MPI_Abort if the allocation fails . */
    build_2d_array(file_cnt, dict_size, &vector);
    /* Respond to requests by workers . */
    terminated = 0;
    assign_cnt = 0;
    assigned = (int*)malloc(p * sizeof(int));
    do {
        /* Get profile fro m worfcer */
        MPI_Recv(buffer, dict_size, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG,
            MPI_COMM_WORLD, &status);
        src = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        if (tag == VECTOR_MSG) {
            for (int i = 0; i < dict_size; i++) {
                vector[assigned[src]][i];
            }
        }
        /* Assig n more wor k o r tel l worke r t o stop . */
        if (assign_cnt < file_cnt) {
            MPI_Send(
                file_name[assign_cnt],
                strlen(file_name[assign_cnt]) + 1,
                MPI_CHAR, src,
                FILE_NAME_MSG,
                MPI_COMM_WORLD);
            assigned[src] = assign_cnt;
            assign_cnt++;
        } else {
            MPI_Send(NULL, 0, MPI_CHAR, src, FILE_NAME_MSG,
                MPI_COMM_WORLD);
            terminated++;
        }
    } while (terminated < (p - 1));

    write_profiles(argv[RES_ARG], file_cnt, dict_size,
        file_name, vector);
}

void worker(int argc, char* argv[], MPI_Comm worker_comm)
{
    char* buffer; /* Words i n dictionar y */
    hash_el* dict; /* Hash tabl e o f words */
    int dict_size; /* profile vecto r siz e */
    long file_len; /* Chars i n dictionar y */
    int i;
    char* name; /* Name o f plaintex t til e */
    int name_len; /* Chars intil e name */
    MPI_Request pending; /* Handl e to r MPI_Isen d */
    uchar* profile; /* Document profile vecto r */
    MPI_Status status; /* Int o abou t message */
    int worker_id; /* Rank i n worker_comm */

    void build_hash_table(char*, int, hash_el***, int*);
    void make_profile(char*, hash_el**, int, uchar*);
    void read_dictionary(char*, char**, long*);

    /* Worker gets its worker ID number */
    MPI_Comm_rank(worker_comm, &worker_id);
    /* Worker makes initia l reques t to r wor k */
    MPI_Isend(NULL, 0, MPI_UNSIGNED_CHAR, 0, EMPTY_MSG,
        MPI_COMM_WORLD, &pending);
    /* Read and broadcast dictionary file */
    if (!worker_id) {
        read_dictionary(argv[DICT_ARG], &buffer, &file_len);
    }
    MPI_Bcast(&file_len, 1, MPI_LONG, 0, worker_comm);
    if (worker_id) {
        buffer = (char*)malloc(file_len);
    }
    MPI_Bcast(buffer, file_len, MPI_CHAR, 0, worker_comm);
    /* Build hash table */
    build_hash_table(buffer, file_len, &dict, &dict_size);
    profile = (uchar*)malloc(dict_size * sizeof(uchar));
    /* Worker 0 sends msg to manager re : size of dictionary */
    if (!worker_id) {
        MPI_Send(&dict_size, 1, MPI_INT, 0,
            DICT_SIZE_MSG, MPI_COMM_WORLD);
    }

    for (;;) {
        /* Find out length of file name */
        MPI_Probe(0, FILE_NAME_MSG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &name_len);
        /* Drop out if no more work */

        if (!name_len)
            break;

        name = (char*)malloc(name_len);
        MPI_Recv(name, name_len, MPI_CHAR, 0, FILE_NAME_MSG,
            MPI_COMM_WORLD, &status);
        make_profile(name, dict, dict_size, profile);
        free(name);
        MPI_Send(profile, dict_size, MPI_UNSIGNED_CHAR, 0,
            VECTOR_MSG, MPI_COMM_WORLD);
    }
}
