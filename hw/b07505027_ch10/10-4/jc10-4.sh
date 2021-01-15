#!/bin/sh
#PBS -P ACD109103
#PBS -N ch10
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=64
#PBS -l place=scatter
#PBS -l walltime=00:02:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR
date 
mpirun -n 1 ./Test 
mpirun -n 2 ./Test 
mpirun -n 3 ./Test 
mpirun -n 4 ./Test
mpirun -n 5 ./Test
mpirun -n 6 ./Test
mpirun -n 7 ./Test
mpirun -n 8 ./Test
mpirun -n 10 ./Test
mpirun -n 12 ./Test
mpirun -n 14 ./Test
mpirun -n 16 ./Test
mpirun -n 24 ./Test
mpirun -n 32 ./Test
mpirun -n 48 ./Test
mpirun -n 64 ./Test