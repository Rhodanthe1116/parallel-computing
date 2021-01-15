#!/bin/sh

FILE=10-4

mpicc -g -std=gnu99 ./$FILE.c -o ./$FILE.out -lm

mpirun -np 1 ./$FILE.out

