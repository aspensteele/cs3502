#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 2
#define TRANSACTIONS_PER_THREAD 5
#define BALANCE_INIT 1000.0
#define LOCK_TIMEOUT_SEC 5

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

// Timeout detection for stuck threads
int lock_with_timeout(pthread_mutex_t* lock, int account_id) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += LOCK_TIMEOUT_SEC;
    
    int result = pthread_mutex_timedlock(lock, &ts);
    
    if (result == ETIMEDOUT) {
        printf("Thread %ld: TIMEOUT waiting for account %d — DEADLOCK DETECTED!\n", 
               pthread_self(), account_id);
        return -1;
    }
    return result;
}

void transfer(int from_id, int to_id, double amount) {
    printf("Thread %ld: Attempting transfer from %d to %d of %.2f\n", 
           pthread_self(), from_id, to_id, amount);
    
    // Lock source account with timeout
    if (lock_with_timeout(&accounts[from_id].lock, from_id) != 0) {
        return;
    }
    printf("Thread %ld: Locked account %d\n", pthread_self(), from_id);
    
    usleep(100000); // Delay to increase deadlock chance
    
    // Waiting for the second account
    printf("Thread %ld: Waiting for account %d — possible deadlock!\n", 
           pthread_self(), to_id);
    
    if (lock_with_timeout(&accounts[to_id].lock, to_id) != 0) {
        pthread_mutex_unlock(&accounts[from_id].lock);
        return;
    }
    printf("Thread %ld: Locked account %d\n", pthread_self(), to_id);
    
    // Perform transfer
    accounts[from_id].balance -= amount;
    accounts[to_id].balance += amount;
    accounts[from_id].transaction_count++;
    accounts[to_id].transaction_count++;
    
    pthread_mutex_unlock(&accounts[to_id].lock);
    pthread_mutex_unlock(&accounts[from_id].lock);
    
    printf("Thread %ld: Completed transfer from %d to %d\n", 
           pthread_self(), from_id, to_id);
}

void* teller_thread(void* arg) {
    int tid = *((int*)arg);
    
    for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
        int from, to;
        
        // Create reliable deadlock: opposite locking order
        if (tid == 0) {
            from = 0;
            to = 1;
        } else {
            from = 1;
            to = 0;
        }
        
        double amount = 50.0;
        transfer(from, to, amount);
        usleep(10000);
    }
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    // Initialize accounts and mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = BALANCE_INIT;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
    
    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        printf("Account %d: %.2f\n", i, accounts[i].balance);
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }
    
    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    
    // Print final balances
    printf("\nFinal balances:\n");
    double total = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n", 
               i, accounts[i].balance, accounts[i].transaction_count);
        total += accounts[i].balance;
    }
    printf("Total balance: %.2f\n", total);
    
    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
    
    return 0;
}
