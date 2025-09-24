#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

// Constants
#define NUM_ACCOUNTS 5
#define NUM_THREADS 4
#define TRANSACTIONS_PER_TELLER 10
#define INITIAL_BALANCE 1000.00

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

    // Seed random number generator per thread
    unsigned int seed = time(NULL) + pthread_self();
    
    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Select random account
        int account_id = rand_r(&seed) % NUM_ACCOUNTS;
        
        // Random amount between -200 and +400
        double amount = (rand_r(&seed) % 600) - 200;
        
        // Perform transaction (THIS WILL HAVE RACE CONDITIONS!)
        printf("Teller %d: Transaction %d - Account %d: %s %.2f\n", 
               teller_id, i, account_id, 
               amount >= 0 ? "Depositing" : "Withdrawing", 
               amount >= 0 ? amount : -amount);
        
        // UNSYNCHRONIZED ACCESS - RACE CONDITION!
        accounts[account_id].balance += amount;
        accounts[account_id].transaction_count++;
        
        // Small delay to increase chance of race conditions
        usleep(1000); // 1 millisecond
    }

    printf("Teller %d: Finished all transactions\n", teller_id);
    return NULL;
}

int main() {
    printf("=== Phase 1: Basic Thread Operations with Race Conditions ===\n");
    
    // Initialize accounts
    printf("Initializing accounts with balance: $%.2f each\n", INITIAL_BALANCE);
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        printf("Account %d: $%.2f\n", i, accounts[i].balance);
    }
    
    // Create threads
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    printf("\nCreating %d teller threads...\n", NUM_THREADS);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
        printf("Created teller thread %d\n", i);
    }
    
    // Wait for all threads to complete
    printf("\nWaiting for all threads to finish...\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Teller %d joined\n", i);
    }
    
    // Calculate expected final balance
    double total_deposits_withdrawals = 0;
    double expected_final_balance = NUM_ACCOUNTS * INITIAL_BALANCE;
    
    // Display final results
    printf("\n=== Final Results ===\n");
    printf("Expected total balance: $%.2f\n", expected_final_balance);
    
    double actual_total_balance = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        actual_total_balance += accounts[i].balance;
        printf("Account %d: $%.2f (%d transactions)\n", 
               i, accounts[i].balance, accounts[i].transaction_count);
    }
    
    printf("\nActual total balance: $%.2f\n", actual_total_balance);
    printf("Discrepancy: $%.2f\n", actual_total_balance - expected_final_balance);
    
    if (actual_total_balance != expected_final_balance) {
        printf("RACE CONDITION DETECTED! Balances are inconsistent.\n");
    } else {
        printf("No race condition detected this time (lucky timing!).\n");
    }
    
    return 0;
}
