	// Aspen Steele
	// CS 3502 - Section 04 - Assignment 3

	#ifndef BUFFER_H
	#define BUFFER_H

	#define BUFFER_SIZE 10
	#define SHM_KEY 0x1234

	typedef struct {
		int value; // Data value
		int producer_id; // Which producer created this
	} item_t;

	typedef struct {
		item_t buffer[BUFFER_SIZE];
		int head; // Next write position
		int tail; // Next read position
		int count; // Current items in buffer
	} shared_buffer_t;

	#endif