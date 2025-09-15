#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <signal.h> 

// Global variables for signal handlers
volatile sig_atomic_t shutdown_flag = 0;  // Set to 1 when SIGINT received
volatile sig_atomic_t stats_flag = 0;     // Set to 1 when SIGUSR1 received

// Statistics tracking
volatile sig_atomic_t lines_processed = 0;
volatile sig_atomic_t chars_processed = 0;
clock_t start_time; 

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    shutdown_flag = 1;  // Tell main loop to exit gracefully
}

// Signal handler for SIGUSR1 (kill -USR1 <pid>)
void handle_sigusr1(int sig) {
    stats_flag = 1;     // Tell main loop to display statistics
} 

int main(int argc, char *argv[]) {
    int max_lines = -1; // -1 means unlimited
    int verbose = 0; 
    char opt; 
    int line_count = 0; 
    int char_count = 0;
    char buffer[1024]; // buffer for reading lines
    
    // Parse arguments first (before setting up signals)
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
    
    struct sigaction sa;                 
    
    sa.sa_handler = handle_sigint;        
    sigemptyset(&sa.sa_mask);             
    sa.sa_flags = 0;                        
    sigaction(SIGINT, &sa, NULL);         
    
    sa.sa_handler = handle_sigusr1;        
    sigaction(SIGUSR1, &sa, NULL);        
    
    clock_t start = clock(); // Start timer for performance metrics
    

    while (fgets(buffer, sizeof(buffer), stdin) != NULL && !shutdown_flag) {
        line_count++;
        char_count += strlen(buffer);
        
        if(verbose) {
            printf("Line %d: %s", line_count, buffer);
        }
        
        // Check if user requested statistics (via kill -USR1 <pid>)
        if (stats_flag) {
            fprintf(stderr, "Current Stats: Lines=%d, Chars=%d\n",
                    line_count, char_count);
            stats_flag = 0; // Reset the flag so we don't keep showing stats
        }
        
        if (max_lines != -1 && line_count >= max_lines) {
            if (verbose) {
                fprintf(stderr, "Reached maximum lines limit (%d)\n", max_lines);
            }
            break;
        }
    }
    
    // After the loop - check if we exited because of signal
    if (shutdown_flag) {
        fprintf(stderr, "Shutdown signal received. Exiting gracefully...\n");
    }
    
    // End timer and calculate performance stats
    clock_t end = clock(); 
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;  
    
    // Print final statistics
    fprintf(stderr, "Lines: %d\n", line_count);
    fprintf(stderr, "Characters: %d\n", char_count);
    fprintf(stderr, "Processing time: %.2f seconds\n", cpu_time);
    
    if (cpu_time > 0) {
        double lines_per_sec = line_count / cpu_time;
        double bytes_per_sec = char_count / cpu_time;
        fprintf(stderr, "Throughput: %.2f lines/sec, %.2f bytes/sec\n",     
                lines_per_sec, bytes_per_sec);
    }
    
    return 0;
}
