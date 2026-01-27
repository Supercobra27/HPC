#include "pthread.h"
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>

#define NO_THREADS 16
#define ARR_LENGTH 256

pthread_mutex_t mutex;
int a[ARR_LENGTH];
int stride = ARR_LENGTH / NO_THREADS;
int gbl_min = INT_MAX;

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec * 1e-6;
}

int mod_rand(int x)
{
    return ((x * 37 + 17) % 100) + 1;
}

void *slave(void *i)
{
    unsigned long j = (unsigned long)i;
    int min = INT_MAX;

    for (int k = 0; k < stride; k++)
    {
        if (min > a[(stride * (int)j) + k])
            min = a[(stride * (int)j) + k];
    }

    pthread_mutex_lock(&mutex);
    if (gbl_min > min)
        gbl_min = min;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char **argv)
{
    unsigned long i;
    pthread_t thread[NO_THREADS];

    for (int i = 0; i < ARR_LENGTH; i++)
    {
        a[i] = mod_rand(i);
    }

    double start, end;

    start = get_time();

    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < NO_THREADS; i++)
    {
        if (pthread_create(&thread[i], NULL, slave, (void *)i) != 0)
            perror("Pthread_create failed.");
    }

    for (i = 0; i < NO_THREADS; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
            perror("Pthread_join failed.");
    }

    end = get_time();

    printf("Global Minimum: %d Time: %.6f\n", gbl_min, end - start);

    return 0;
}
