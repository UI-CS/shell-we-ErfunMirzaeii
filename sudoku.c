#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9
#define THREADS 11

//complete your sudoku board 
int sudoku[SIZE][SIZE] = {
    {5,3,4,6,7,8,9,1,2},
    {6,7,2,1,9,5,3,4,8},
    {1,9,8,3,4,2,5,6,7},
    {8,5,9,7,6,1,4,2,3},
    {4,2,6,8,5,3,7,9,1},
    {7,1,3,9,2,4,8,5,6},
    {9,6,1,5,3,7,2,8,4},
    {2,8,7,4,1,9,6,3,5},
    {3,4,5,2,8,6,1,7,9}
};

// Shared result array
int valid[THREADS];

// Parameters for subgrid threads 
typedef struct {
    int row;
    int col;
    int index;
} parameters;

// Thread 0: validate all rows 
void *validate_rows(void *param) {
    for (int i = 0; i < SIZE; i++) {
        int seen[10] = {0};
        for (int j = 0; j < SIZE; j++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || seen[num]) {
                valid[0] = 0;
                pthread_exit(NULL);
            }
            seen[num] = 1;
        }
    }
    valid[0] = 1;
    pthread_exit(NULL);
}

// Thread 1: validate all columns 
void *validate_columns(void *param) {
    for (int j = 0; j < SIZE; j++) {
        int seen[10] = {0};
        for (int i = 0; i < SIZE; i++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9 || seen[num]) {
                valid[1] = 0;
                pthread_exit(NULL);
            }
            seen[num] = 1;
        }
    }
    valid[1] = 1;
    pthread_exit(NULL);
}

// Threads 2–10: validate one 3x3 subgrid 
void *validate_subgrid(void *param) {
    parameters *data = (parameters *)param;
    int seen[10] = {0};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int num = sudoku[data->row + i][data->col + j];
            if (num < 1 || num > 9 || seen[num]) {
                valid[data->index] = 0;
                pthread_exit(NULL);
            }
            seen[num] = 1;
        }
    }

    valid[data->index] = 1;
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[THREADS];

    // Create row validator thread 
    pthread_create(&threads[0], NULL, validate_rows, NULL);

    // Create column validator thread 
    pthread_create(&threads[1], NULL, validate_columns, NULL);

    // Create 9 subgrid validator threads 
    int index = 2;
    for (int i = 0; i < SIZE; i += 3) {
        for (int j = 0; j < SIZE; j += 3) {
            parameters *data = malloc(sizeof(parameters));
            data->row = i;
            data->col = j;
            data->index = index;

            pthread_create(&threads[index], NULL, validate_subgrid, data);
            index++;
        }
    }

    // Wait for all threads 
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Final result 
    for (int i = 0; i < THREADS; i++) {
        if (valid[i] == 0) {
            printf("Sudoku is INVALID ❌\n");
            return 0;
        }
    }

    printf("Sudoku is VALID ✅\n");
    return 0;
}
