#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define NUM_TRIALS 1000

int main(int argc, char **argv)
{
    int rank, size;
    double trial_times[NUM_TRIALS], overhead_times[NUM_TRIALS];
    double start, end, temp, overhead, total;

    char *b64, *kb32, *mb1;
    int sizes[3] = {64, 1024 * 32, 1048576};
    b64 = (char *)malloc(sizeof(char) * 64);
    kb32 = (char *)malloc(sizeof(char) * 1024 * 32);
    mb1 = (char *)malloc(sizeof(char) * 1048576);
    char *ptrs[3] = {b64, kb32, mb1};

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int i = 0; i < 3; i++)
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
            if (!rank) printf("%d, %d, %d, %.10f\n", size, sizes[i], j + 1, trial_times[j]);
        }
    }

    MPI_Finalize();

    return 0;
}