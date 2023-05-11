#!/bin/sh

#SBATCH --account=csmpistud
#SBATCH --cpus-per-task=32
#SBATCH --partition=csmpi_fpga_short
#SBATCH --time=00:05:00
#SBATCH --output=stencil_omp.out

# Compile on the machine, not the head node
make bin/stencil_omp

for P in 32 ; do
    echo "#threads: "${P} >>results/stencil_omp_"${P}".txt
    for iterations in 1000; do
        echo "Iterations: "${iterations} "\n" >> results/stencil_omp_"$P".txt
        for n in 500 5000 50000 500000 5000000 50000000 500000000; do 
        echo "n: "${n} "\n" >> results/stencil_omp_"$P".txt
            for i in 1 2 3; do
                OMP_NUM_THREADS="$P" bin/stencil_omp ${n} ${iterations} >> results/stencil_omp_"$P".txt
                "\n" >> results/stencil_omp_"$P".txt
            done
            echo "FINISHED MEASUREMENTS OF "${iterations} " , " ${n} 
        done
    done
done

