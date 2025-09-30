// Phase 3: Deadlock Creation
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 3
#define TRANSFERS_PER_TELLER 3
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock; 
} Account;

Account accounts[NUM_ACCOUNTS];

// Transfer function (deadlock-prone: locks in given order)
void transfer(int from_id, int to_id, double amount) {
    printf("Thread %ld: Attempting transfer from %d to %d\n",
           pthread_self(), from_id, to_id);

    pthread_mutex_lock(&accounts[from_id].lock);
    printf("Thread %ld: Locked account %d\n", pthread_self(), from_id);

    // small delay to make deadlock more likely
    usleep(100);

    printf("Thread %ld: Waiting for account %d\n", pthread_self(), to_id);
    pthread_mutex_lock(&accounts[to_id].lock);

    // Perform transfer
    if (accounts[from_id].balance >= amount) {
        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;
        accounts[from_id].transaction_count++;
        accounts[to_id].transaction_count++;
        printf("Thread %ld: Transfer %.2f from %d -> %d SUCCESS\n",
               pthread_self(), amount, from_id, to_id);
    } else {
        printf("Thread %ld: Transfer FAILED (Insufficient funds)\n",
               pthread_self());
    }

    pthread_mutex_unlock(&accounts[to_id].lock);
    pthread_mutex_unlock(&accounts[from_id].lock);
}

// Teller thread
void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;
    unsigned int seed = time(NULL) ^ teller_id;

    for (int i = 0; i < TRANSFERS_PER_TELLER; i++) {
        int from = rand_r(&seed) % NUM_ACCOUNTS;
        int to = (from + 1) % NUM_ACCOUNTS; // always transfer to "other" account
        transfer(from, to, 50.0);
        usleep(500); // allow interleaving
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
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    printf("=== Phase 3 (Deadlock Creation) ===\n");

    // Create teller threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }

    // Wait for threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final balances
    double total_actual = 0;
    printf("\nFinal Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f (Transactions: %d)\n",
               i, accounts[i].balance, accounts[i].transaction_count);
        total_actual += accounts[i].balance;
    }
    printf("Total money: %.2f\n", total_actual);

    // Destroy mutexes
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
