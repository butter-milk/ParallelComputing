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

void StencilBlocked(REAL **in, REAL **out, size_t size, int iterations, int my_rank, int p)
{
    REAL *inBuffer = malloc((SPACEBLOCK + 2 * iterations) * sizeof(REAL));
    REAL *outBuffer = malloc((SPACEBLOCK + 2 * iterations) * sizeof(REAL));

    int blocks;
    if (my_rank == 0 || my_rank == p-1) {
        blocks = (size - TIMEBLOCK) / SPACEBLOCK;
    } else {
        blocks = (size - 2 * TIMEBLOCK) / SPACEBLOCK;
    }

    for (size_t block = 0; block < blocks; block++) {
        if (my_rank == 0 && block == 0) {
            memcpy(inBuffer, *in, (SPACEBLOCK + iterations) * sizeof(REAL));
            Left(&inBuffer, &outBuffer, SPACEBLOCK, iterations);
            memcpy(*out, outBuffer, SPACEBLOCK * sizeof(REAL));
        } else if (my_rank == p - 1 && block == blocks - 1) {
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

void SendRecvNeighborValues(REAL **in, size_t size, int my_rank, int p, int iterations)
{
    if (my_rank != 0) {
        // send in[iterations: 2 * iterations] to previous thread
        MPI_Send((*in) + iterations * sizeof(REAL), iterations, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD);
    }
    if (my_rank != p - 1) {
        // send in[size - 2*iterations: size - iterations] to next thread
        MPI_Send((*in) + (size - 2 * iterations) * sizeof(REAL), iterations, MPI_DOUBLE, my_rank + 1, 0, MPI_COMM_WORLD);
    }

    if (my_rank != 0) {
        // Recive in[0 : iterations] from previous thread
        MPI_Recv((*in), iterations, MPI_DOUBLE, my_rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (my_rank != p - 1) {
        // Recive in[size - iterations : size] from next thread
        MPI_Recv((*in) + (size - iterations) * sizeof(REAL), iterations, MPI_DOUBLE, my_rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}
void Stencil(REAL **in, REAL **out, size_t size, int iterations, int my_rank, int p)
{
    int rest_iters = iterations % TIMEBLOCK;
    if (rest_iters != 0) {
        StencilBlocked(in, out, size, rest_iters, my_rank, p);
        REAL *temp = *out;
        *out = *in;
        *in = temp;
        SendRecvNeighborValues(in, size, my_rank, p, rest_iters);
    }

    for (int t = rest_iters; t < iterations; t += TIMEBLOCK) {
        StencilBlocked(in, out, size, TIMEBLOCK, my_rank, p);
        REAL *temp = *out;
        *out = *in;
        *in = temp;
        SendRecvNeighborValues(in, size, my_rank, p, TIMEBLOCK);
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

void init_input(REAL **in, REAL **out, int *size, size_t n, int my_rank, int p) {
    if (p == 1) {
        *size = n;
        *in = calloc(*size,  sizeof(REAL));
        *out = malloc(*size * sizeof(REAL));
        (*in)[0] = 100;
        (*in)[n/2] = n;
        (*in)[n - 1] = 1000;
    } else if (my_rank == 0) {
        *size = n/p + TIMEBLOCK;
        *in = calloc(*size,  sizeof(REAL));
        *out = malloc(*size * sizeof(REAL));

        (*in)[0] = 100;
        if (n/2 < *size) {
            (*in)[n/2] = n;
        }
    } else if (my_rank == p - 1) {
        *size = n/p + TIMEBLOCK;
        *in = calloc(*size,  sizeof(REAL));
        *out = malloc(*size * sizeof(REAL));

        (*in)[*size - 1] = 1000;
        if (n/2 >= n - *size) {
            (*in)[n/2 - (n - *size)] = n;
        }
    } else {
        *size = n/p + 2*TIMEBLOCK;
        *in = calloc(*size,  sizeof(REAL));
        *out = malloc(*size * sizeof(REAL));
        int start = my_rank * n/p - TIMEBLOCK;
        if (n/2 >= start && n/2 < start + *size) {
            (*in)[n/2 - start] = n;
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

    if (n % SPACEBLOCK != 0) {
        printf("I am lazy, so assumed that SPACEBLOCK divides n. Suggestion: n = %ld\n",
                n / SPACEBLOCK * SPACEBLOCK);
        return EXIT_FAILURE;
    }

    
    MPI_Init(&argc, &argv);

    int my_rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if ((n / SPACEBLOCK) % p != 0) {
        printf("I am lazy, so assumed that rank count divides the number of blocks.\n");
        return EXIT_FAILURE;
    }

    int size;
    REAL *in, *out;
    init_input(&in, &out, &size, n, my_rank, p);
    
    double duration;
    TIME(duration, Stencil(&in, &out, size, iterations, my_rank, p););
    if(my_rank==0){printf("%lf %lf\n", duration, iterations * (n-2) * 5 / 1000000000 /duration);}
    
    free(in);
    free(out);

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

    return EXIT_SUCCESS;
}
