#!/bin/sh
#PBS -P ACD109103
#PBS -N 5-11
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=5
#PBS -l place=scatter
#PBS -l walltime=00:01:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

mpicc -g -std=gnu99 ./5-11.c -o 5-11.out

date

mpirun -np 1 ./5-11.out   1000000 100
mpirun -np 2 ./5-11.out   1000000 100    
mpirun -np 4 ./5-11.out   1000000 100
mpirun -np 8 ./5-11.out   1000000 100
mpirun -np 16 ./5-11.out  1000000 100
mpirun -np 32 ./5-11.out  1000000 100
mpirun -np 64 ./5-11.out  1000000 100
mpirun -np 128 ./5-11.out 1000000 100 
mpirun -np 256 ./5-11.out 1000000 100 
mpirun -np 512 ./5-11.out 1000000 100 
mpirun -np 1024 ./5-11.out 1000000 100 
