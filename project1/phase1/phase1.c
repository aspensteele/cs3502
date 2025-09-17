#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> // For strerror()
#include <errno.h> // For error codes

#define NUM_ACCOUNTS 3 
#define NUM_THREADS 2 //start with 2 and then change later
#define TRANSACTIONS_PER_TELLER 30 
#define INITIAL_BALANCE 1000.0 //starting balance per account

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

    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Select random account
	int account_id = rand() % NUM_ACCOUNTS;

	// Select a random amount to withdrawal or add 
	double amount = (rand() % 100) + 1; 

        // Perform deposit or withdrawal
	if (rand() % 2 == 0) {
	//unsafe deposit
	accounts[account-id].balance += amount;
	} else {
        // THIS WILL HAVE RACE CONDITIONS!
	if (accounts[account_id].balance >= amount) {
	    accounts[account_id].balance -= amount;
	}
    }
       	 accounts[account_id].transaction_count++; 
	 printf("Teller %d: Transaction %d\n", teller_id, i);
    }

    return NULL;
}

// Creating threads (see Appendix \ref{sec:voidpointer} for void* explanation)
pthread_t threads[NUM_THREADS];
int thread_ids[NUM_THREADS];

for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
}
