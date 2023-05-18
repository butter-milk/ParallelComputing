#!/bin/sh

#SBATCH --account=csmpi
#SBATCH --ntasks=NTASKS
#SBATCH --nodes=NODES
#SBATCH --ntasks-per-node=8
#SBATCH --threads-per-core=1
#SBATCH --partition=csmpi_short
#SBATCH --time=00:05:00
#SBATCH --output=hello_mpi_NTASKS.out

# Compile on the machine, not the head node
make bin/stencil_opt_mpi

for threads in 1 2 4 8 16 32; do
    echo "" > results/stencil_opt_mpi_t${threads}.txt
    for n in 4000 200000 100000 500000 5000000 50000000; do
        echo "n: ${n}" > results/stencil_opt_mpi_t${threads}.txt
        for iterations in 1000 10000 100000; do
            echo "iterations: ${iterations}" > results/stencil_opt_mpi_t${threads}.txt
            for measurement in 1 2 3 4 5; do
                mpirun -N ${threads} bin/stencil_opt_mpi ${n} ${iterations} >> results/stencil_opt_mpi_t"${threads}".txt