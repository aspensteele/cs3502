#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>   // for usleep

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 1
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

    // Select random account
    int random_account = rand_r(&seed) % NUM_ACCOUNTS;

    // Random short pause to cause overlaps
    usleep(rand_r(&seed) % 2000); // 0–2 ms pause

    // Fixed roles: threads 0 & 1 deposit, thread 2 withdraws
    if (teller_id == 0 || teller_id == 1) {
        // VULNERABLE TO RACE CONDITION: Read-modify-write pattern
        double current_balance = accounts[random_account].balance; // Read
        usleep(rand_r(&seed) % 1000); // Increase chance of context switch
        accounts[random_account].balance = current_balance + 100.0; // Write
        
        printf("Thread %d: Depositing 100.00\n", teller_id + 1);
    } else {
        // VULNERABLE TO RACE CONDITION: Read-modify-write pattern  
        double current_balance = accounts[random_account].balance; // Read
        usleep(rand_r(&seed) % 1000); // Increase chance of context switch
        accounts[random_account].balance = current_balance - 50.0; // Write
        
        printf("Thread %d: Withdrawing 50.00\n", teller_id + 1);
    }

    accounts[random_account].transaction_count++;
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
    
    // Show expected result for comparison
    double expected_balance = INITIAL_BALANCE + 100.0 + 100.0 - 50.0;
    if (accounts[0].balance != expected_balance) {
        printf("RACE CONDITION DETECTED! Should be: %.2f\n", expected_balance);
    }
    
    return 0;
}
