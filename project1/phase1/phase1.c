#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 3
#define NUM_THREADS 4
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
    int thread_id = *(int*)arg;

    // Seed random number generator per thread
    unsigned int seed = time(NULL) ^ pthread_self();

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Pick a random account
        int random_account = rand_r(&seed) % NUM_ACCOUNTS;

        // Randomly choose deposit or withdrawal
        int deposit = rand_r(&seed) % 2;
        double amount = 50.0;  // fixed amount for clarity

        if (deposit) {
            accounts[random_account].balance += amount;
            printf("Thread %d: Deposited %.2f into Account %d\n",
                   thread_id, amount, random_account);
        } else {
            accounts[random_account].balance -= amount;
            printf("Thread %d: Withdrew %.2f from Account %d\n",
                   thread_id, amount, random_account);
        }

        accounts[random_account].transaction_count++;
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

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final balances
    printf("\nFinal Account Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: Balance = %.2f, Transactions = %d\n",
               accounts[i].account_id,
               accounts[i].balance,
               accounts[i].transaction_count);
    }

    return 0;
}
