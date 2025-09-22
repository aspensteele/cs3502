#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 5
#define INITIAL_BALANCE 1000.0

// Account structure with mutex
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock; // Mutex to protect account
} Account;

Account accounts[NUM_ACCOUNTS];

// Initialize accounts and mutexes
void initialize_accounts() {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        if (pthread_mutex_init(&accounts[i].lock, NULL) != 0) {
            perror("Mutex init failed");
            exit(EXIT_FAILURE);
        }
    }
}

// Deposit function (thread-safe)
void deposit(int account_id, double amount) {
    pthread_mutex_lock(&accounts[account_id].lock);
    accounts[account_id].balance += amount;
    accounts[account_id].transaction_count++;
    pthread_mutex_unlock(&accounts[account_id].lock);
}

// Withdraw function (thread-safe)
void withdraw(int account_id, double amount) {
    pthread_mutex_lock(&accounts[account_id].lock);
    accounts[account_id].balance -= amount;
    accounts[account_id].transaction_count++;
    pthread_mutex_unlock(&accounts[account_id].lock);
}

// Cleanup mutexes
void cleanup_accounts() {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

// Thread function
void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;
    unsigned int seed = time(NULL) + teller_id;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        usleep(rand_r(&seed) % 10000); // Random pause

        if (teller_id == 0 || teller_id == 1) {
            deposit(0, 100.0);
            printf("Thread %d: Depositing 100.00\n", teller_id + 1);
        } else {
            withdraw(0, 50.0);
            printf("Thread %d: Withdrawing 50.00\n", teller_id + 1);
        }
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    initialize_accounts();

    printf("Initial balance: %.2f\n", accounts[0].balance);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final balance: %.2f\n", accounts[0].balance);

    // Verify result
    double expected_balance = INITIAL_BALANCE + (2 * 100 * TRANSACTIONS_PER_TELLER) - (50 * TRANSACTIONS_PER_TELLER);
    if (accounts[0].balance == expected_balance) {
        printf("No race condition! Balance matches expected: %.2f\n", expected_balance);
    } else {
        printf("ERROR: Expected %.2f but got %.2f\n", expected_balance, accounts[0].balance);
    }

    cleanup_accounts();

    return 0;
}
