// ============================================
// producer.c - Producer process
// Aspen Steele
// CS 3502 - Section 04 - Assignment 3
// ============================================
#include "buffer.h"
#include <stddef.h>   // ensures NULL is defined
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>

// Global variables for cleanup
shared_buffer_t* buffer = NULL;
sem_t* mutex = NULL;
sem_t* empty = NULL;
sem_t* full = NULL;
int shm_id = -1;

// Cleanup function
void cleanup() {
    if (buffer != NULL) {
        shmdt(buffer);
    }
    if (mutex != SEM_FAILED && mutex != NULL) sem_close(mutex);
    if (empty != SEM_FAILED && empty != NULL) sem_close(empty);
    if (full != SEM_FAILED && full != NULL) sem_close(full);
}

// Signal handler
void signal_handler(int sig) {
    printf("\nProducer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        exit(1);
    }

    int producer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);

        srand(time(0) + producer_id); // Seed random number generator

        //shared memory setup
        int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);

        if ( shm_id < 0) {
            perror ("shmget - failed ") ;
            exit (1) ;
            }

        shared_buffer_t * buffer = ( shared_buffer_t *) shmat ( shm_id , 0 , 0) ;
        if ( buffer == ( void *) -1) {
        perror ("shmat - failed ") ;
        exit (1) ;
        }

        // Create or open named semaphores
        sem_t * mutex = sem_open ("/sem_mutex", O_CREAT , 0644 , 1) ;
        sem_t * empty = sem_open ("/sem_empty", O_CREAT , 0644 , BUFFER_SIZE ) ;
        sem_t * full = sem_open ("/sem_full", O_CREAT , 0644 , 0) ;

        if ( mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED ) {
        perror (" sem_open - failed ") ;
        exit (1) ;
        }

        for (int i = 0; i < num_items; i++) {
            item_t item;
            item.value = producer_id * 1000 + i;
            item.producer_id = producer_id;

            sem_wait(empty); // Wait for empty slot
            sem_wait(mutex); // Enter critical section

            // Add item to buffer
            buffer->buffer[buffer->head] = item;
            buffer->head = (buffer->head + 1) % BUFFER_SIZE; // Circular increment
            buffer->count++;

        printf("Producer %d: Produced value %d\n", producer_id, item.value);

            sem_post(mutex); // Exit critical section
            sem_post(full);  // Signal that a new item is available

            // Simulate variable production time
            usleep(rand() % 10000); // simulate work 

        }

        // Cleanup (shared memory segments stay allocated in the system even after program ends, takes up system resources)
        shmdt(buffer);
        sem_close(mutex);
        sem_close(empty);
        sem_close(full);

        return 0;
    }