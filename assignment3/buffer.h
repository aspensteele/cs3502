// ============================================
// buffer.h - Shared definitions
// Aspen Steele
// CS 3502 - Section 04 - Assignment 3
// ============================================
#ifndef BUFFER_H
#define BUFFER_H

// Required includes for both producer and consumer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// Constants for shared memory and semaphores
#define BUFFER_SIZE 10
#define SHM_KEY 0x1234

// Semaphore names
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL  "/sem_full"


// Item structure
typedef struct {
    int value;        // The data value
    int producer_id;  // ID of the producer that created this item
    time_t timestamp; // OPTIONAL but helpful: time item was produced
} item_t;


// Shared buffer structure
typedef struct {
    item_t buffer[BUFFER_SIZE];  // Circular buffer array
    int head;                    // Next write position
    int tail;                    // Next read position
    int count;                   // Current number of items in buffer
} shared_buffer_t;

#endif
