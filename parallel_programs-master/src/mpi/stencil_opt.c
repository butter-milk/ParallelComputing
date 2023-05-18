#include <stdlib.h>
#include <immintrin.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>

#define REAL double // You can change this to float for less precision

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

const REAL a = 0.3;
const REAL b = 0.5;
const REAL c = 0.4;

/* We split up the stencil in smaller stencils, of roughly SPACEBLOCK size,
 * and treat them for TIMEBLOCK iterations. Play around with these. Do the considerations
 * change when parallelising? */
const int SPACEBLOCK = 1000; //ORIGINALLY WAS 1250
const int TIMEBLOCK = 100;

/* Takes buffers *in, *out of size n + iterations.
 * out[0: n - 1] is the first part of the stencil of in[0, n + iterations - 1]. */
void Left(REAL **in, REAL **out, size_t n, int iterations)
{
    (*out)[0] = (*in)[0];

    for (int t = 1; t <= iterations; t++) {
        // CAN BE PAR using omp
        for (size_t i = 1; i < n + iterations - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        if (t != iterations) {
            REAL *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

/* Takes buffers *in, *out of size n + 2 * iterations.
 * out[iterations: n + iterations - 1] is the
 * middle part of the stencil of in[0, n + 2 * iterations - 1]. */
void Middle(REAL **in, REAL **out, size_t n, int iterations)
{
    for (int t = 1; t <= iterations; t++) {
        //CAN BE PAR using omp
        for (size_t i = t; i < n + 2 * iterations - t; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        if (t != iterations) {
            REAL *temp = *in;
            *in = *out;
            *out = temp;
        }
    }

}

/* Takes buffers *in, *out of size n + iterations.
 * out[iterations: n + iterations - 1] is the last part of
 * the stencil of in[0, n + iterations - 1]. */
void Right(REAL **in, REAL **out, size_t n, int iterations)
{
    (*out)[n + iterations - 1] = (*in)[n + iterations - 1];

    for (int t = 1; t <= iterations; t++) {
        //CAN BE PAR using omp
        for (size_t i = t; i < n + iterations - 1; i++) {
            (*out)[i] = a * (*in)[i - 1] + b * (*in)[i] + c * (*in)[i + 1];
        }

        if (t != iterations) {
            REAL *temp = *in;
            *in = *out;
            *out = temp;
        }
    }
}

void StencilBlocked(REAL **in, REAL **out, size_t n, int iterations, int my_rank, int p)
{
    REAL *inBuffer = malloc((SPACEBLOCK + 2 * iterations) * sizeof(REAL));

    REAL *outBuffer = malloc((SPACEBLOCK + 2 * iterations) * sizeof(REAL));
    if (my_rank == 0 && p!=1){
        double x, y;
        y= (*in)[n-2];
        MPI_Recv(&x, 1, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        (*in)[n-1] = x;
        MPI_Send(&y, 1, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD);
        /*
        get first element of next rank
        send element to first index of the next rank, if not last rank
        */
    }
    if (my_rank < p-1 && my_rank != 0) {
        double x, y;
        y= (*in)[n-2];
        MPI_Recv(&x, 1, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        (*in)[n-1] = x;
        MPI_Send(&y, 1, MPI_DOUBLE, my_rank+1, 0, MPI_COMM_WORLD);
        /*
        get first element of next rank
        send element to first index of the next rank, if not last rank
        */
    }
    if (my_rank > 0) {
        double x, y;
        x = (*in)[1];  
        MPI_Send(&x, 1, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD);
        MPI_Recv(&y, 1, MPI_DOUBLE, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        (*in)[0] = y;
        /*
        get last element of previous rank
        send element to last index of the previous rank, if not last rank
        */
        }

    // (#blocks-1)*iterations*2 OVERLAP 
    for (size_t block = 0; block < n / SPACEBLOCK; block++) {
        if (block == 0) {
            memcpy(inBuffer, *in, (SPACEBLOCK + iterations) * sizeof(REAL));
            Left(&inBuffer, &outBuffer, SPACEBLOCK, iterations);
            memcpy(*out, outBuffer, SPACEBLOCK * sizeof(REAL));
        } else if (block == n / SPACEBLOCK - 1) {
            memcpy(inBuffer, *in + block * SPACEBLOCK - iterations,
                   (SPACEBLOCK + iterations) * sizeof(REAL));
            Right(&inBuffer, &outBuffer, SPACEBLOCK, iterations);
            memcpy(*out + block * SPACEBLOCK, outBuffer + iterations, SPACEBLOCK * sizeof(REAL));
        } else {
            memcpy(inBuffer, *in + block * SPACEBLOCK - iterations,
                    (SPACEBLOCK + 2 * iterations) * sizeof(REAL));
            Middle(&inBuffer, &outBuffer, SPACEBLOCK, iterations);
            memcpy(*out + block * SPACEBLOCK, outBuffer + iterations, SPACEBLOCK * sizeof(REAL));
        }
        }
    free(inBuffer);
    free(outBuffer);
}

void Stencil(REAL **in, REAL **out, size_t n, int iterations, int my_rank, int p)
{
    int rest_iters = iterations % TIMEBLOCK;
    if (rest_iters != 0) {
        StencilBlocked(in, out, n, rest_iters, my_rank,p);
        REAL *temp = *out;
        *out = *in;
        *in = temp;
    }

    for (int t = rest_iters; t < iterations; t += TIMEBLOCK) {
        StencilBlocked(in, out, n, TIMEBLOCK, my_rank, p);
        REAL *temp = *out;
        *out = *in;
        *in = temp;
    }

    REAL *temp = *out;
    *out = *in;
    *in = temp;
}


//WE DO NOT CARE ABOUT THIS
void StencilSlow(REAL **in, REAL **out, size_t n, int iterations)
{
    (*out)[0] = (*in)[0];
    (*out)[n - 1] = (*in)[n - 1];

    for (int t = 1; t <= iterations; t++) {
        /* Update only the inner values. */
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


#ifdef CHECK
bool equal(REAL *x, REAL *y, size_t n, REAL error)
{
    for (size_t i = 0; i < n; i++) {
        if (fabs(x[i] - y[i]) > error) {
            printf("Index %zu: %lf != %lf\n", i, x[i], y[i]);
            return false;
        }
    }

    return true;
}
#endif
//START CARING

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Please specify 2 arguments (n, iterations).\n");
        return EXIT_FAILURE;
    }

    size_t n = atoll(argv[1]);
    int iterations = atoi(argv[2]);

    if (n % SPACEBLOCK != 0) {
        printf("I am lazy, so assumed that SPACEBLOCK divides n. Suggestion: n = %ld\n",
                n / SPACEBLOCK * SPACEBLOCK);
        return EXIT_FAILURE;
    }

    
    MPI_Init(&argc, &argv);
    /*
    assuming that p divides n, so if we use 1,2,4,8,16,32 threads, n should always divide 32!
    furthermore, n should divide SPACEBLOCK as well, meaning that n should be a multiple of lcm(32,1000) = 4000
    */
    int my_rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    REAL *in = calloc(n/p + 2, sizeof(REAL));
    REAL *out = malloc((n/p +2) * sizeof(REAL));

    
    if (my_rank==0) {in[0] = 100;}
    if (my_rank*n/p <= n/2 && (my_rank+1)*n/p >= n/2){in[n/2-my_rank*n/p] = n;}//THIS IS CORRECT SINCE N IS ASSUMED TO BE DIVIDED BY 32
    if (my_rank == p-1) {in[n-1]=1000;}
    double duration;
    TIME(duration, Stencil(&in, &out, n/p, iterations, my_rank, p););
    if(my_rank==0){printf("%lf %lf\n", duration, iterations * (n-2) * 5 / 1000000000 /duration);}
    MPI_Finalize();
    
#ifdef CHECK
    REAL *in2 = calloc(n, sizeof(REAL));
    in2[0] = 100;
    in2[n / 2] = n;
    in2[n - 1] = 1000;
    REAL *out2 = malloc(n * sizeof(REAL));
    TIME(duration, StencilSlow(&in2, &out2, n, iterations););
    printf("Slow version took %lfs, or ??? Gflops/s\n", duration);
    printf("Checking whether they computed the same result with error 0.0000...\n");
    if (equal(out, out2, n, 0.0000)) {
        printf("They are (roughly) equal\n");
    }
    free(in2);
    free(out2);
#endif

    free(in);
    free(out);

    return EXIT_SUCCESS;
}
