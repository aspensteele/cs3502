#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
	FILE *input = stdin; 
	int buffer_size = 4096;
	int opt; 
	int verbose = 0; 
	char *filename = NULL;
	char *buffer = NULL;
	size_t bytes_read;
	// Parse command line arguments
	while ((opt = getopt(argc, argv, "f:b:vh")) != -1) {
		switch  (opt) {
		    case 'f':
			 filename = optarg;
			 break;
		    case 'b':	 
			buffer_size = atoi(optarg);
			if (buffer_size <= 0) {
				fprintf(stderr, "Error: Buffer size must be positive\n");
				return 1;
			}
		  	break;
		    case 'v': 
			verbose = 1; //Flag with no argument
			break;
                    case 'h':
		    default:
			printf("Usage:  %s [-f file] [-b size] [-v]\n", argv[0]);
			exit(1);
		}
	}
	// Open file if -f provided
	if (filename != NULL) {
	input = fopen(filename, "r");
	if (input == NULL) {
	  	perror("Error opening file"); 
		return 1;
		}
	}

		if (verbose) {
	    if (filename != NULL) {
	        fprintf(stderr, "Producer: Reading from file '%s' with buffer size %d\n", 
	                filename, buffer_size);
	    } else {
	        fprintf(stderr, "Producer: Reading from stdin with buffer size %d\n", buffer_size);
	    }
	}
		
	buffer = malloc(buffer_size); 
	if (buffer == NULL) {
	   fprintf(stderr, "Error: Failed to allocate memory for buffer\n");
	if (input != stdin) {
	   fclose(input);
	}
	return 1;
     }
	
	int chunk_count = 0;  
	int total_bytes = 0; 
	
	while((bytes_read = fread(buffer, 1, buffer_size, input)) > 0) {
		chunk_count++;          
    	total_bytes += bytes_read;
	   if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {
	       fprintf(stderr, "Error: Failed to write to stdout\n");
	       break;
		}
		 if (verbose) {
        fprintf(stderr, "Producer: Chunk %d - read %zu bytes (total: %d bytes)\n",
                chunk_count, bytes_read, total_bytes);
    	}
     }

    if (ferror(input)) {
	perror("Error reading input");
    }

    free(buffer);
    if (input != stdin) {
	fclose(input);
    }
		
	return 0; 
}




