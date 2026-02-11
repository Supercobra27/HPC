#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_TRIALS 10000

/*
    1B, 10B, 100B, 1KB, 10KB, 100KB, 1MB
    Ping-Pong
*/

int main(int argc, char **argv)
{
    int rank, size;

    double trial_times[NUM_TRIALS], overhead_times[NUM_TRIALS];
    double start, end, temp, overhead, total;

    int sizes[7] = {1, 10, 100, 1024, 10240, 102400, 1048576};
    char *b1, *b10, *b100, *kb1, *kb10, *kb100, *mb1;
    b1 = (char *)malloc(sizeof(char) * 1);
    b10 = (char *)malloc(sizeof(char) * 10);
    b100 = (char *)malloc(sizeof(char) * 100);
    kb1 = (char *)malloc(sizeof(char) * 1024);
    kb10 = (char *)malloc(sizeof(char) * 10240);
    kb100 = (char *)malloc(sizeof(char) * 102400);
    mb1 = (char *)malloc(sizeof(char) * 1048576);
    char *ptrs[7] = {b1, b10, b100, kb1, kb10, kb100, mb1};

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(!rank) printf("bytes, trial, times\n");

    if (size > 2)
        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_COUNT);
    for (int i = 0; i < 7; i++)
    {

        for (int j = 0; j < NUM_TRIALS; j++)
        {
            if (!rank)
            {
                temp = MPI_Wtime();
                start = MPI_Wtime();
                MPI_Send(ptrs[i], sizes[i], MPI_BYTE, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(ptrs[i], sizes[i], MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                end = MPI_Wtime();
                overhead = start - temp;
                total = end - start - overhead;
                overhead_times[j] = overhead;
                trial_times[j] = total / 2;
                printf("%d, %d, %.10f\n", sizes[i], j + 1, trial_times[j]);
            }

            if (rank)
            {
                MPI_Recv(ptrs[i], sizes[i], MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(ptrs[i], sizes[i], MPI_BYTE, 0, 0, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Finalize();

    return 0;
}
