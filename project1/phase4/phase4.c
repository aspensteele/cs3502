#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> // For strerror()
#include <errno.h>  // For error codes

#define NUM_ACCOUNTS 2
#define NUM_THREADS 2
#define TRANSACTIONS_PER_THREAD 5
#define BALANCE_INIT 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock; // Mutex for this account
} Account;

Account accounts[NUM_ACCOUNTS];


void safe_transfer (int tid, int from_id , int to_id , double amount ) {
    // Always lock lower ID first
    int first = (from_id < to_id) ? from_id : to_id ;
    int second = (from_id < to_id) ? to_id : from_id ;

    pthread_mutex_lock(&accounts[first].lock);
    pthread_mutex_lock(&accounts[second].lock);

    // Capture before balances while holding locks
    double before_from = accounts[from_id].balance;
    double before_to = accounts[to_id].balance;

    // Perform transfer
    accounts[from_id].balance -= amount;
    accounts[to_id].balance += amount;
    accounts[from_id].transaction_count++;
    accounts[to_id].transaction_count++;

    // Capture after balances
    double after_from = accounts[from_id].balance;
    double after_to = accounts[to_id].balance;

    pthread_mutex_unlock(&accounts[second].lock);
    pthread_mutex_unlock(&accounts[first].lock);

    // Print before/after for both accounts so interleavings are visible
    printf("Thread %d: Completed transfer %d -> %d | from: %8.2f -> %8.2f ; to: %8.2f -> %8.2f\n",
        tid, from_id, to_id,
        before_from, after_from, before_to, after_to);
}



void* teller_thread(void* arg) {
    int tid = *((int*)arg);
    unsigned int seed = time(NULL) ^ (tid * 12345);

    for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
        int from = rand_r(&seed) % NUM_ACCOUNTS;
        int to = (from + 1) % NUM_ACCOUNTS; // pick the other account
        double amount = ((double)rand_r(&seed) / RAND_MAX) * 100.0;

    safe_transfer(tid, from, to, amount);
        usleep(rand_r(&seed) % 1000);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Initialize accounts and mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = BALANCE_INIT;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        printf("Account %d: %.2f\n", i, accounts[i].balance);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // Print final balances
    printf("\nFinal balances:\n");
    double total = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n", i, accounts[i].balance, accounts[i].transaction_count);
        total += accounts[i].balance;
    }
    printf("Total balance: %.2f\n", total);

    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
