#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 3
#define ITERATIONS 100000

long balance = 0;
pthread_mutex_t balance_mutex = PTHREAD_MUTEX_INITIALIZER;

void* deposit(void* arg){
    int thread_id = *(int*)arg;

    for (int i = 0; i < ITERATIONS; i++){
        pthread_mutex_lock(&balance_mutex);
        balance += 1;
        pthread_mutex_unlock(&balance_mutex);
    }

    printf("Thread %d finished depositing. \n", thread_id);
    return NULL;
}

void run_synchronized_demo(){
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    printf("\n Synchronized Deposit Demo \n");
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

    
}

pthread_mutex_t account_A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t account_B = PTHREAD_MUTEX_INITIALIZER; 

void* transfer_A_to_B(void* arg){
//     printf("THread 1: locking Account A.. \n");
//     pthread_mutex_lock(&account_A);
//     sleep(1); 
//     printf("Thread 1: locking Account B.. \n");
//     pthread_mutex_lock(&account_B);

//     printf("Thread 1: transferring from A to B\n");
//     pthread_mutex_unlock(&account_B);
//     pthread_mutex_unlock(&account_A);
//     return NULL;
}

void* transfer_B_to_A(void* arg){
//     printf("Thread 2: locking Account B.. \n");
//     pthread_mutex_lock(&account_B);
//     sleep(1); 
//     printf("Thread 2: locking Account A.. \n");
//     pthread_mutex_lock(&account_A);

//     printf("Thread 2: transferring from B to A\n");
//     pthread_mutex_unlock(&account_A);
//     pthread_mutex_unlock(&account_B);
//     return NULL;
}

void* safe_transfer_A_to_B(void* arg){
    pthread_mutex_lock(&account_A);
    sleep(1);
    pthread_mutex_lock(&account_B);

    printf("Thread 1 (safe): transferring from A to B \n");
    pthread_mutex_unlock(&account_B);
    pthread_mutex_unlock(&account_A);
    return NULL;
}

void* safe_transfer_B_to_A(void* arg){
    pthread_mutex_lock(&account_A);
    sleep(1);
    pthread_mutex_lock(&account_B);

    printf("Thread 2 (safe): transferring from B to A \n");
    pthread_mutex_unlock(&account_B);
    pthread_mutex_unlock(&account_A);
    return NULL;
}

int main(){
    run_synchronized_demo();

    // printf("\n UNSAFE Deadlock demo (Will crash) \n");
    // pthread_t t1, t2;
    // pthread_create(&t1, NULL, transfer_A_to_B, NULL);
    // pthread_create(&t2, NULL, transfer_B_to_A, NULL);
    // pthread_join(t1, NULL);
    // pthread_join(t2, NULL);
    // printf("WIll not print if deadlock occurs \n");

    printf("\n SAFE Deadlock-Free Demo (consistent lock ordering) \n");
    pthread_t t3, t4;
    pthread_create(&t3, NULL, safe_transfer_A_to_B, NULL);
    pthread_create(&t4, NULL, safe_transfer_B_to_A, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    printf("Both transfers completed successfully - no deadlock.\n");

    return 0;
}
