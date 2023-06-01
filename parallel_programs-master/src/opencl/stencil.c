#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../simple.h"
#include <CL/cl.h>

#define DATA_SIZE 10240000

struct timespec start, stop;

void printTimeElapsed( char *text)
{
  double elapsed = (stop.tv_sec -start.tv_sec)*1000.0
                  + (double)(stop.tv_nsec -start.tv_nsec)/1000000.0;
  printf( "%s: %f msec\n", text, elapsed);
}

void timeDirectImplementation( int count, float* data, float* results)
{
  clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &start);
  for (int i = 0; i < count; i++)
    results[i] = data[i] * data[i];
  clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &stop);
  printTimeElapsed( "kernel equivalent on host");
}


#define REAL double

/* You may need a different method of timing if you are not on Linux. */
#define TIME(duration, fncalls)                                        \
    do {                                                               \
        struct timeval tv1, tv2;                                       \
        gettimeofday(&tv1, NULL);                                      \
        fncalls                                                        \
        gettimeofday(&tv2, NULL);                                      \
        duration = (REAL) (tv2.tv_usec - tv1.tv_usec) / 1000000 +    \
         (REAL) (tv2.tv_sec - tv1.tv_sec);                           \
    } while (0)

const REAL a = 0.1;
const REAL b = 0.2;
const REAL c = 0.3;

void Stencil(REAL **in, REAL **out, size_t n, int iterations)
{
    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    for (int t = 1; t <= iterations; t++) {
        /* Update only the inner values. */

        /* For every i, we perform 5flops (2 addition and 3 multiplication operations )*/
        /* So our total Gflops/s = iterations * (n-1) * 5 / 1000000000 /duration*/
        for (int i = 1; i < n - 1; i++) {
            (*out)[i] = a * (*in)[i - 1] +
                        b * (*in)[i] +
                        c * (*in)[i + 1];
        }

        /* The output of this iteration is the input of the next iteration (if there is one). */
        if (t != iterations) {
            REAL *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        return EXIT_FAILURE;
    }

    cl_int err;
    cl_kernel kernel;
    size_t global[1];
    size_t local[1];
    local[0] = atoi(argv[1]);
    /* Reading the openCL kernel code from 'stencil.cl' */
    char *KernelSource = readOpenCL( "src/opencl/stencil.cl");

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    REAL *in = calloc(n, sizeof(REAL));
    in[0] = 100;
    in[n - 1] = 1000;
    REAL *out = malloc(n * sizeof(REAL));
    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    err = initGPUVerbose();

    if( err == CL_SUCCESS) {
        for (int t = 1; t <= iterations; t++) {
            kernel = setupKernel( KernelSource, "stencil", 3, FloatArr, count, data,
                                                        FloatArr, count, results,
                                                        IntConst, count);

            runKernel( kernel, 1, global, local);

            clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &stop);
            printKernelTime();
            printTransferTimes();
            printTimeElapsed( "CPU time spent");
        }


    }
    //double duration;
    //TIME(duration, Stencil(&in, &out, n, iterations););
    //printf("This took %lfs, or %lf Gflops/s\n", duration, iterations * (n-1) * 5 / 1000000000 /duration);

    free(in);
    free(out);

    return EXIT_SUCCESS;
}
