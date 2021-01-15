#!/bin/sh

FILE=9-10

mpicc -g -std=gnu99 ./$FILE.c -o ./$FILE.out -lm

mpirun -np 1 ./$FILE.out

