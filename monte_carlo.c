#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_processes> <points_per_process>\n", argv[0]);
        return 1;
    }

    int num_processes = atoi(argv[1]);
    long long points_per_process = atoll(argv[2]);

    // Shared memory for counts 
    long long *counts = mmap(NULL,
                             num_processes * sizeof(long long),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS,
                             -1, 0);

    if (counts == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Child process 
            unsigned int seed = time(NULL) ^ getpid();
            long long inside = 0;

            for (long long j = 0; j < points_per_process; j++) {
                double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
                double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

                if (x * x + y * y <= 1.0)
                    inside++;
            }

            counts[i] = inside;
            exit(0);
        }
        else if (pid < 0) {
            perror("fork failed");
            return 1;
        }
    }

    // Parent waits for all children 
    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    
    long long total_inside = 0;
    for (int i = 0; i < num_processes; i++) {
        total_inside += counts[i];
    }

    long long total_points = num_processes * points_per_process;
    double pi = 4.0 * (double)total_inside / (double)total_points;

    printf("Estimated Pi = %.6f\n", pi);

    munmap(counts, num_processes * sizeof(long long));
    return 0;
}
