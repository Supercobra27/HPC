#include "pthread.h"
#include <stdio.h>
#include <math.h>

#define NO_THREADS 64


pthread_mutex_t mutex;
static long num_steps = 100000;
double TRUEPI = 3.141592653589793238462643;
double sum = 0.0;


void* slave(void* i) {
    unsigned long j = (unsigned long)i;
    double step = 1.0 / (double) num_steps;
    double x, local_sum;
    for (int k = (int)j + 1; k <=num_steps; k += NO_THREADS){
        x = step*(k-0.5);
        local_sum += 4/(1.0 + x*x);
    }

    pthread_mutex_lock(&mutex);
    sum += local_sum*step;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char** argv) {
    unsigned long i;
    pthread_t thread[NO_THREADS];
    pthread_mutex_init(&mutex, NULL);
    
    for(i = 0; i < NO_THREADS; i++){
        if(pthread_create(&thread[i], NULL, slave, (void *)i) != 0)
        perror("Pthread_create failed.");
    }

    for(i = 0; i < NO_THREADS; i++){
        if(pthread_join(thread[i], NULL) != 0)
        perror("Pthread_join failed.");
    }

    printf("PI = %20.16f: Error = %20.16f\n", sum, fabs(sum-TRUEPI));

    return 0;
}
