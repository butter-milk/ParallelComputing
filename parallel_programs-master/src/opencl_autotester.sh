#!/bin/sh

#SBATCH --account=csmpistud
#SBATCH --partition=csmpi_fpga_short
#SBATCH --gres=gpu:nvidia_a30:1
#SBATCH --time=0:05:00
#SBATCH --output=opencl.out

export XILINX_XRT=/opt/xilinx/xrt

#Compile on the machine, not the head node
make bin/stencil_cl
echo "" >> results/result.txt
for n in 3200 32000 320000 3200000 32000000; do
    for iterations in 1000 10000 100000 1000000; do
        echo "" >> measurements_"${n}"_"${iterations}"_"${n}"
        for measurements in 1 2 3 4 5; do
            bin/stencil_cl ${n} ${iterations} > results/result.txt
        done
    done
done