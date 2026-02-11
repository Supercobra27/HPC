#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define NUM_TRIALS 10000

int main(int argc, char **argv)
{
    int rank, size;
    double trial_times[NUM_TRIALS], overhead_times[NUM_TRIALS];
    double start, end, temp, overhead, total;

    char *b4, *b32, *b256, *kb2, *kb16, *kb128, *mb1;
    int sizes[7] = {4, 32, 256, 1024 * 2, 1024 * 16, 1024 * 128, 1024 * 1024};
    b4 = (char *)malloc(sizeof(char) * 4);
    b32 = (char *)malloc(sizeof(char) * 32);
    b256 = (char *)malloc(sizeof(char) * 256);
    kb2 = (char *)malloc(sizeof(char) * 1024 * 2);
    kb16 = (char *)malloc(sizeof(char) * 1024 * 16);
    kb128 = (char *)malloc(sizeof(char) * 1024 * 128);
    mb1 = (char *)malloc(sizeof(char) * 1024 * 1024);
    char *ptrs[7] = {b4, b32, b256, kb2, kb16, kb128, mb1};

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(!rank) printf("size,trial,time\n");
    for (int i = 0; i < 7; i++)
    {

        for (int j = 0; j < NUM_TRIALS; j++)
        {
            MPI_Barrier(MPI_COMM_WORLD);
            temp = MPI_Wtime();
            start = MPI_Wtime();
            MPI_Bcast(ptrs[i], sizes[i], MPI_BYTE, 0, MPI_COMM_WORLD);
            end = MPI_Wtime();
            overhead = start - temp;
            total = end - start - overhead;
            overhead_times[j] = overhead;
            trial_times[j] = total;
            if (!rank) printf("%d,%d,%.10f\n",sizes[i], j + 1, trial_times[j]);
        }
    }

    MPI_Finalize();

    return 0;
}