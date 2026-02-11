#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ROOT_NODE 0
#define RECV_NODE 1
#define MIRROR 67
#define N_MSG 61

double compute_relative_gap(double prev_gap, double curr_gap)
{
    return fabs((curr_gap - prev_gap) / prev_gap);
}

int main(int argc, char **argv)
{
    int rank, size;

    int n = 10;
    float epsilon = 0.01; // 1%

    int dummy = 0;

    MPI_Status status;

    double start, end, msg_end, prev_gap = 1.0, curr_gap = 99999.0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2)
        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_COUNT);

    if (!rank)
    {
        char msg = 'm';
        char recv = 'l';
        char* msg_n = NULL;

        double RTT_n, RTT_1, NRTT, g0;

        // initial baseline
        MPI_Send(&n, 1, MPI_INT, RECV_NODE, N_MSG, MPI_COMM_WORLD);

        start = MPI_Wtime();
        for (int i = 0; i < n; i++)
        {
            MPI_Send(&msg, 1, MPI_CHAR, RECV_NODE, 0, MPI_COMM_WORLD);
        }

        MPI_Recv(&recv, 1, MPI_CHAR, RECV_NODE, MIRROR, MPI_COMM_WORLD, &status);
        end = MPI_Wtime();

        prev_gap = (end - start) / n;
        RTT_1 = end - start;

        printf("Initial n=%d gap=%.8f\n", n, prev_gap);
        fflush(stdout);

        // Find g(0)
        while (compute_relative_gap(prev_gap, curr_gap) > epsilon && RTT_1 > RTT_n * epsilon)
        {
            n *= 2;
            prev_gap = curr_gap;

            printf("Starting with %d messages\n", n);

            MPI_Send(&n, 1, MPI_INT, RECV_NODE, N_MSG, MPI_COMM_WORLD);

            start = MPI_Wtime();
            for (int i = 0; i < n; i++)
            {
                MPI_Send(&msg, 1, MPI_CHAR, RECV_NODE, 0, MPI_COMM_WORLD);
            }
            msg_end = MPI_Wtime();

            NRTT = msg_end - start;

            MPI_Recv(&recv, 1, MPI_CHAR, RECV_NODE, MIRROR, MPI_COMM_WORLD, &status);
            end = MPI_Wtime();

            curr_gap = (end - start) / n;
            printf("n=%d gap=%.8f\n", n, curr_gap);
            fflush(stdout);

            RTT_n = end - start;
        }
        int cancel = -1;
        MPI_Send(&cancel, 1, MPI_INT, RECV_NODE, N_MSG, MPI_COMM_WORLD);

        printf("Saturated message count: n=%d gap=%.8f RTT=%.8f\n", n, curr_gap, RTT_n);
        printf("g(0) = %.8f\n", (NRTT / n));
        g0 = NRTT / n;

        printf("2nd RTT...\n");

        double o_s, o_r, RTT_0;

        // Wait for MIRROR to be ready
        MPI_Recv(&dummy, 1, MPI_INT, RECV_NODE, MIRROR, MPI_COMM_WORLD, &status);
        
        // Zero-byte send
        printf("Sending zero byte message...\n");
        start = MPI_Wtime();
        MPI_Send(NULL, 0, MPI_BYTE, RECV_NODE, MIRROR, MPI_COMM_WORLD);
        msg_end = MPI_Wtime();
        MPI_Recv(NULL, 0, MPI_BYTE, RECV_NODE, MIRROR, MPI_COMM_WORLD, &status);
        end = MPI_Wtime();

        printf("Zero byte NRTT %.8f : Zero byte RTT %.8f\n", (msg_end-start), (end-start));
        RTT_0 = end - start;

        printf("L = %.8f s\n", ((RTT_0 - 2*g0)/2));

        double msg_1[5];

        for (int i = 0; i < 19; i++)
        {
        // First RTT
        msg_n = (char *)malloc(sizeof(char)*(1 << i));
        printf("Sending messages for First RTT...\n");
        start = MPI_Wtime();
        MPI_Send(msg_n, (int)(1 << i), MPI_BYTE, RECV_NODE, MIRROR, MPI_COMM_WORLD);
        msg_end = MPI_Wtime();
        MPI_Recv(NULL, 0, MPI_BYTE, RECV_NODE, MIRROR, MPI_COMM_WORLD, &status);
        end = MPI_Wtime();

        printf("NRTT %.8f : RTT %.8f\n", (msg_end-start), (end-start));
        o_s = msg_end - start;

        // Second RTT
        msg_n = (char *)malloc(sizeof(char)*(1 << i));
        printf("Sending messages for second RTT...\n");
        start = MPI_Wtime();
        MPI_Send(NULL, 0, MPI_BYTE, RECV_NODE, MIRROR, MPI_COMM_WORLD);
        msg_end = MPI_Wtime();
        MPI_Recv(msg_n, (int)(1 << i), MPI_BYTE, RECV_NODE, MIRROR, MPI_COMM_WORLD, &status);
        end = MPI_Wtime();

        printf("NRTT %.8f : RTT %.8f\n", (msg_end-start), (end-start));
        o_r = end - msg_end;

        printf("Overhead Times for %d message size:\n", (int)(1 << i));      
        printf("o_s = %.8f s\n", o_s);
        printf("o_r = %.8f s\n", o_r);
        printf("RTT_%d = %.8f\n", (int)(1 << i), (end - start) - RTT_0 + g0);
        if ((int)(1 << i) == 1)
        {
            msg_1[0] = end - start; // RTT
            msg_1[1] = msg_1[0] - RTT_0 + g0; // g
            msg_1[2] = (o_s + o_r) / 2; // o
            msg_1[3] = o_s;
            msg_1[4] = o_r;
        }
        }

        printf("g(0) = %.8f us\n", g0*1000000);
        printf("n = %d\n", n);
        printf("LogP\n");
        printf("L = %.8f us\n", (((RTT_0 - 2*g0)/2) + msg_1[1] - msg_1[3] - msg_1[4])*1000000);
        printf("o = %.8f us\n", msg_1[2]*1000000);
        printf("g = %.8f us\n", msg_1[1]*1000000);
        printf("G = %.9f ns\n", (((end - start) - RTT_0 + g0)/pow(2, 18))*1000000000);
        printf("P = %d\n", size);
    }  

    if (rank)
    {
        char msg = 'a';
        char recv = 'o';
        char* msg_n = (char *)malloc(sizeof(char) * 1024 * 256);
        while (1)
        {
            MPI_Recv(&n, 1, MPI_INT, ROOT_NODE, N_MSG, MPI_COMM_WORLD, &status);
            if (n == -1)
                break;

            for (int i = 0; i < n; i++)
            {
                MPI_Recv(&msg, 1, MPI_CHAR, ROOT_NODE, 0, MPI_COMM_WORLD, &status);
            }
            MPI_Send(&recv, 1, MPI_CHAR, ROOT_NODE, MIRROR, MPI_COMM_WORLD);
        }

        // Ready for 2nd step

        int dummy = 1;
        MPI_Send(&dummy, 1, MPI_INT, ROOT_NODE, MIRROR, MPI_COMM_WORLD);

        // Zero byte msg
        MPI_Recv(NULL, 0, MPI_BYTE, ROOT_NODE, MIRROR, MPI_COMM_WORLD, &status);

        MPI_Send(NULL, 0, MPI_BYTE, ROOT_NODE, MIRROR, MPI_COMM_WORLD);

        // First RTT

        for (int i = 0; i < 19; i++)
        {
        msg_n = realloc(msg_n, sizeof(char)*(1 << i));
        MPI_Recv(msg_n, (int)(1 << i), MPI_BYTE, ROOT_NODE, MIRROR, MPI_COMM_WORLD, &status);

        MPI_Send(NULL, 0, MPI_BYTE, ROOT_NODE, MIRROR, MPI_COMM_WORLD);

        // Second RTT
        MPI_Recv(NULL, 0, MPI_BYTE, ROOT_NODE, MIRROR, MPI_COMM_WORLD, &status);

        MPI_Send(msg_n, (int)(1 << i), MPI_BYTE, ROOT_NODE, MIRROR, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

    return 0;
}
