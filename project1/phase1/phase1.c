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

// Predefined transactions: + = deposit, - = withdraw
double transactions[NUM_THREADS][TRANSACTIONS_PER_TELLER] = {
    {100, -50, 200},   // Teller 0
    {50, 50, -100},    // Teller 1  // FIXED: added missing comma
    {100, 50, -200}    // Teller 2  // FIXED: changed from Teller 3 to 2
};

void delay() {
    for (volatile int i = 0; i < 1000000; i++);
}

// Edge case validation function
int is_valid_account(int account_id) {
    return (account_id >= 0 && account_id < NUM_ACCOUNTS);
}

void* teller_thread(void* arg) {
    int teller_id = *(int*)arg;
    unsigned int seed = time(NULL) ^ teller_id;

    for (int i = 0; i < TRANSACTIONS_PER_TELLER; i++) {
        // Test edge cases: sometimes generate invalid accounts
        int acc;
        if (rand_r(&seed) % 5 == 0) { // 20% chance of invalid account
            acc = NUM_ACCOUNTS + (rand_r(&seed) % 3); // Invalid account 2, 3, or 4
        } else {
            acc = rand_r(&seed) % NUM_ACCOUNTS; // Valid account 0 or 1
        }
        
        double amount = transactions[teller_id][i];

        // Edge case handling for non-existent accounts
        if (!is_valid_account(acc)) {
            printf("Teller %d: ERROR - Account %d does not exist! Transaction %+.2f skipped.\n",
                   teller_id, acc, amount);
            continue;
        }

        // Edge case handling for potential negative balances
        double old_balance = accounts[acc].balance;
        double new_balance = old_balance + amount;
        
        // AMPLIFY THE RACE WINDOW
        delay();
        
        accounts[acc].balance = new_balance;

        // Add warning for negative amounts and negative balances
        const char* warning = "";
        if (amount < 0) warning = " [NEGATIVE AMOUNT]";
        if (new_balance < 0) warning = " [NEGATIVE BALANCE!]";
        
        printf("Teller %d: %+.2f to Account %d (before=%.2f, after=%.2f)%s\n",
               teller_id, amount, acc, old_balance, accounts[acc].balance, warning);

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

    double total_expected = NUM_ACCOUNTS * INITIAL_BALANCE;
    for (int t = 0; t < NUM_THREADS; t++) {
        for (int j = 0; j < TRANSACTIONS_PER_TELLER; j++) {
            total_expected += transactions[t][j];
        }
    }

    printf("=== Starting Phase 1 (Race Condition Demo with Edge Cases) ===\n");
    printf("Testing: Negative amounts, non-existent accounts, race conditions\n\n");

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

    printf("\nTotal money: %.2f (should be %.2f)\n", total_actual, total_expected);
    printf("Race condition detected: %s\n", 
           total_actual != total_expected ? "YES" : "NO");

    // Edge case summary
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        if (accounts[i].balance < 0) {
            printf("Edge Case Result: Account %d has negative balance due to race conditions!\n", i);
        }
    }

    return 0;
}
