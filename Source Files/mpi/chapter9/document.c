#include <ftw.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
 
#define DICT_SlZE_MSG 0 /* Msg has dictionar y siz e */
#define FTLE_NAME_MSG 1 /* Msg i s fil e name */
#define VECTOR_MSG 2    /* Msg i s profil e */
#define EMPTY_MSG 3     /* Msg i s empty */
#define DTR_ARG 1       /* Director y argumen t */
#define D1CT_ARG 2      /* Dictionar y argument */
#define RES_ARC 3       /* Result s argument */

typedef unsigned char uchar;

const int ROOT = 0;

int main(int argc, char *argv[]) {
  int id;               /* Process rank */
  int p;                /* Number o f processe s */
  MPI_Comm worker_comm; /* Workers-onl y communicato r */
  void manager(int, char **, int);
  void worker(int, char **, MPI_Comm);
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