#include "pthread.h"
#include <stdio.h>

#define NO_THREADS 8

// Have to use "long" because of the size difference or else I get a warning.

void* slave(void* i) {
    unsigned long j = (unsigned long)i;
    printf("Hello world: I am process/thread %d, out of %d processes/threads.\n", j, NO_THREADS);
}

int main(int argc, char** argv) {
    unsigned long i;
    pthread_t thread[NO_THREADS];
    
    for(i = 0; i < NO_THREADS; i++){
        if(pthread_create(&thread[i], NULL, slave, (void *)i) != 0)
        perror("Pthread_create failed.");
    }

    for(i = 0; i < NO_THREADS; i++){
        if(pthread_join(thread[i], NULL) != 0)
        perror("Pthread_join failed.");
    }

    return 0;
}
