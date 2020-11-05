#!/bin/sh
#PBS -P ACD109103
#PBS -N 4-12
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=5
#PBS -l place=scatter
#PBS -l walltime=00:01:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

mpicc -g -std=gnu99 ./4-12.c -o 4-12.out

date

mpirun -np 1 ./4-12.out  
mpirun -np 2 ./4-12.out      
mpirun -np 3 ./4-12.out  
mpirun -np 4 ./4-12.out  
mpirun -np 5 ./4-12.out 
mpirun -np 6 ./4-12.out 
mpirun -np 7 ./4-12.out 
mpirun -np 8 ./4-12.out 
