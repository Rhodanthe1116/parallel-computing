#!/bin/sh
#PBS -P ACD109103
#PBS -N 9-10
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=64
#PBS -l place=scatter
#PBS -l walltime=00:00:10
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

FILE=9-10

mpicc -g -std=gnu99 ./$FILE.c -o ./$FILE.out -lm

mpirun -np 1 ./$FILE.out   
mpirun -np 2 ./$FILE.out   
mpirun -np 4 ./$FILE.out   
mpirun -np 8 ./$FILE.out   
mpirun -np 16 ./$FILE.out  
mpirun -np 32 ./$FILE.out  
mpirun -np 64 ./$FILE.out  
# mpirun -np 128 ./$FILE.out 

