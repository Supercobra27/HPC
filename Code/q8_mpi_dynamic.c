#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define X_RESN 800
#define Y_RESN 800

#define DATA_TAG 1
#define TERM_TAG 2
#define RESULT_TAG 3
#define ROW_TAG 4

#define START start = MPI_Wtime();
#define END end = MPI_Wtime();
#define TIMING printf("Total time elapsed: %.6f\n", (end - start));

// i = Row, j = Col
#define IDX(i, j) ((i) * X_RESN + j)

typedef struct complextype
{
    float real, imag;
} Compl;

int main(int argc, char **argv)
{
    int rank, size;

    double start, end;
    int rows_recv = 0;

    int i, j, *k = NULL, *local_k = NULL;
    Compl z, c;
    float lengthsq, temp;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    local_k = (int *)malloc(sizeof(int) * X_RESN);

    MPI_Status status;

    // First Row
    if (rank != 0)
    {
        int current_row;
        // Post call to recv queue to init computation
        MPI_Recv(&current_row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int tag = status.MPI_TAG;
        while (tag == DATA_TAG)
        {
            for (i = 0; i < X_RESN; i++)
            {

                z.real = z.imag = 0.0;
                c.real = ((float)i - 400.0) / 200.0;
                c.imag = ((float)current_row - 400.0) / 200.0;
                local_k[i] = 0;

                do
                { /* iterate for pixel color */
                    temp = z.real * z.real - z.imag * z.imag + c.real;
                    z.imag = 2.0 * z.real * z.imag + c.imag;
                    z.real = temp;
                    lengthsq = z.real * z.real + z.imag * z.imag;
                    local_k[i]++;
                } while (lengthsq < 4.0 && local_k[i] < 100);
            }
            // send both local partition + row number back for correct memcpy
            MPI_Send(local_k, X_RESN, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD);
            MPI_Send(&current_row, 1, MPI_INT, 0, ROW_TAG, MPI_COMM_WORLD);
            // Recv new row, if tag = term_tag cancel
            MPI_Recv(&current_row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            tag = status.MPI_TAG;
        }
    }

    if (rank == 0)
    {
            START
            k = (int *)malloc(sizeof(int)*X_RESN*Y_RESN);
            int count = 0, row = 0, row_acq = 0;

            // Send inital row numbers
        for (int index = 1; index < size; index++)
        {
            MPI_Send(&row, 1, MPI_INT, index, DATA_TAG, MPI_COMM_WORLD);
            count++;
            row++;
        }

        do
        {
            // receive local partition + row number
            MPI_Recv(local_k, X_RESN, MPI_INT, MPI_ANY_SOURCE, RESULT_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&row_acq, 1, MPI_INT, MPI_ANY_SOURCE, ROW_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // copy into memory
            memcpy(&k[row_acq * X_RESN], local_k, sizeof(int)*X_RESN);
            
            count--;

            if (row < Y_RESN)
            {
                // send to processes that have finished computation
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, DATA_TAG, MPI_COMM_WORLD);
                row++;
                count++;
            }
            else if (row == Y_RESN)
            {
                // gets invoked if this is the last row
                MPI_Send(&row, 1, MPI_INT, status.MPI_SOURCE, TERM_TAG, MPI_COMM_WORLD);
            }

            rows_recv++;
        } while (count > 0 && rows_recv < Y_RESN);

        MPI_Barrier(MPI_COMM_WORLD);

        END

        TIMING

        FILE *output;

        output = fopen("mandelbrot_data_dynamic.txt", "w+");
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

    if(k) free(k);

    

    free(local_k);


    MPI_Finalize();

    return 0;
}
