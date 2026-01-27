#include "omp.h"
#include <stdio.h>

#define NUM_THREADS 64
#define ARR_LENGTH 1000

int main(int argc, char** argv) {
    int nthreads, id;
    int a[ARR_LENGTH];
    int sum = 0;

    // Initialize Array
    for (int i=0; i<ARR_LENGTH; i++){
        a[i] = i+1;
    }

    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel for reduction(+:sum)
    for(int i=0; i<ARR_LENGTH; i++){
        sum += a[i];
    }

    printf("Final Result: %d\n", sum);

    return 0;
}

