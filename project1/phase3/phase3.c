//testing for  github
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int account_id;
    double balance;
    pthread_mutex_t lock;
} Account;

Account accounts[2]; // Two accounts for deadlock demo

typedef struct {
    int from_id;
    int to_id;
    double amount;
} TransferArgs;

void* transfer_thread(void* arg) {
    TransferArgs* t = (TransferArgs*)arg;
    printf("Thread %ld: Attempting transfer from %d to %d\n",
           pthread_self(), t->from_id, t->to_id);

    pthread_mutex_lock(&accounts[t->from_id].lock);
    printf("Thread %ld: Locked account %d\n", pthread_self(), t->from_id);

    usleep(100); // Simulate processing delay

    printf("Thread %ld: Waiting for account %d\n", pthread_self(), t->to_id);
    pthread_mutex_lock(&accounts[t->to_id].lock);

    // Perform transfer
    accounts[t->from_id].balance -= t->amount;
    accounts[t->to_id].balance += t->amount;

    pthread_mutex_unlock(&accounts[t->to_id].lock);
    pthread_mutex_unlock(&accounts[t->from_id].lock);

    printf("Thread %ld: Transfer complete from %d to %d\n",
           pthread_self(), t->from_id, t->to_id);

    return NULL;
}

int main() {
    pthread_t t1, t2;

    // Initialize accounts
    for (int i = 0; i < 2; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = 1000.0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    // Thread 1: transfer 100 from account 0 -> 1
    TransferArgs args1 = {0, 1, 100};
    // Thread 2: transfer 200 from account 1 -> 0
    TransferArgs args2 = {1, 0, 200};

    pthread_create(&t1, NULL, transfer_thread, &args1);
    pthread_create(&t2, NULL, transfer_thread, &args2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\nFinal balances:\n");
    for (int i = 0; i < 2; i++) {
        printf("Account %d: %.2f\n", i, accounts[i].balance);
    }

    // Cleanup mutexes
    for (int i = 0; i < 2; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
