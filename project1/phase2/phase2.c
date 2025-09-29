#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 3
#define INITIAL_BALANCE 1000.0

// Account structure with mutex and transaction count
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock; // Mutex for this account
} Account;

Account accounts[NUM_ACCOUNTS];

// Simple delay to simulate context switches
void delay() {
    for (volatile int i = 0; i < 10000000; i++);
}

// Validate account ID
int is_valid_account(int account_id) {
    return (account_id >= 0 && account_id < NUM_ACCOUNTS);
}

// Thread-safe deposit/withdraw function
void deposit(int account_id, double amount) {
    if (pthread_mutex_lock(&accounts[account_id].lock) != 0) {
        perror("Failed to acquire lock");
        return;
    }

    // Critical section
    double old_balance = accounts[account_id].balance;
    accounts[account_id].balance += amount;
    accounts[account_id].transaction_count++;

    pthread_mutex_unlock(&accounts[account_id].lock);

    const char* type = (amount >= 0) ? "Deposit" : "Withdrawal";
    const char* warning = "";
    if (accounts[account_id].balance < 0) {
        warning = " [OVERDRAFT]";
    }

    printf("%s %+.2f to Account %d (before=%.2f, after=%.2f)%s\n",
           type, amount, account_id, old_balance, accounts[account_id].balance, warning);
}

// Teller thread
void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;
    unsigned int seed = time(NULL) ^ teller_id;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        int acc;
        if (rand_r(&seed) % 5 == 0) {
            acc = NUM_ACCOUNTS + (rand_r(&seed) % 3); // invalid account
        } else {
            acc = rand_r(&seed) % NUM_ACCOUNTS;
        }

        int multiple = (rand_r(&seed) % 4) + 1; // 1-4
        double amount = multiple * 50.0;
        if (rand_r(&seed) % 2 == 0) amount = -amount; // withdrawal

        if (!is_valid_account(acc)) {
            printf("Teller %d: ERROR - Account %d does not exist! Transaction %+.2f skipped.\n",
                   teller_id, acc, amount);
            continue;
        }

        deposit(acc, amount);
        delay();
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Initialize accounts and mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    printf("=== Phase 2 (Thread-Safe with Mutexes) ===\n");

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Wait for threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final balances
    double total_actual = 0;
    printf("\nFinal Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n",
               i, accounts[i].balance, accounts[i].transaction_count);
        total_actual += accounts[i].balance;
    }

    printf("\nTotal money: %.2f\n", total_actual);

    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
