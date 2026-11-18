#include <pthread.h>
#include <stdio.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_counter = 0;

void *worker(void *arg){

    int threadNumber = (int)(long)arg;
    pthread_mutex_lock(&mutex);
    int initialized_shared_counter = shared_counter;

    for (size_t i = 0; i < 10000; i++){
        shared_counter++;
    }

    printf("Shared counter value from %d to %d for threadNumber: %d\n", initialized_shared_counter, shared_counter, threadNumber);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(void){

    pthread_t t1, t2;

    pthread_create(&t1, NULL, worker, (void*)(long)1);
    pthread_create(&t2, NULL, worker, (void*)(long)2);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}