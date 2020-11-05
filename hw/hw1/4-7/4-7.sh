#!/bin/sh
#PBS -P ACD109103
#PBS -N 4-7
#PBS -q ctest
#PBS -l select=1:ncpus=1:mpiprocs=10
#PBS -l place=scatter
#PBS -l walltime=00:01:00
#PBS -j n
module purge
module load intel/2018_u1
cd $PBS_O_WORKDIR
echo $PBS_O_WORKDIR

mpicc ./4-7.c -o 4-7.out
date
mpirun ./4-7.out