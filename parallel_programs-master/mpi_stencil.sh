#!/bin/sh

if [ $# -lt 2 ]
    then
        echo "Supply 'n' and 'iterations' please"
        exit 1
fi
    
sed "s/NODES/8/g" mpi_stencil.sh.template > mpi_stencil_"$NTASKS".sh
sed -i "s/NTASKS/64/g" mpi_stencil_"$NTASKS".sh
sed -i "s/LEN/$1/g" mpi_stencil_"$NTASKS".sh
sed -i "s/ITER/$2/g" mpi_stencil_"$NTASKS".sh
sbatch mpi_stencil_"$NTASKS".sh

rm mpi_stencil_"$NTASKS".sh    


