#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define X_RESN 800
#define Y_RESN 800

#define START start = MPI_Wtime();
#define END end = MPI_Wtime();
#define TIMING printf("Total time elapsed: %.6f\n", (end - start));

typedef struct complextype
{
    float real, imag;
} Compl;

int main(int argc, char **argv)
{
    int rank, size;
    double start, end;

    int i, j, *k, *local_k;
    Compl z, c;
    float lengthsq, temp;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // How many rows should each process handle
    int row = Y_RESN / size;

// i = Row, j = Col
#define IDX(i, j) ((i) * X_RESN + j)

    // Allocate local array
    local_k = (int *)malloc(sizeof(int) * X_RESN * row);

    for (j = 0; j < row; j++)
    {
        // Important for global location of element
        int jg = row * rank + j;
        for (i = 0; i < X_RESN; i++)
        {

            z.real = z.imag = 0.0;
            c.real = ((float)i - 400.0) / 200.0;
            c.imag = ((float)jg - 400.0) / 200.0;
            local_k[IDX(j, i)] = 0;

            do
            { /* iterate for pixel color */
                temp = z.real * z.real - z.imag * z.imag + c.real;
                z.imag = 2.0 * z.real * z.imag + c.imag;
                z.real = temp;
                lengthsq = z.real * z.real + z.imag * z.imag;
                local_k[IDX(j, i)]++;
            } while (lengthsq < 4.0 && local_k[IDX(j, i)] < 100);
        }
    }

    if (rank == 0)
    {
        START
        // Allocate main array
        k = (int *)malloc(sizeof(int) * X_RESN * Y_RESN);
    }
    else
    {
        k = NULL;
    }

    // Gather all local computations into central array. Side note: It took me 45 minutes to realize that my problem was to move the collective call OUTSIDE of the if statement above this :(
    MPI_Gather(local_k, row * X_RESN, MPI_INT, k, row * X_RESN, MPI_INT, 0, MPI_COMM_WORLD);


    free(local_k);

    if (rank == 0)
    {

        END

        TIMING
        FILE *output;

        output = fopen("mandelbrot_data_static.txt", "w+");
        if (output == NULL)
        {
            printf("Error opening file\n");
            return -1;
        }

        for (i = 0; i < X_RESN; i++)
            for (j = 0; j < Y_RESN; j++)
            {
                fprintf(output, "%d\n", k[IDX(i, j)]);
            }

        fclose(output);
    }

    if (k)
        free(k);

    MPI_Finalize();

    return 0;
}
