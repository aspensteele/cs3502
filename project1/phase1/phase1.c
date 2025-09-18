#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // for usleep

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
    int teller_id = *(int*)arg; 
    unsigned int seed = time(NULL) + teller_id;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Random short pause to cause overlaps
        usleep(rand_r(&seed) % 10000); // 0–10 ms

        // Random account (only one here, but ready for multiple)
        int random_account = rand_r(&seed) % NUM_ACCOUNTS;

        // Deposit or withdraw based on thread
        if (teller_id == 0 || teller_id == 1) {
            // Read-modify-write (race condition!)
            double current_balance = accounts[random_account].balance;
            usleep(rand_r(&seed) % 5000); // increase chance of context switch
            accounts[random_account].balance = current_balance + 100.0;

            printf("Thread %d: Depositing 100.00\n", teller_id + 1);
        } else {
            double current_balance = accounts[random_account].balance;
            usleep(rand_r(&seed) % 5000);
            accounts[random_account].balance = current_balance - 50.0;

            printf("Thread %d: Withdrawing 50.00\n", teller_id + 1);
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

    // Perfect sequential expected result
    double expected_balance = INITIAL_BALANCE + (2 * 100 * TRANSACTIONS_PER_TELLER) - (50 * TRANSACTIONS_PER_TELLER);
    if (accounts[0].balance != expected_balance) {
        printf("RACE CONDITION DETECTED! Expected: %.2f\n", expected_balance);
    }

    return 0;
}
