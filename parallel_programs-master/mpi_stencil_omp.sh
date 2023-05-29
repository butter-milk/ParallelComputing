#!/bin/sh

if [ $# -lt 2 ]
    then
        echo "Supply 'n' and 'iterations' please"
        exit 1
fi
if [ ! -d "results" ]; then
    mkdir results
    echo "created directory 'results'"
fi
if [ ! -d "reports" ]; then
    mkdir reports
    echo "created directory 'reports'"
fi

OMPTHREADS=4
TASKS_PER_NODE=$((8 / OMPTHREADS))

NODES=1
while [ $NODES -le 8 ]
do

    NTASKS=$(( TASKS_PER_NODE * NODES ))
    
    sed "s/NODES/$NODES/g" mpi_stencil_omp.sh.template > mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/NTASKS/$NTASKS/g" mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/OMPTHREADS/$OMPTHREADS/g" mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/TASKS_PER_NODE/$TASKS_PER_NODE/g" mpi_omp_stencil_"$NTASKS".sh

    sed -i "s/LEN/$1/g" mpi_omp_stencil_"$NTASKS".sh
    sed -i "s/ITER/$2/g" mpi_omp_stencil_"$NTASKS".sh
    sbatch mpi_omp_stencil_"$NTASKS".sh

    rm mpi_omp_stencil_"$NTASKS".sh    

    NODES=$(( 2 * NODES ))
done