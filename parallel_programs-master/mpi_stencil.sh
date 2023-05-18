#!/bin/sh

NODES=1

if [ $# -lt 2 ]
    then
        echo "Supply 'n' and 'iterations' please"
fi


while [ $NODES -le 8 ]
do
    for EXPRM in {1..5}
        NTASKS=$(( 8 * NODES ))
        
        sed "s/NODES/$NODES/g" mpi_stencil.sh.template > mpi_stencil_"$NTASKS".sh
        sed -i "s/NTASKS/$NTASKS/g" mpi_stencil_"$NTASKS".sh
        sed -i "s/EXPRM/$EXPRM/g" mpi_stencil_"$NTASKS".sh
        sed -i "s/LEN/$1/g" mpi_stencil_"$NTASKS".sh
        sed -i "s/ITER/$2/g" mpi_stencil_"$NTASKS".sh
        sbatch mpi_stencil_"$NTASKS".sh

        rm mpi_stencil_"$NTASKS".sh

        NODES=$(( 2 * NODES ))
    
done
