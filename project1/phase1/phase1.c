#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ACCOUNTS 1          // Just one account to keep it simple
#define NUM_THREADS 3           // Three threads as requested
#define TRANSACTIONS_PER_TELLER 10 
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
    int teller_id = *(int*)arg; // Cast void* to int* and dereference
    
    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Select account (only account 0 exists)
        int account_id = 0;
        double amount = (rand() % 100) + 1;
        
        // Perform deposit or withdrawal
        // THIS WILL HAVE RACE CONDITIONS!
        if (rand() % 2 == 0) {
            printf("Thread %d: Depositing %.2f\n", teller_id, amount);
            accounts[account_id].balance += amount;
        } else {
            if (accounts[account_id].balance >= amount) {
                printf("Thread %d: Withdrawing %.2f\n", teller_id, amount);
                accounts[account_id].balance -= amount;
            }
        }
        
        accounts[account_id].transaction_count++;
        printf("Teller %d: Transaction %d\n", teller_id, i);
    }
    return NULL;
}

int main() {
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize the single account
    accounts[0].account_id = 0;
    accounts[0].balance = INITIAL_BALANCE;
    accounts[0].transaction_count = 0;
    
    printf("Initial balance: %.2f\n", accounts[0].balance);
    
    // Creating threads
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Final balance: %.2f\n", accounts[0].balance);
    
    return 0;
}
