#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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
    int teller_id = *(int*)arg; // Cast void* to int* and dereference
    
    // Seed random number generator per thread
    unsigned int seed = time(NULL) + pthread_self();
    
    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Select random account
        int account_id = rand_r(&seed) % NUM_ACCOUNTS;
        
        // Perform deposit or withdrawal
        // THIS WILL HAVE RACE CONDITIONS!
        
        double amount = (rand_r(&seed) % 100) + 10; // $10-$109
        int operation = rand_r(&seed) % 2; // 0 = deposit, 1 = withdraw
        
        if (operation == 0) {
            // Deposit
            printf("Teller %d: Transaction %d - Depositing $%.2f to Account %d\n", 
                   teller_id, i, amount, account_id);
            
            // RACE CONDITION: Read-modify-write without protection
            double old_balance = accounts[account_id].balance;
            usleep(100); // Small delay to increase chance of race condition
            accounts[account_id].balance = old_balance + amount;
            accounts[account_id].transaction_count++;
            
        } else {
            // Withdraw
            printf("Teller %d: Transaction %d - Withdrawing $%.2f from Account %d\n", 
                   teller_id, i, amount, account_id);
            
            // RACE CONDITION: Read-modify-write without protection
            double old_balance = accounts[account_id].balance;
            if (old_balance >= amount) {
                usleep(100); // Small delay to increase chance of race condition
                accounts[account_id].balance = old_balance - amount;
                accounts[account_id].transaction_count++;
            } else {
                printf("Teller %d: Insufficient funds in Account %d\n", teller_id, account_id);
            }
        }
        
        printf("Teller %d: Transaction %d complete\n", teller_id, i);
    }
    
    return NULL;
}

int main() {
    printf("=== Phase 1: Basic Thread Operations ===\n");
    printf("Demonstrating race conditions with %d threads and %d accounts\n\n", 
           NUM_THREADS, NUM_ACCOUNTS);
    
    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
    }
    
    // Print initial balances
    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%.2f\n", i, accounts[i].balance);
    }
    printf("\n");
    
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
    
    // Print final balances
    printf("\nFinal balances:\n");
    double total_balance = 0.0;
    int total_transactions = 0;
    
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%.2f (Transactions: %d)\n", 
               i, accounts[i].balance, accounts[i].transaction_count);
        total_balance += accounts[i].balance;
        total_transactions += accounts[i].transaction_count;
    }
    
    printf("\nTotal balance: $%.2f\n", total_balance);
    printf("Expected total: $%.2f\n", NUM_ACCOUNTS * INITIAL_BALANCE);
    printf("Total transactions processed: %d\n", total_transactions);
    
    if (total_balance != (NUM_ACCOUNTS * INITIAL_BALANCE)) {
        printf("\n*** RACE CONDITION DETECTED! ***\n");
        printf("Balance mismatch indicates race conditions occurred!\n");
    }
    
    printf("\nRun this program multiple times to see different results!\n");
    
    return 0;
}
