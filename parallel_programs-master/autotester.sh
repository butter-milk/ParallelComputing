for n in 4000 200000 100000 500000 5000000 50000000; do
    for iterations in 1000 10000 100000; do 
        ./mpi_stencil.sh ${n} ${iterations}
    done
done
