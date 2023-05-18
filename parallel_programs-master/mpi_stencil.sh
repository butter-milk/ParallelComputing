#!/bin/sh

if [ $# -lt 2 ]
    then
        echo "Supply 'n' and 'iterations' please"
        exit 1
fi

NODES=1

while [ $NODES -le 8 ]
do
    for EXPRM in 1 2 3 4 5;
    do
        NTASKS=$(( 8 * NODES ))
        
        sed "s/NODES/$NODES/g" mpi_stencil.sh.template > mpi_stencil_"$NTASKS"_"$EXPRM".sh
        sed -i "s/NTASKS/$NTASKS/g" mpi_stencil_"$NTASKS"_"$EXPRM".sh
        sed -i "s/EXPRM/$EXPRM/g" mpi_stencil_"$NTASKS"_"$EXPRM".sh
        sed -i "s/LEN/$1/g" mpi_stencil_"$NTASKS"_"$EXPRM".sh
        sed -i "s/ITER/$2/g" mpi_stencil_"$NTASKS"_"$EXPRM".sh
        sbatch mpi_stencil_"$NTASKS"_"$EXPRM".sh

        rm mpi_stencil_"$NTASKS"_"$EXPRM".sh    
    done

    NODES=$(( 2 * NODES ))
done
