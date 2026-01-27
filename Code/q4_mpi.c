#include "mpi.h"
#include <stdio.h>

int main(int argc, char** argv) {
    int data, size;
    int received;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &data);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int prev = (data - 1 + size) % size;
    int next = (data + 1) % size;

    if (data == 0){
        MPI_Send(&data, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&received, 1, MPI_INT, 7, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received the data %d from process %d.\n", data, received, received);
    } else {
        MPI_Recv(&received, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received the data %d from process %d.\n", data, received, received);
        MPI_Send(&data, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
