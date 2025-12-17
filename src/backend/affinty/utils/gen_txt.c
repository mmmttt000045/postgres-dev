#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Fisher-Yates shuffle algorithm
void shuffle(int *array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // Swap array[i] and array[j]
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int main() {
    int n = 10000;
    int *numbers = (int *)malloc(n * sizeof(int));

    if (numbers == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Initialize array with numbers 1 to 10000
    for (int i = 0; i < n; i++) {
        numbers[i] = i + 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Shuffle the array
    shuffle(numbers, n);

    // Open file for writing
    FILE *fp = fopen("test.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file for writing\n");
        free(numbers);
        return 1;
    }

    // Write shuffled numbers and random 0-3 values to file
    for (int i = 0; i < n; i++) {
        int random_0_to_3 = rand() % 4;
        fprintf(fp, "%d\t%d\n", numbers[i], random_0_to_3);
    }

    fclose(fp);
    free(numbers);

    printf("Successfully generated output.txt with %d lines\n", n);

    return 0;
}

