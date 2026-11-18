#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_CONCURRENT_ACCESS 5

sem_t semaphore;

void *worker(void *arg)
{
    int id = *(int *)arg;

    sem_wait(&semaphore);

    printf("Thread %d entered the resource pool\n", id);

    sleep(3);

    printf("Thread %d released its permit\n", id);

    sem_post(&semaphore);

    return NULL;
}

int main(void)
{
    pthread_t threads[10];
    int thread_ids[10];

    sem_init(&semaphore, 0, MAX_CONCURRENT_ACCESS);

    for (int i = 0; i < 10; i++)
    {
        thread_ids[i] = i + 1;

        pthread_create(
            &threads[i],
            NULL,
            worker,
            &thread_ids[i]
        );
    }

    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore);

    return 0;
}