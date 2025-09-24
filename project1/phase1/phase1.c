#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 1
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

// Global trackers for expected balance
double total_deposits = 0.0;
double total_withdrawals = 0.0;

// Thread function
void* teller_thread(void* arg) {
    int teller_id = *(int*)arg; // Cast void* to int* and dereference
    unsigned int seed = time(NULL) + teller_id;

    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Random short pause to cause overlaps
        usleep(rand_r(&seed) % 10000); // 0–10 ms

        // Select random account 
        int random_account = rand_r(&seed) % NUM_ACCOUNTS;

        // THIS WILL HAVE RACE CONDITIONS!
        if (teller_id % 2 == 0) {
            // Even threads deposit
            double amount = (rand_r(&seed) % 901) + 100; // random 100–1000
            double current_balance = accounts[random_account].balance;
            usleep(rand_r(&seed) % 5000);
            accounts[random_account].balance = current_balance + amount;

            // Track for expected balance
            __sync_fetch_and_add((long long*)&total_deposits, (long long)amount);

            printf("Teller %d: Transaction %d - Depositing %.2f\n", teller_id, i + 1, amount);
        } else {
            // Odd threads withdraw
            double amount = (rand_r(&seed) % 451) + 50; // random 50–500
            double current_balance = accounts[random_account].balance;
            usleep(rand_r(&seed) % 5000);
            accounts[random_account].balance = current_balance - amount;

            // Track for expected balance
            __sync_fetch_and_add((long long*)&total_withdrawals, (long long)amount);

            printf("Teller %d: Transaction %d - Withdrawing %.2f\n", teller_id, i + 1, amount);
        }

        accounts[random_account].transaction_count++;
    }

    return NULL;
}

int main() {
    // Creating threads
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

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final balance: %.2f\n", accounts[0].balance);

    // Expected balance = Initial + deposits – withdrawals
    double expected_balance = INITIAL_BALANCE + total_deposits - total_withdrawals;
    printf("Expected balance: %.2f\n", expected_balance);

    if (accounts[0].balance != expected_balance) {
        printf("RACE CONDITION DETECTED! Difference: %.2f\n",
               expected_balance - accounts[0].balance);
    } else {
        printf("No race condition detected this run (try running again)\n");
    }

    return 0;
}
