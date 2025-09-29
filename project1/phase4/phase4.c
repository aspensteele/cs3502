#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2

// Account structure
typedef struct {
    int account_id;
    double balance;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

// Phase 4: Safe transfer function
void safe_transfer(int from_id, int to_id, double amount) {
    // Always lock lower ID first to prevent deadlock
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    pthread_mutex_lock(&accounts[first].lock);
    pthread_mutex_lock(&accounts[second].lock);

    // Perform transfer
    accounts[from_id].balance -= amount;
    accounts[to_id].balance += amount;

    pthread_mutex_unlock(&accounts[second].lock);
    pthread_mutex_unlock(&accounts[first].lock);

    printf("Thread %ld: Transferred %.2f from Account %d to Account %d\n",
           pthread_self(), amount, from_id, to_id);
}

// Teller thread wrapper
void* teller_thread(void* arg) {
    int* ids = (int*)arg;
    safe_transfer(ids[0], ids[1], 50);
    return NULL;
}

int main() {
    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = 1000;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    pthread_t t1, t2;
    int args1[2] = {0, 1}; // Transfer Account 0 -> 1
    int args2[2] = {1, 0}; // Transfer Account 1 -> 0

    // Create threads
    pthread_create(&t1, NULL, teller_thread, args1);
    pthread_create(&t2, NULL, teller_thread, args2);

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Print final balances
    printf("\nFinal Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f\n", i, accounts[i].balance);
    }

    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
