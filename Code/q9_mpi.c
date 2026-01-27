#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARR_LENGTH 1000
#define ROOT 0
#define AMT_TAG 123

int main(int argc, char **argv)
{
    int rank, size;

    int *a = NULL, *local_a, *local_local_a;
    int sum = 0;

    int amount_per_proc, proc;

    MPI_Status status;

    // Initialize Array

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    amount_per_proc = (ARR_LENGTH / 2) / (size / 2);

    local_local_a = (int *)malloc(sizeof(int) * amount_per_proc);

    local_a = (int *)malloc(sizeof(int) * ARR_LENGTH / 2);

    if (!rank)
    {
        a = (int *)malloc(sizeof(int) * ARR_LENGTH);

        for (int i = 0; i < ARR_LENGTH; i++)
        {
            a[i] = i + 1;
        }

        memcpy(local_a, &a[0], sizeof(int) * ARR_LENGTH / 2);
        MPI_Send(local_a, ARR_LENGTH / 2, MPI_INT, size / 2, 0, MPI_COMM_WORLD);
        MPI_Send(&amount_per_proc, 1, MPI_INT, size / 2, AMT_TAG, MPI_COMM_WORLD);

        proc = (ARR_LENGTH / 2) / amount_per_proc;

        memcpy(local_a, &a[ARR_LENGTH / 2], sizeof(int) * ARR_LENGTH / 2);

        for (int i = 0; i < proc; i++)
        {
            memcpy(local_local_a, &local_a[i * amount_per_proc], amount_per_proc * sizeof(int));
            MPI_Send(local_local_a, amount_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == size / 2)
    {
        MPI_Recv(local_a, ARR_LENGTH / 2, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&amount_per_proc, 1, MPI_INT, ROOT, AMT_TAG, MPI_COMM_WORLD, &status);
        proc = (ARR_LENGTH / 2) / amount_per_proc;

        for (int i = size / 2; i < proc * 2; i++)
        {
            memcpy(local_local_a, &local_a[(i - size / 2) * amount_per_proc], amount_per_proc * sizeof(int));
            MPI_Send(local_local_a, amount_per_proc, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Recv(local_local_a, amount_per_proc, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    for (int k = 0; k < amount_per_proc; k++)
    {
        sum += local_local_a[k];
    }

    //printf("Process %d's sum is %d\n", rank, sum);
    int final_sum = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank < size / 2 && rank != ROOT)
    {
        MPI_Send(&sum, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
    }
    else if (rank >= size / 2 && rank != size / 2)
    {
        MPI_Send(&sum, 1, MPI_INT, size / 2, 0, MPI_COMM_WORLD);
    }

    if (rank == size / 2)
    {
        int temp = 0;
        final_sum += sum;
        for (int i = size / 2 + 1; i < size; i++)
        {
            MPI_Recv(&temp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            final_sum += temp;
        }
        MPI_Send(&final_sum, 1, MPI_INT, ROOT, 1, MPI_COMM_WORLD);
    }

    if (!rank)
    {
        int temp = 0;
        final_sum += sum;
        for (int i = 1; i < size / 2; i++)
        {
            MPI_Recv(&temp, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            final_sum += temp;
        }
        MPI_Recv(&temp, 1, MPI_INT, size / 2, 1, MPI_COMM_WORLD, &status);
        final_sum += temp;
        printf("Final Sum %d\n", final_sum);
        free(a);
    }

    free(local_a);
    free(local_local_a);

    MPI_Finalize();

    return 0;
}
