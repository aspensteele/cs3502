#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>   // For strerror()
#include <errno.h>    // For error codes
#include <sys/time.h> // For timing

#define NUM_ACCOUNTS 2
#define TRANSACTIONS_PER_TELLER 5
#define NUM_THREADS 3
#define BALANCE_INIT 1000.0

// shared data structure
typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;  // mutex for each account
} Account;

// global accounts array (shared resource)
Account accounts[NUM_ACCOUNTS];

// track expected balance with its own mutex
double expected_balance = NUM_ACCOUNTS * BALANCE_INIT;
pthread_mutex_t expected_balance_lock = PTHREAD_MUTEX_INITIALIZER;

// timing function
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void* teller_thread(void* arg) {
    int teller_id = *((int*)arg);
    unsigned int seed = time(NULL) ^ (teller_id * 12345); // persistent seed

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        int random_account = rand_r(&seed) % NUM_ACCOUNTS;

        double amount = ((double)rand_r(&seed) / RAND_MAX) * 100.0;
        if (rand_r(&seed) % 2 == 0) amount = -amount;

        // lock account for safe access
        pthread_mutex_lock(&accounts[random_account].lock);

        double before = accounts[random_account].balance;

        if (amount < 0 && before + amount < 0) {
            printf("Thread %d: OVERDRAFT WARNING! Attempted to withdraw %.2f from Account %d (Before: %.2f)\n",
                   teller_id, -amount, random_account, before);
            amount = -before;
        }

        usleep(5000);
        usleep(rand_r(&seed) % 100);

        double after = before + amount;
        accounts[random_account].balance = after;
        accounts[random_account].transaction_count++;

        if (amount >= 0) {
            printf("Thread %d: Depositing  %.2f into Account %d (Before: %.2f -> After: %.2f)\n",
                   teller_id, amount, random_account, before, after);
        } else {
            printf("Thread %d: Withdrawing %.2f from Account %d (Before: %.2f -> After: %.2f)\n",
                   teller_id, -amount, random_account, before, after);
        }

        pthread_mutex_unlock(&accounts[random_account].lock);

        // safely update expected balance with separate lock
        pthread_mutex_lock(&expected_balance_lock);
        expected_balance += amount;
        pthread_mutex_unlock(&expected_balance_lock);
    }

    return NULL;
}

int main() {
    pthread_t tellers[NUM_THREADS];
    int teller_ids[NUM_THREADS];

    // initialize accounts and mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = BALANCE_INIT;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    printf("=== Phase 2: Multi-threaded Banking with Mutexes ===\n");

    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f\n", i, accounts[i].balance);
    }
    printf("\n");

    // start timing
    double start_time = get_time();

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

    // end timing
    double end_time = get_time();

    // print results
    printf("\nFinal balances:\n");
    double total_final = 0.0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n",
               i, accounts[i].balance, accounts[i].transaction_count);
        total_final += accounts[i].balance;
    }

    printf("\n=== Results ===\n");
    printf("Total Final Balance : %.2f\n", total_final);
    printf("Expected Balance    : %.2f\n", expected_balance);
    printf("Execution Time      : %.6f seconds\n", end_time - start_time);

    if (total_final != expected_balance) {
        printf("// ERROR: Race condition causes inconsistent results!\n");
    } else {
        printf("// SUCCESS: Correct result with mutexes — no race conditions!\n");
    }

    // destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
    pthread_mutex_destroy(&expected_balance_lock);

    return 0;
}
