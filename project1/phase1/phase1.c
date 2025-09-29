#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 3
#define TRANSACTIONS_PER_TELLER 3
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
} Account;

Account accounts[NUM_ACCOUNTS];

void delay() {
    for (volatile int i = 0; i < 10000000; i++);
}

int is_valid_account(int account_id) {
    return (account_id >= 0 && account_id < NUM_ACCOUNTS);
}

void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;
    unsigned int seed = time(NULL) ^ teller_id;
    
    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Test edge cases: sometimes generate invalid accounts
        int acc;
        if (rand_r(&seed) % 5 == 0) {
            acc = NUM_ACCOUNTS + (rand_r(&seed) % 3);
        } else {
            acc = rand_r(&seed) % NUM_ACCOUNTS;
        }
        
        // Random amount: multiples of 50 (range: 50 to 200)
        int multiple = (rand_r(&seed) % 4) + 1; // 1 to 4
        double amount = multiple * 50.0;
        
        // Random operation: deposit or withdraw
        if (rand_r(&seed) % 2 == 0) {
            amount = -amount; // Withdrawal
        }
        
        if (!is_valid_account(acc)) {
            printf("Teller %d: ERROR - Account %d does not exist! ", teller_id, acc);
            printf("Transaction %+.2f skipped.\n", amount);
            continue;
        }
        
        double old_balance = accounts[acc].balance;
        
        delay();
        
        accounts[acc].balance = old_balance + amount;
        
        const char* type = (amount >= 0) ? "Deposit" : "Withdrawal";
        const char* warning = "";
        
        if (accounts[acc].balance < 0) {
            warning = " [OVERDRAFT - RACE CONDITION!]";
        }
        
        printf("Teller %d: %s %+.2f to Account %d (before=%.2f, after=%.2f)%s\n",
               teller_id, type, amount, acc, old_balance, accounts[acc].balance, warning);
        usleep(1000);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
    }
    
    printf("=== Phase 1 ===\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double total_actual = 0;
    printf("\nFinal Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: %.2f\n", i, accounts[i].balance);
        total_actual += accounts[i].balance;
    }
    
    printf("\nTotal money: %.2f\n", total_actual);
    
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        if (accounts[i].balance < 0) {
            printf("Edge Case Result: Account %d has negative balance due to race conditions!\n", i);
        }
    }
    
    return 0;
}
