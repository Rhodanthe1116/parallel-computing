#!/bin/sh
mpicc -g -std=gnu99 ./8-10.c -o 8-10.out

echo
date
echo

mpirun -np 4 ./8-10.out  ./A b

