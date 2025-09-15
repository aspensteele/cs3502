#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>

// global variables for signal handlers
volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t stats_flag = 0;

// signal handlers
void handle_sigint(int sig) {
    shutdown_flag = 1;
}

void handle_sigusr1(int sig) {
    stats_flag = 1;
}

int main(int argc, char *argv[]) {
    int max_lines = -1; // -1 means unlimited
    int verbose = 0;
    int opt;

    int line_count = 0;
    int char_count = 0;
    char buffer[1024]; // buffer for reading lines

    // Parse arguments
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

    // register SIGINT handler
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // register SIGUSR1 handler
    struct sigaction sa2;
    sa2.sa_handler = handle_sigusr1;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    sigaction(SIGUSR1, &sa2, NULL);

    // start timer for performance metrics
    clock_t start = clock();

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        line_count++;
        char_count += strlen(buffer);

        if (verbose) {
            printf("Line %d: %s", line_count, buffer);
        }

        // if SIGUSR1 received, print stats
        if (stats_flag) {
            fprintf(stderr, ">>> Current Stats: Lines=%d, Chars=%d\n",
                    line_count, char_count);
            stats_flag = 0; // reset
        }

        // If SIGINT received, shutdown gracefully
        if (shutdown_flag) {
            fprintf(stderr, ">>> Shutdown signal received. Exiting...\n");
            break;
        }

        // If max_lines reached
        if (max_lines != -1 && line_count >= max_lines) {
            if (verbose) {
                fprintf(stderr, "Reached maximum lines limit (%d)\n", max_lines);
            }
            break;
        }
    }

    // end timer and performance stats
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    fprintf(stderr, "Lines: %d\n", line_count);
    fprintf(stderr, "Characters: %d\n", char_count);
    fprintf(stderr, "Time elapsed: %.2f sec\n", cpu_time);

    if (cpu_time > 0) {
        double lines_per_sec = line_count / cpu_time;
        double bytes_per_sec = char_count / cpu_time;
        fprintf(stderr, "Throughput: %.2f lines/sec, %.2f bytes/sec\n",
                lines_per_sec, bytes_per_sec);
    }

    return 0;
}

