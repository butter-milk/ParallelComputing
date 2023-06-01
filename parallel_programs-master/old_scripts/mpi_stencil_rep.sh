#!/bin/sh

if [ $# -lt 2 ]
    then
        echo "Supply 'n' and 'iterations' please"
        exit 1
fi
if [ ! -d "measurements" ]; then
    mkdir measurements
    echo "created directory 'measurements'"
fi
if [ ! -d "reports" ]; then
    mkdir reports
    echo "created directory 'reports'"
fi

NODES=1

while [ $NODES -le 8 ]
do

    NTASKS=$(( 8 * NODES ))

    sed "s/NODES/$NODES/g" mpi_stencil_rep.sh.template > mpi_stencil_"$NTASKS".sh
    sed -i "s/NTASKS/$NTASKS/g" mpi_stencil_"$NTASKS".sh
    sed -i "s/LEN/$1/g" mpi_stencil_"$NTASKS".sh
    sed -i "s/ITER/$2/g" mpi_stencil_"$NTASKS".sh
    sbatch mpi_stencil_"$NTASKS".sh

    rm mpi_stencil_"$NTASKS".sh

    NODES=$(( 2 * NODES ))
done
