#!/bin/sh

if [ $# -lt 2 ]
    then
        echo "Supply 'n' and 'iterations' please"
        exit 1
fi

NODES=1
OMPTHREADS=4
while [ $NODES -le 8 ]
do

    NTASKS=$(( 8 * NODES ))
    
    sed "s/NODES/$NODES/g" mpi_stencil_rep.sh.template > mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/NTASKS/$NTASKS/g" mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/LEN/$1/g" mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/ITER/$2/g" mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/OMPTHREADS/$OMPTHREADS/g" mpi_omp_stencil_"$NTASKS".sh
    sbatch mpi_omp_stencil_"$NTASKS".sh

    rm mpi_stencil_"$NTASKS".sh    

    NODES=$(( 2 * NODES ))
done