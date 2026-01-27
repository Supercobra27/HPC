#include "omp.h"
#include <stdio.h>
#include <limits.h>

#define NUM_THREADS 16
#define ARR_LENGTH 256

int mod_rand(int x)
{
    return ((x * 37 + 17) % 100) + 1;
}

int main(int argc, char **argv)
{
    int nthreads, id;
    int min = INT_MAX;
    int a[ARR_LENGTH];

    // Initialize Array
    for (int i = 0; i < ARR_LENGTH; i++)
    {
        a[i] = mod_rand(i);
    }

    omp_set_num_threads(NUM_THREADS);

    double start, end;

    start = omp_get_wtime();

#pragma omp parallel for reduction(min : min)
    for (int i = 0; i < ARR_LENGTH; i++)
    {
        if (min > a[i])
        {
            min = a[i];
        }
    }

    end = omp_get_wtime();

    printf("Minimum: %d Time: %.6f\n", min, end - start);

    return 0;
}
