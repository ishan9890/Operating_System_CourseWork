#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 3
#define ITERATIONS 100000

long balance = 0;

void* deposit(void* arg){
    int thread_id = *(int*)arg;

    for (int i = 0; i < ITERATIONS; i++){
        balance += 1;
    }

    printf("Thread %d finished depositing. \n", thread_id);
    return NULL;
}

int main(){
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    printf("Starting balance: %ld\n", balance);

    for (int i = 0; i < NUM_THREADS; i++){
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, deposit, &thread_ids[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Final balance: %ld\n", balance);
    printf("Expected balance: %ld\n", (long)NUM_THREADS * ITERATIONS);

    return 0;
}