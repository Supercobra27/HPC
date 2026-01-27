#include "omp.h"
#include <math.h>
#include <stdio.h>

#define NUM_THREADS 64

static long num_steps = 100000;
double step;

int main(int argc, char** argv) {
    int i;
    double x, pi, sum[NUM_THREADS];
    double TRUEPI = 3.141592653589793238462643;
    step = 1.0/(double)num_steps;

    omp_set_num_threads(NUM_THREADS);

    printf("Work Sharing Method:\n");
    #pragma omp parallel
    {
        double x; int id;
        id = omp_get_thread_num();
        sum[id] = 0;

        #pragma omp for
            for (i = id;i<num_steps; i++){
                x = (i+0.5)*step;
                sum[id] += 4.0/(1.0+x*x);
            }
    }

    for(i=0, pi=0.0; i<NUM_THREADS; i++) pi += sum[i]*step;

    printf("PI = %20.16f: Error = %20.16f\n", pi, fabs(pi-TRUEPI));


    // Why does this have really bad accuracy?
    printf("Critical Section Method:\n");
    double critical_sum; pi = 0.0;
    #pragma omp parallel private(x, critical_sum)
    {
        for(i=omp_get_thread_num(), critical_sum = 0.0; i<num_steps; i += omp_get_num_threads()){
            x = (i+0.5)*step;
            critical_sum += 4.0/(1.0+x*x);
        }

        #pragma omp critical
            pi += critical_sum*step;
    }

    printf("PI = %20.16f: Error = %20.16f\n", pi, fabs(pi-TRUEPI));

    printf("Reduction Method:\n");
    double reduction_sum = 0.0; pi = 0.0;
    #pragma omp parallel for reduction(+:reduction_sum) private(x)
        for(i=1; i<=num_steps;i++){
            x = (i-0.5)*step;
            reduction_sum += 4.0/(1.0 + x*x);
        }

        pi = step*reduction_sum;

    printf("PI = %20.16f: Error = %20.16f\n", pi, fabs(pi-TRUEPI));
    return 0;
}
