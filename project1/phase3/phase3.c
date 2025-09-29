//Phase 3: Deadlock Creation
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> 
#include <errno.h>

#define NUM_ACCOUNTS 2

typedef struct {
	int account_id;
	double balance;
	int transaction_count;
	pthread_mutex_t lock; // mutex for thus account
} Account; 


Account accounts[NUM_ACCOUNTS];

void transfer(int from_id, int to_id, double amount) {
	printf("Thread %ld: Attempting transfer from %d to %d\n",
	      pthread_self(), from_id, to_id); 

  	pthread_mutex_lock(&accounts[from_id].lock);
	printf("Thread %ld: Locked account %d\n", pthread_self(), from_id);
	
	//simulate processing delay - gives other thread time to create deadlock
	usleep(100); //sleep for 100 microseconds

	printf("Thread %ld: Waiting for account %d\n", pthread_self(), to_id);
	pthread_mutex_lock(&accounts[to_id].lock);

	// If we get here, no deadlock occured this time.
	accounts[from_id].balance -= amount;
	accounts[to_id].balance += amount; 

	pthread_mutex_unlock(&accounts[to_id].lock);
	pthread_mutex_unlock(&accounts[from_id].lock);

}

void* teller_thread(void* arg) {
	int* ids = (int*) arg;
	transfer(ids[0], ids[1], 50);
	return NULL;
}	

int main() {
	//initialize accounts
	for(int i = 0; i < NUM_ACCOUNTS; i++) {
	   accounts[i].account_id = i;
	   accounts[i].balance = 1000;
	   pthread_mutex_init(&accounts[i].lock, NULL);
	}
	
	pthread_t t1,t2;
	int args1[2] = {0,1};
 	int args2[2] = {1,0};

	pthread_create(&t1, NULL, teller_thread, args1);
	pthread_create(&t2, NULL, teller_thread, args2);

	pthread_join(t1,NULL);	
	pthread_join(t2,NULL);
		
	printf("Final Balances: Account 0:  %2f, Account 1: %.2f\n",
		accounts[0].balance, accounts[1].balance);

	
return 0; 
}
