#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 5
#define NUM_THREADS 10
#define TRANSACTIONS_PER_THREAD 1000
#define BALANCE_INIT 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

void transfer(int from_id, int to_id, double amount) {
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    pthread_mutex_lock(&accounts[first].lock);
    pthread_mutex_lock(&accounts[second].lock);

    if (accounts[from_id].balance >= amount) {
        double before_from = accounts[from_id].balance;
        double before_to = accounts[to_id].balance;

        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;
        accounts[from_id].transaction_count++;
        accounts[to_id].transaction_count++;

        printf("Thread %ld: Transfer %.2f from Account %d (%.2f -> %.2f) to Account %d (%.2f -> %.2f)\n",
               pthread_self(), amount,
               from_id, before_from, accounts[from_id].balance,
               to_id, before_to, accounts[to_id].balance);
    }

    pthread_mutex_unlock(&accounts[second].lock);
    pthread_mutex_unlock(&accounts[first].lock);
}


void* teller_thread(void* arg) {
    int tid = *((int*)arg);
    unsigned int seed = time(NULL) ^ tid;  // unique seed per thread

    for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
        int from = rand_r(&seed) % NUM_ACCOUNTS;
        int to;
        do {
            to = rand_r(&seed) % NUM_ACCOUNTS;
        } while (to == from);

        double amount = ((double)rand_r(&seed) / RAND_MAX) * 100.0;

        transfer(from, to, amount);
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

    // Start performance timer
    clock_t start = clock();

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // Stop performance timer
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    // Print final balances
    printf("\nFinal balances:\n");
    double total = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n", 
               i, accounts[i].balance, accounts[i].transaction_count);
        total += accounts[i].balance;
    }
    printf("Total balance: %.2f\n", total);
    printf("Execution time: %.2f seconds\n", elapsed);

    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
