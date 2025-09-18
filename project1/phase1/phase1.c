#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
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
    
    // Introduce a random delay to increase chance of race conditions
    unsigned int seed = time(NULL) + teller_id;
    usleep(rand_r(&seed) % 2000); // 0-2ms delay
    
    if (teller_id == 0 || teller_id == 1) {
        // DEPOSIT - vulnerable to race conditions
        double current_balance = accounts[0].balance;
        printf("Thread %d: Read balance %.2f for deposit of 100.00\n", 
               teller_id, current_balance);
        
        // Small delay to increase chance of context switch
        usleep(rand_r(&seed) % 1000);
        
        accounts[0].balance = current_balance + 100.0;
        printf("Thread %d: Deposited 100.00 (Balance now: %.2f)\n",
               teller_id, accounts[0].balance);
    } else {
        // WITHDRAWAL - vulnerable to race conditions
        double current_balance = accounts[0].balance;
        printf("Thread %d: Read balance %.2f for withdrawal of 50.00\n", 
               teller_id, current_balance);
        
        // Small delay to increase chance of context switch
        usleep(rand_r(&seed) % 1000);
        
        accounts[0].balance = current_balance - 50.0;
        printf("Thread %d: Withdrew 50.00 (Balance now: %.2f)\n",
               teller_id, accounts[0].balance);
    }
    
    accounts[0].transaction_count++;
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

    printf("Initial Balance: %.2f\n", accounts[0].balance);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final Balance: %.2f\n", accounts[0].balance);
    
    // Calculate what the balance should be without race conditions
    double expected_balance = INITIAL_BALANCE + 100.0 + 100.0 - 50.0;
    printf("Expected Balance: %.2f\n", expected_balance);
    
    if (accounts[0].balance != expected_balance) {
        printf("RACE CONDITION DETECTED! Difference: %.2f\n", 
               accounts[0].balance - expected_balance);
    } else {
        printf("No race condition occurred this time (try running again)\n");
    }
    
    return 0;
}
