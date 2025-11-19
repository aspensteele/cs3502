// ============================================
// consumer.c - Consumer process
// Aspen Steele
// CS 3502 - Section 04 - Assignment 3
// ============================================
#include "buffer.h"
#include <stddef.h>

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

// Catch Ctrl+C or termination
void signal_handler(int sig) {
    printf("\nConsumer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {

    // Validate arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
        exit(1);
    }

    int consumer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);

        srand(time(0) + consumer_id); // Seed random number generator

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

        for (int i = 0; i < num_items ; i ++) {
            sem_wait ( full ) ; // Wait for item
            sem_wait ( mutex ) ; // Enter critical section

            // Remove from buffer
            item_t item = buffer->buffer[buffer->tail];
            buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
            buffer->count--;

            printf(" Consumer %d: Consumed value %d from Producer %d\n",
            consumer_id , item . value , item . producer_id ) ;

            sem_post ( mutex ) ; // Exit critical section
            sem_post ( empty ) ; // Signal slot available

            usleep ( rand () % 100000) ; // Simulate work
    }

        // Close semaphores
        sem_close ( mutex ) ;
        sem_close ( empty ) ;
        sem_close ( full ) ;

        // Detach shared memory
        shmdt ( buffer ) ;

        return 0;
    }