#include "mpi.h"
#include <stdio.h>

#define BUFF_SIZE 8

int main(int argc, char** argv) {
    int rank, size;
    int position = 0;

    char rank_data[BUFF_SIZE] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    char buff[BUFF_SIZE];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int prev = (rank - 1 + size) % size;
    int next = (rank + 1) % size;

    if(rank == 0) {
        MPI_Pack(&rank_data, BUFF_SIZE, MPI_BYTE, buff, BUFF_SIZE, &position, MPI_COMM_WORLD);
        MPI_Send(buff, position, MPI_PACKED, next, 0, MPI_COMM_WORLD);
    }

    if(rank != 0) {
        char local_buff[BUFF_SIZE];
        char out_buff[BUFF_SIZE];
        char rank_data = '\0';

        MPI_Recv(&local_buff, BUFF_SIZE, MPI_BYTE, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Unpack(&local_buff, BUFF_SIZE, &position, &local_buff, BUFF_SIZE, MPI_BYTE, MPI_COMM_WORLD);
        
        rank_data = local_buff[BUFF_SIZE - rank];
        printf("Data Captured[%d] (Rank: %d): %c\n", BUFF_SIZE - rank, rank, rank_data);

        position = 0;
        MPI_Pack(&local_buff, BUFF_SIZE, MPI_BYTE, out_buff, BUFF_SIZE, &position, MPI_COMM_WORLD);
        MPI_Send(out_buff, position, MPI_PACKED, next, 0, MPI_COMM_WORLD);
    }


    MPI_Finalize();

    return 0;
}
