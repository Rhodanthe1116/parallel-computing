#!/bin/sh
#PBS -P ACD109103
#PBS -N 6-8
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=2
#PBS -l place=scatter
#PBS -l walltime=00:01:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

mpicc -g -std=gnu99 ./6-8.c -o 6-8.out

date

# mpirun ./6-8.out 10 8
# mpirun ./6-8.out 100 8
# mpirun ./6-8.out 1000 8
# mpirun ./6-8.out 10000 8
# mpirun ./6-8.out 100000 8
# mpirun ./6-8.out 1000000 8
# mpirun ./6-8.out 10000000 8

mpirun ./6-8.out 10000 8
mpirun ./6-8.out 10000 100
mpirun ./6-8.out 10000 1000
mpirun ./6-8.out 10000 10000
mpirun ./6-8.out 10000 100000
mpirun ./6-8.out 10000 1000000