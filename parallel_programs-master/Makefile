CC = gcc
MPICC = mpicc
FLAGS = -Ofast -march=native -mtune=native -Wall -g
LFLAGS = -lm

SEQ_SRC = $(wildcard src/seq/*.c)
SEQ = $(patsubst src/seq/%.c, bin/%_seq, $(SEQ_SRC))
OMP_SRC = $(wildcard src/omp/*.c)
OMP = $(patsubst src/omp/%.c, bin/%_omp, $(OMP_SRC))
MPI_SRC = $(wildcard src/mpi/*.c)
MPI = $(patsubst src/mpi/%.c, bin/%_mpi, $(MPI_SRC))
CL_SRC = $(wildcard src/opencl/*.c)
CL = $(patsubst src/opencl/%.c, bin/%_cl, $(CL_SRC))

.PHONY: all seq omp mpi cl clean

all: seq omp mpi cl

seq: $(SEQ)

omp: $(OMP)

mpi: $(MPI)

cl: $(CL)

bin/%_seq: src/seq/%.c
	$(CC) $(FLAGS) $^ -o $@ $(LFLAGS)

bin/%_omp: src/omp/%.c
	$(CC) $(FLAGS) $^ -o $@ $(LFLAGS) -fopenmp

bin/%_mpi: src/mpi/%.c
	$(MPICC) $(FLAGS) $^ -o $@ $(LFLAGS)

# The OpenCL flag must be after the target
bin/%_cl: src/opencl/%.c bin/simple.o
	$(CC) $(FLAGS) -D CL_TARGET_OPENCL_VERSION=220 $^ -o $@ -lOpenCL

bin/simple.o: src/simple.c src/simple.h
	$(CC) $(FLAGS) -D CL_TARGET_OPENCL_VERSION=220 -c $< -o $@ -lOpenCL

clean:
	$(RM) bin/* mpi_hello_* *.out
