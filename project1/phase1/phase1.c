// phase1_demo.c  -- Phase 1 demo (intentional race condition)
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 1
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
} Account;

Account accounts[NUM_ACCOUNTS];

void* teller_thread(void* arg) {
    int id = *(int*)arg; // 1..NUM_THREADS
    int acct = 0;
    double depositAmount = 100.0;
    double withdrawAmount = 50.0;
    int do_deposit = (id == 1 || id == 2); // threads 1 & 2 deposit, thread 3 withdraw

    // Print the operation (matches PDF style)
    if (do_deposit) {
        printf("Thread %d: Depositing 100.00\n", id);
    } else {
        printf("Thread %d: Withdrawing 50.00\n", id);
    }

    // INTENTIONALLY UNSYNCHRONIZED read-modify-write:
    double old = accounts[acct].balance;

    // Pause to increase chance of overlap (makes race conditions visible)
    usleep(100000); // 100 ms

    if (do_deposit) {
        accounts[acct].balance = old + depositAmount;
    } else {
        accounts[acct].balance = old - withdrawAmount;
    }

    accounts[acct].transaction_count++;
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int tids[NUM_THREADS];

    // init account
    accounts[0].account_id = 0;
    accounts[0].balance = INITIAL_BALANCE;
    accounts[0].transaction_count = 0;

    printf("Initial balance : %.2f\n", accounts[0].balance);

    // create threads (pass 1..3 so output matches sample)
    for (int i = 0; i < NUM_THREADS; ++i) {
        tids[i] = i + 1;
        pthread_create(&threads[i], NULL, teller_thread, &tids[i]);
        // tiny stagger so prints usually appear in order
        usleep(1000);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Final balance : %.2f\n", accounts[0].balance);
    return 0;
}

