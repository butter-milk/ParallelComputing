#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../simple.h"
#include <CL/cl.h>

#define DATA_SIZE 10240000

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

const REAL a = 0.1;
const REAL b = 0.2;
const REAL c = 0.3;

void Stencil(REAL **in, REAL **out, size_t n, int iterations)
{
    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    cl_int err;
    cl_kernel kernel;
    size_t global[1];
    size_t local[1];
    local[0] = 32;
    global[0] = n;
    /* Reading the openCL kernel code from 'stencil.cl' */
    char *KernelSource = readOpenCL( "./src/opencl/stencil.cl");

    err = initGPUVerbose();
    if( err == CL_SUCCESS) {
        kernel = setupKernel( KernelSource, "stencil", 3, FloatArr, n, (*in),
                                            FloatArr, n, (*out),
                                            IntConst, n);
        for (int t = 1; t <= iterations; t++) {

            if(t%2){
                clSetKernelArg(kernel, 0, sizeof(cl_mem), &in);
                clSetKernelArg(kernel, 1, sizeof(cl_mem), &out);

            }else{
                clSetKernelArg(kernel, 0, sizeof(cl_mem), &out);
                clSetKernelArg(kernel, 1, sizeof(cl_mem), &in);
            }
            //switch in and out pointers in kernel
            if (t==iterations){
                runKernel( kernel, 1, global, local);
            }else{
                launchKernel( kernel, 1, global, local);
            }
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        return EXIT_FAILURE;
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    n += n%32;
    REAL *in = calloc(n, sizeof(REAL));
    in[0] = 100;
    in[n - 1] = 1000;
    REAL *out = malloc(n * sizeof(REAL));

    double duration;
    TIME(duration, Stencil(&in, &out, n, iterations););
    printf("This took %lfs, or %lf Gflops/s\n", duration, iterations * (n-1) * 5 / 1000000000 /duration);

    free(in);
    free(out);

    return EXIT_SUCCESS;
}