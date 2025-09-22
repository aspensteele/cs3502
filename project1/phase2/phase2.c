#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 10
#define TRANSACTIONS_PER_TELLER 5
#define INITIAL_BALANCE 1000.0

// Account structure with mutex
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
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
void deposit(int account_id, double amount, int thread_id) {
    pthread_mutex_lock(&accounts[account_id].lock);
    accounts[account_id].balance += amount;
    accounts[account_id].transaction_count++;
    printf("Thread %d: Deposited %.2f, New balance: %.2f\n", 
           thread_id, amount, accounts[account_id].balance);
    pthread_mutex_unlock(&accounts[account_id].lock);
}

// Withdraw function (thread-safe)
void withdraw(int account_id, double amount, int thread_id) {
    pthread_mutex_lock(&accounts[account_id].lock);
    accounts[account_id].balance -= amount;
    accounts[account_id].transaction_count++;
    printf("Thread %d: Withdrew %.2f, New balance: %.2f\n", 
           thread_id, amount, accounts[account_id].balance);
    pthread_mutex_unlock(&accounts[account_id].lock);
}

// Cleanup mutexes
void cleanup_accounts() {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

// Thread function - FIXED: Pass thread ID by value, not reference
void* teller_thread(void* arg) {
    // FIX: Cast directly to avoid race condition on thread_ids array
    long teller_id = (long)arg;
    unsigned int seed = time(NULL) + teller_id;
    
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        usleep(rand_r(&seed) % 10000); // Random pause
        
        // First 5 threads deposit, last 5 withdraw
        if (teller_id < 5) {
            deposit(0, 100.0, teller_id + 1);
        } else {
            withdraw(0, 50.0, teller_id + 1);
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    
    initialize_accounts();
    printf("Initial balance: %.2f\n", accounts[0].balance);
    
    // Create threads - FIX: Pass thread ID by value, not by reference to array
    for (int i = 0; i < NUM_THREADS; i++) {
        // Cast int to void* to avoid thread ID race condition
        if (pthread_create(&threads[i], NULL, teller_thread, (void*)(long)i) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Final balance: %.2f\n", accounts[0].balance);
    printf("Total transactions: %d\n", accounts[0].transaction_count);
    
    // Calculate expected: 5 threads * 5 transactions * $100 - 5 threads * 5 transactions * $50
    double expected_balance = INITIAL_BALANCE + (5 * 5 * 100.0) - (5 * 5 * 50.0);
    
    if (accounts[0].balance == expected_balance) {
        printf("SUCCESS! Balance matches expected: %.2f\n", expected_balance);
    } else {
        printf("ERROR: Expected %.2f but got %.2f\n", expected_balance, accounts[0].balance);
    }
    
    cleanup_accounts();
    return 0;
}
