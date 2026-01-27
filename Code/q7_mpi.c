#include "mpi.h"
#include <stdio.h>
#include <limits.h>

#define BOUND 16
#define ARR_LENGTH 256
#define START start = MPI_Wtime();
#define END end = MPI_Wtime();
#define TIMING printf("Total time elapsed: %.6f\n", (end - start));

int mod_rand(int x) {
    return ((x*37+17)%100)+1;
}

int main(int argc, char** argv) {
    int rank, size, nthreads;
    double start, end;

    int min = INT_MAX;
    int global_min;
    int a[ARR_LENGTH];

    // Initialize Array
    for (int i=0; i<ARR_LENGTH; i++){
        a[i] = mod_rand(i);
    }

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int stride = ARR_LENGTH / size;
    
    int a_recv[stride];

    int prev = (rank - 1 + size) % size;
    int next = (rank + 1) % size;

    // Figure out scatter lengths
    int to_send[size];
    int displs[size];

    for(int i = 0; i < size; i++) {
        to_send[i] = stride;
        displs[i] = i*(stride);
    }

    START
    
    MPI_Scatterv(&a, to_send, displs, MPI_INT, a_recv, stride, MPI_INT, 0, MPI_COMM_WORLD);

        for(int i = 0; i < stride; i++){
            if(min > a[i]) {
                min = a[i];
            }
        }

    MPI_Reduce(&min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    END

    if(rank == 0) printf("Global Minimum: %d\n", global_min);
    if (rank == 0) TIMING

    MPI_Finalize();

    return 0;
}
