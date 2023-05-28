for n in 64000 640000 6400000 64000000; do
  for iterations in 1000 10000 100000; do
    ./mpi_omp_stencil_rep.sh ${n} ${iterations}
  done
done
