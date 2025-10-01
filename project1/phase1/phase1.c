#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> // For strerror()
#include <errno.h>  // For error codes

#define NUM_ACCOUNTS 2
#define TRANSACTIONS_PER_TELLER 5
#define NUM_THREADS 3
#define BALANCE_INIT 1000.0

// shared data structure
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
} Account;

// global accounts array (shared resource)
Account accounts[NUM_ACCOUNTS];

// track expected balance (no lock in Phase 1)
double expected_balance = NUM_ACCOUNTS * BALANCE_INIT;

void* teller_thread(void* arg) {
    int teller_id = *((int*)arg);

    // initialize a unique seed for this thread
    unsigned int seed = time(NULL) ^ (teller_id * 12345);

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        int random_account = rand_r(&seed) % NUM_ACCOUNTS + 1;

        // check account exists
        if (random_account < 0 || random_account >= NUM_ACCOUNTS) {
            fprintf(stderr, "Thread %d: Invalid account %d\n", teller_id, random_account);
            continue;
        }

        double amount = ((double)rand_r(&seed) / RAND_MAX) * 100.0; // only positive
        if (rand_r(&seed) % 2 == 0) amount = -amount; // randomly flip sign

        double before = accounts[random_account].balance;

        // check for potential overdraft
        if (amount < 0 && before + amount < 0) {
            printf("Thread %d: OVERDRAFT WARNING! Attempted to withdraw %.2f from Account %d (Before: %.2f)\n",
                   teller_id, -amount, random_account, before);
            amount = -before; // withdraw only what is available
        }

        // simulate transaction processing
        usleep(5000);

        // small window to increase chance of race
        usleep(rand_r(&seed) % 100);

        double after = before + amount;
        accounts[random_account].balance = after;
        accounts[random_account].transaction_count++;

        // track expected balance (still unsafe)
        expected_balance += amount;

        if (amount >= 0) {
            printf("Thread %d: Depositing  %.2f into Account %d (Before: %.2f -> After: %.2f)\n",
                   teller_id, amount, random_account, before, after);
        } else {
            printf("Thread %d: Withdrawing %.2f from Account %d (Before: %.2f -> After: %.2f)\n",
                   teller_id, -amount, random_account, before, after);
        }
    }

    return NULL;
}



int main() {
    pthread_t tellers[NUM_THREADS];
    int teller_ids[NUM_THREADS];

    // initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = BALANCE_INIT;
        accounts[i].transaction_count = 0;
    }

    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f\n", i, accounts[i].balance);
    }

    // create teller threads
    for (int i = 0; i < NUM_THREADS; i++) {
        teller_ids[i] = i + 1;
        if (pthread_create(&tellers[i], NULL, teller_thread, &teller_ids[i]) != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // wait for all tellers
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tellers[i], NULL);
    }

    // print results

    printf("\nFinal balances:\n");
    double total_final = 0.0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n",
            i, accounts[i].balance, accounts[i].transaction_count);
        total_final += accounts[i].balance;
    }

    printf("\nTotal Final Balance : %.2f\n", total_final);
    printf("Expected Balance    : %.2f\n", expected_balance);

    if (total_final != expected_balance) {
        printf("// Race condition causes inconsistent results!\n");
    } else {
        printf("// Correct result this time — try running again.\n");
    }

    return 0;
}
