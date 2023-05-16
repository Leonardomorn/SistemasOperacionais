#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define BARTTHREAD 0
#define LISATHREAD 1
#define MAGGIETHREAD 2
#define NUM_THREADS  3

#define NUM_STEPS 3

sem_t bart_sem, lisa_sem, maggie_sem;
int pastTurn_global = BARTTHREAD;
void *bartFunc ()
{
    for (int i = 0; i < NUM_THREADS; i++)
    {
        sem_wait (&bart_sem);
        printf("Bart\n");
        pastTurn_global = BARTTHREAD;
        sem_post(&lisa_sem);
    }
    return NULL;
}

void *maggieFunc ()
{
    for (int i = 0; i< NUM_THREADS; i++)
    {
        sem_wait(&maggie_sem);
        printf("\t\tMaggie\n");
        pastTurn_global = MAGGIETHREAD;
        sem_post(&lisa_sem);
    }
    return NULL;
}

void *lisaFunc ()
{
    for (int i = 0; i< 2*NUM_THREADS-1; i++)
    {
        sem_wait(&lisa_sem);
        printf("\tLisa\n");
        if (pastTurn_global == BARTTHREAD)
            sem_post(&maggie_sem);
        else
            sem_post(&bart_sem);
    }
    return NULL;    
}

int main()
{
    pthread_t thread [NUM_THREADS];
    pthread_attr_t attr;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);

    sem_init(&bart_sem, 0, 1); sem_init(&lisa_sem, 0, 0); sem_init(&maggie_sem, 0, 0);
    pthread_create (&thread[BARTTHREAD], &attr, bartFunc, NULL );
    pthread_create (&thread[LISATHREAD], &attr, lisaFunc, NULL );
    pthread_create (&thread[MAGGIETHREAD], &attr, maggieFunc, NULL);



    pthread_exit(NULL);

}