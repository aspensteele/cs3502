Producer–Consumer – Assignment 3

Author: Aspen Steele
Course: CS 3502 – Section 04

This project implements the classic Producer–Consumer problem using shared memory, named semaphores, and a circular buffer. Multiple producers and consumers safely share data without race conditions or deadlocks.

Files
buffer.h      # Shared structures
producer.c    # Producer process
consumer.c    # Consumer process

Compile
gcc producer.c -o producer -lpthread
gcc consumer.c -o consumer -lpthread

Run
Producer:
./producer <producer_id> <num_items>

Consumer:
./consumer <consumer_id> <num_items>


Example:

./producer 1 5 &
./consumer 1 5 &

How It Works

Shared memory stores a circular buffer

/sem_mutex protects the critical section

/sem_empty tracks free slots

/sem_full tracks filled slots

Producers: empty → mutex → produce → mutex → full

Consumers: full → mutex → consume → mutex → empty

Cleanup Commands
Command:
ipcrm -M 0x1234
rm /dev/shm/sem.*