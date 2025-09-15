#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

//global variables for signal handlers
volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t stats_flag = 0;

//statistics tracking
volatile sig_atomic_t lines_processed = 0;
volatile sig_atomic_t chars_processed = 0;
clock_t start_time; 

void handle_sigint(int sig) {
	shutdown_flag = 1;
}

void handle_sigusr1(int sig) {
	stats_flag = 1;
} 

int main(int argc, char *argv[]) {
	int max_lines = -1; // -1 means unlimited
	int verbose = 0; 
	char opt; 

	int line_count = 0; 
	int char_count = 0;
	char buffer[1024]; //buffer for reading lines

	// Set yp signal handlers
	struct sigaction sa_int, sa_usr1; 

		
	//Parse arguments
	while ((opt = getopt(argc, argv, "n:v")) != -1) {
	    switch (opt) {
		case 'n': 
		max_lines = atoi(optarg);
		if (max_lines <= 0) {
		 fprintf(stderr, "Error: Max lines must be positive\n");
		return 1;
		}
		break;
		case 'v': 
		verbose = 1; 	
		break;
		default:
			fprintf(stderr, "Usage: %s [-n max_lines] [-v]\n", argv[0]);
		return 1;
		}
	}
	
	//register signal handlers
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);	

	clock_t start = clock(); // start timer for performance metrics

	while (fgets(buffer, sizeof(buffer), stdin) != 	NULL) {
	   line_count++;
	   
	   char_count += strlen(buffer);
	   
	   if(verbose) {
		printf("Line  %d: %s", line_count, buffer);
	   }
	   // if sigusr1 received, print stats
	  if (stats_flag) {
		fprintf(stderr, " Current Stats: Lines=%d, Chars=%d\n",
				line_count, char_count);
		stats_flag = 0; //reset
	    }
	  // If SIGINT received, shutdown gracefully
	  if (shutdown_flag) {
	 fprintf(stderr,"Shutdown signal received. Exiting...\n");
	}
	  if (max_lines != -1 && line_count >= max_lines) {
		if (verbose) {
		fprintf(stderr, "Reached maximum lines limit (%d)\n", max_lines);
	}
	 break;
	}
    }


	//end timer and performance stats
	clock_t end = clock(); 
	double cpu_time = ((double)(end - start)) // CLOCKS_PER_SEC;

	fprintf(stderr, "Lines: %d\n", line_count);
	fprintf(stderr, "Characters: %d\n", char_count);
	
	if (cpu_time > 0) {
	double lines_per_sec = line_count / cpu_time;
	double bytes_per_sec = char_count / cpu_time;
	fprintf(stderr, "Throughout: %.2f lines/sec, 5.2f bytes/sec\n",	
	 	lines_per_sec, bytes_per_sec);
	}
	
	return 0;
} 

