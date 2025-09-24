#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 5
#define INITIAL_BALANCE 1000.0

// Shared data structure
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
} Account;

// Global accounts array (shared resource)
Account accounts[NUM_ACCOUNTS];

// Thread function
void* teller_thread(void* arg) {
    int teller_id = *(int*)arg; // Cast void* to int* and dereference

    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Select random account
        // Perform deposit or withdrawal
        // THIS WILL HAVE RACE CONDITIONS!

        printf("Teller %d: Transaction %d\n", teller_id, i);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
    }

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All teller threads have finished.\n");
    return 0;
}
