#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ACCOUNTS 1
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 1
#define  INITIAL_BALANCE 1000.0

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
    int teller_id = *(int*)arg; // Cast void * to int * and dereference
    unsigned int seed = time(NULL) + pthread_self(); 

    // Perform multiple transactions
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Select random account
	int random_account = rand_r(&seed) % NUM_ACCOUNTS;
        // Perform deposit or withdrawal
	int deposit = rand_r(&seed) % 2; // 0 = withdrawal 1= deposit
	double depositAmount = 100; 
	double withdrawAmount = 50;
	
       // THIS WILL HAVE RACE CONDITIONS !
	if (deposit) {
	  accounts[random_account].balance += depositAmount;
	  printf("Thread %d: Deposited %.2f (Balance now: %.2f)\n",
		teller_id,depositAmount, accounts[random_account].balance);
	}
	else {
		accounts[random_account].balance -= withdrawAmount;
		printf("Thread %d: Withdrew %.2f (Balance now: %.2f)\n",
		teller_id,withdrawAmount, accounts[random_account].balance);
	}
	accounts[random_account].transaction_count++;
       // printf("Teller %d: Transaction %d\n", teller_id, i);
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = 1000.0;
        accounts[i].transaction_count = 0;
    }

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
	
	printf("Final Balance: %.2f\n", accounts[0].balance);
    return 0;
}
