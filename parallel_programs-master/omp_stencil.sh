#!/bin/sh

#SBATCH --account=csmpistud
#SBATCH --cpus-per-task=32
#SBATCH --partition=csmpi_fpga_short
#SBATCH --time=00:05:00
#SBATCH --output=stencil_omp.out

# Compile on the machine, not the head node
make bin/stencil_omp

n = 5000000
i = 1000

for P in 1 2 4 8 16 32; do
    OMP_NUM_THREADS="$P" bin/stencil_omp ${n} ${i} > results/stencil_omp_"$n"_"$i"_"$P".txt
done
