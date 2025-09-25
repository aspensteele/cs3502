#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 3
#define INITIAL_BALANCE 1000.0  

// Account structure with mutex
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

// Predefined transactions (same as Phase 1)
double transactions[NUM_THREADS][TRANSACTIONS_PER_TELLER] = {
    {150, -100, 50},   // Teller 0
    {120, -80, 100},   // Teller 1
    {100, -150, 200}   // Teller 2
};

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

// Thread-safe transaction function
void apply_transaction(int thread_id, int txn_index) {
    int acc = 0; // Only one account for simplicity
    double amount = transactions[thread_id][txn_index];

    pthread_mutex_lock(&accounts[acc].lock);  // Enter critical section
    double old_balance = accounts[acc].balance;
    accounts[acc].balance += amount;
    accounts[acc].transaction_count++;
    pthread_mutex_unlock(&accounts[acc].lock); // Exit critical section

    const char* type = (amount >= 0) ? "Deposit" : "Withdrawal";
    printf("Thread %d: %s %+.2f (before=%.2f, after=%.2f)\n",
           thread_id, type, amount, old_balance, accounts[acc].balance);
}

// Thread function
void* teller_thread(void* arg) {
    long teller_id = (long)arg;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        usleep(1000); // Small delay to simulate interleaving
        apply_transaction((int)teller_id, i);
    }

    return NULL;
}

// Cleanup mutexes
void cleanup_accounts() {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

int main() {
    pthread_t threads[NUM_THREADS];

    initialize_accounts();
    printf("Initial balance: %.2f\n", accounts[0].balance);

    // Create threads
    for (long i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, teller_thread, (void*)i) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final balance
    printf("\nFinal balance: %.2f\n", accounts[0].balance);
    printf("Total transactions: %d\n", accounts[0].transaction_count);

    // Calculate expected balance
    double total_expected = INITIAL_BALANCE;
    for (int t = 0; t < NUM_THREADS; t++) {
        for (int j = 0; j < TRANSACTIONS_PER_TELLER; j++) {
            total_expected += transactions[t][j];
        }
    }

    if (accounts[0].balance == total_expected) {
        printf("SUCCESS! Balance matches expected: %.2f\n", total_expected);
    } else {
        printf("ERROR: Expected %.2f but got %.2f\n", total_expected, accounts[0].balance);
    }

    cleanup_accounts();
    return 0;
}
