#!/bin/sh
#PBS -P ACD109103
#PBS -N 8-10
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=5
#PBS -l place=scatter
#PBS -l walltime=00:01:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

mpicc -g -std=gnu99 ./8-10.c -o 8-10.out

date

N=10000

./gen-double-matrix.out $N A
./gen-vector.out $N b

mpirun -np 1 ./8-10.out   A b
mpirun -np 2 ./8-10.out   A b    
mpirun -np 4 ./8-10.out   A b
mpirun -np 8 ./8-10.out   A b
mpirun -np 16 ./8-10.out  A b
mpirun -np 32 ./8-10.out  A b
mpirun -np 64 ./8-10.out  A b
mpirun -np 128 ./8-10.out A b 

