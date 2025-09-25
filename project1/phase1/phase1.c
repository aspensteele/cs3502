#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 2
#define TRANSACTIONS_PER_TELLER 3
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
} Account;

Account accounts[NUM_ACCOUNTS];

// Predefined transactions: + = deposit, - = withdraw
double transactions[NUM_THREADS][TRANSACTIONS_PER_TELLER] = {
    {100, -50, 200},   // Teller 0
    {50, 50, -100}     // Teller 1
};

// Thread function
void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;
    unsigned int seed = time(NULL) ^ teller_id;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        int acc = rand_r(&seed) % NUM_ACCOUNTS; // random account
        double amount = transactions[teller_id][i];

        double old_balance = accounts[acc].balance;
        // Race condition here: read-modify-write without protection
        accounts[acc].balance = old_balance + amount;

        printf("Teller %d: %+.2f to Account %d (before=%.2f, after=%.2f)\n",
               teller_id, amount, acc, old_balance, accounts[acc].balance);

        usleep(1000); // encourage interleaving
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
    }

    // Compute total expected (based on all transactions)
    double total_expected = NUM_ACCOUNTS * INITIAL_BALANCE;
    for (int t = 0; t < NUM_THREADS; t++) {
        for (int j = 0; j < TRANSACTIONS_PER_TELLER; j++) {
            total_expected += transactions[t][j];
        }
    }

    printf("=== Starting Phase 1 (Race Condition Demo) ===\n");

    // Create teller threads
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
        printf("Account %d: %.2f\n", i, accounts[i].balance);
        total_actual += accounts[i].balance;
    }

    // Verification step
    printf("\nTotal money: %.2f (should be %.2f)\n", total_actual, total_expected);

    return 0;
}
