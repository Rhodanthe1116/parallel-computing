#!/bin/sh
#PBS -P ACD109103
#PBS -N 6-13
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=5
#PBS -l place=scatter
#PBS -l walltime=00:01:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

mpicc -g -std=gnu99 ./6-13.c -o 6-13.out

date

mpirun -np 1 ./6-13.out   10 10
mpirun -np 2 ./6-13.out   10 10    
mpirun -np 4 ./6-13.out   10 10
mpirun -np 8 ./6-13.out   10 10
mpirun -np 16 ./6-13.out  10 10
mpirun -np 32 ./6-13.out  10 10
mpirun -np 64 ./6-13.out  10 10
mpirun -np 128 ./6-13.out 10 10 

