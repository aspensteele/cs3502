#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

int main(int argc, char *argv[]) {
	int max_lines = -1; // -1 means uinlimited
	int verbose = 0; 
	char opt; 

	int line_count = 0; 
	int char_count = 0;
	char buffer[1024]; //buffer for reading lines

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


	while (fgets(buffer, sizeof(buffer), stdin) != 	NULL) {
	   line_count++;
	   
	   char_count += strlen(buffer);
	   
	   if(verbose) {
		printf("Line  %d: %s", line_count, buffer);
	   }

	  if (max_lines != -1 && line_count >= max_lines) {
		if (verbose) {
		fprintf(stderr, "Reached maximum lines limit (%d)\n", max_lines);
	}
	 break;
	}
    }

	fprintf(stderr, "Lines: %d\n", line_count);
	fprintf(stderr, "Characters: %d\n", char_count);

	return 0;
} 

