#include "omp.h"
#include <stdio.h>

#define NUM_THREADS 8

int main(int argc, char** argv) {
    int nthreads, id;

    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel private(nthreads, id)
    {
        id = omp_get_thread_num();
        nthreads = omp_get_num_threads();
        printf("Hello world: I am process/thread %d, out of %d processes/threads.\n", id, nthreads);
    }
    return 0;
}

