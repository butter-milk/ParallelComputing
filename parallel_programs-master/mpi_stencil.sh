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

mpirun -N 4 bin/stencil_opt_mpi 400000 400000 > results/stencil_opt_mpi_test.txt