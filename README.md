# parallel-computing

```bash
module load intel/2018_u1
mpicc -o Test mpihello.c
qsub test.sh
qstat 1111111.srvc1
more 1111111.srvc1
```

```bash
module load anaconda3/5.1.10
conda create --prefix /home/hwww1116/condapy3 python=3 anaconda
```