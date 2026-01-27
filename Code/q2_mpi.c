#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main(int argc, char** argv) {
    int rank, size;

    int done = 0, n = 64, i;

    double local_pi, pi, h, sum, x;
    double TRUEPI = 3.141592653589793238462643;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while (!done)
    {
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (n == 0) break;

        h = 1.0 / (double) n; // Step
        sum = 0.0;
        for (i = rank + 1; i <= n; i += size) {
            x = h * ((double)i - 0.5);
            sum += 4.0 / (1.0 + x*x);
        }
        local_pi = h*sum;

        MPI_Reduce(&local_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            printf("PI = %20.16f: Error = %20.16f\n", pi, fabs(pi-TRUEPI));
            n = 0; // So program can exit.
        }
    }

    MPI_Finalize();

    return 0;
}
