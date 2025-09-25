#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 3
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
} Account;

Account accounts[NUM_ACCOUNTS];

// Deterministic transactions (amounts)
double transactions[TRANSACTIONS_PER_TELLER] = {100.0, -50.0, 200.0};
// Mapping transactions to account index
int transaction_accounts[TRANSACTIONS_PER_TELLER] = {0, 1, 0}; // first to acc0, second to acc1, third to acc0

void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        int acc_index = transaction_accounts[i];
        double amount = transactions[i];

        // Read balance
        double read_balance = accounts[acc_index].balance;
        printf("[READ]  Teller %d, txn %d (Acc %d): read %.2f\n",
               teller_id, i + 1, acc_index, read_balance);
        fflush(stdout);

        // Small delay to encourage interleaving
        usleep(100000);

        // Detect intervening write (race)
        double balance_before_write = accounts[acc_index].balance;
        if (balance_before_write != read_balance) {
            printf("  >>> INTERLEAVED (RACE) detected by Teller %d, txn %d on Acc %d: balance changed from %.2f to %.2f BEFORE write\n",
                   teller_id, i + 1, acc_index, read_balance, balance_before_write);
            fflush(stdout);
        } else {
            printf("  (no intervening write) Teller %d, txn %d on Acc %d\n",
                   teller_id, i + 1, acc_index);
            fflush(stdout);
        }

        // Write balance
        double new_balance = read_balance + amount;
        accounts[acc_index].balance = new_balance;
        accounts[acc_index].transaction_count++;

        printf("[WRITE] Teller %d, txn %d (Acc %d): amount %+7.2f, wrote %.2f\n",
               teller_id, i + 1, acc_index, amount, new_balance);
        fflush(stdout);

        usleep(50000); // extra small delay
    }

    return NULL;
}

int main(void) {
    srand(time(NULL));

    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
    }

    // Compute expected balances per account
    double expected[NUM_ACCOUNTS];
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        expected[i] = INITIAL_BALANCE;

    for (int t = 0; t < NUM_THREADS; t++) {
        for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
            int acc_index = transaction_accounts[i];
            expected[acc_index] += transactions[i];
        }
    }

    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        printf("Account %d: %.2f\n", i, accounts[i].balance);
    printf("\n");

    // Create threads
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // Check results
    printf("\nExpected vs Actual balances per account:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d - Expected: %.2f, Actual: %.2f\n",
               i, expected[i], accounts[i].balance);
        if (expected[i] != accounts[i].balance)
            printf(">>> Race condition detected on account %d!\n", i);
    }

    return 0;
}
