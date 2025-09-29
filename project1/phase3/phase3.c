//Phase 3: Deadlock Creation
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h> 
#include <errno.h>

typedef struct {
	int account_id;
	double balance;
	int transaction_count;
	pthread_mutex_t lock; // mutex for thus account
} Account; 



void transer(int from_id, int to_id, double amount) {
	printf("Thread %ld: Attempting transfer from %d to %d\n",
	      pthread_self()m, from_id, to_id); 

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
	pthread_mutex_unlock(&accounts[from_id].lock)ll

}

int main() {
	
	transfer(1,2,50)
	transfer(2,1,50)
	
	
return 0; 
}
