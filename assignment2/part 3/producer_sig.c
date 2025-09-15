#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

// Global flags
volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t stats_flag = 0;

// Counters
volatile sig_atomic_t total_bytes = 0;
volatile sig_atomic_t total_chunks = 0;

// Handlers
void handle_sigint(int sig) {
    shutdown_flag = 1; // shutdown
}

void handle_sigusr1(int sig) {
    stats_flag = 1; // request stats
}

int main(int argc, char *argv[]) {
    FILE *input = stdin; 
    int buffer_size = 4096;
    int verbose = 0; 
    char *filename = NULL;
    char *buffer = NULL;
    size_t bytes_read;
    int opt;

    // Parse args
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
                verbose = 1;
                break;
            case 'h':
            default:
                printf("Usage:  %s [-f file] [-b size] [-v]\n", argv[0]);
                exit(1);
        }
    }

    // Open file if provided
    if (filename != NULL) {
        input = fopen(filename, "r");
        if (input == NULL) {
            perror("Error opening file"); 
            return 1;
        }
    }

    buffer = malloc(buffer_size); 
    if (!buffer) {
        fprintf(stderr, "Error: Failed to allocate buffer\n");
        if (input != stdin) fclose(input);
        return 1;
    }

    // Register signals
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = handle_sigusr1;
    sigaction(SIGUSR1, &sa, NULL);

    // Start timer (CPU time)
    clock_t start = clock();

    // Main loop
    while (!shutdown_flag && (bytes_read = fread(buffer, 1, buffer_size, input)) > 0) {
        total_bytes += bytes_read;
        total_chunks++;

        if (fwrite(buffer, 1, bytes_read, stdout) != bytes_read) {
            fprintf(stderr, "Error writing to stdout\n");
            break;
        }

        if (verbose) {
            fprintf(stderr, "Chunk %d: %zu bytes written (total %d)\n",
                    total_chunks, bytes_read, total_bytes);
        }

        if (stats_flag) {
            fprintf(stderr, "Producer Stats: Chunks=%d, Bytes=%d\n",
                    total_chunks, total_bytes);
            stats_flag = 0;
        }
    }

    if (shutdown_flag) {
        fprintf(stderr, "Shutdown signal received. Stopping producer...\n");
    }

    if (ferror(input)) perror("Error reading input");

    // End timer
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    free(buffer);
    if (input != stdin) fclose(input);

    // Final stats
    fprintf(stderr, "Total Chunks: %d\n", total_chunks);
    fprintf(stderr, "Total Bytes: %d\n", total_bytes);
    fprintf(stderr, "Processing time: %.2f sec\n", cpu_time);

    if (cpu_time > 0) {
        double bytes_per_sec = total_bytes / cpu_time;
        double mb_per_sec = (total_bytes / 1024.0 / 1024.0) / cpu_time;
        fprintf(stderr, "Throughput: %.2f bytes/sec, %.2f MB/sec\n",
                bytes_per_sec, mb_per_sec);
    }

    return 0; 
}
