#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CODE_SIZE 4
#define MAX_ATTEMPTS 8
#define NUM_COLORS 6

// Function prototypes
void display_comb(int comb[]);
void read_comb(int arr[]);
void random_comb(int comb[]);
int well_placed(int comb[], int propos[]);
int present(int comb[], int number);
void check_result(int comb[], int propos[], char result[]);
void init_grid(char *grid[]);
void free_grid(char *grid[]);
void display_grid(char *grid[]);
void update_grid(char *grid[], int attempt, int propos[], char result[]);

// Display combination
void display_comb(int comb[]) {
    for (int i = 0; i < CODE_SIZE; i++) {
        printf("%d ", comb[i]);
    }
    printf("\n");
}

// Read user input combination
void read_comb(int arr[]) {
    for (int i = 0; i < CODE_SIZE; i++) {
        scanf("%d", &arr[i]);
    }
}

// Generate random combination
void random_comb(int comb[]) {
    srand(time(NULL));
    for (int i = 0; i < CODE_SIZE; i++) {
        comb[i] = (rand() % NUM_COLORS) + 1;
    }
}

// Count well-placed numbers
int well_placed(int comb[], int propos[]) {
    int count = 0;
    for (int i = 0; i < CODE_SIZE; i++) {
        if (comb[i] == propos[i]) {
            count++;
        }
    }
    return count;
}

// Check if a number is present
int present(int comb[], int number) {
    for (int i = 0; i < CODE_SIZE; i++) {
        if (comb[i] == number) {
            return 1;
        }
    }
    return 0;
}

// Generate result feedback
void check_result(int comb[], int propos[], char result[]) {
    for (int i = 0; i < CODE_SIZE; i++) {
        if (comb[i] == propos[i]) {
            result[i] = 'o';
        } else if (present(comb, propos[i])) {
            result[i] = 'x';
        } else {
            result[i] = '-';
        }
    }
}

// Initialize grid
void init_grid(char *grid[]) {
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        grid[i] = malloc(20 * sizeof(char));
        sprintf(grid[i], "- - - - | - - - -");
    }
}

// Free grid memory
void free_grid(char *grid[]) {
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        free(grid[i]);
    }
}

// Display the grid
void display_grid(char *grid[]) {
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        printf("%s\n", grid[i]);
    }
}

// Update grid with new attempt
void update_grid(char *grid[], int attempt, int propos[], char result[]) {
    sprintf(grid[attempt], "%d %d %d %d | %c %c %c %c",
            propos[0], propos[1], propos[2], propos[3],
            result[0], result[1], result[2], result[3]);
}

// Main function
int main() {
    int comb[CODE_SIZE];
    int propos[CODE_SIZE];
    char result[CODE_SIZE];
    char *grid[MAX_ATTEMPTS];
    int attempts = 0;1

    random_comb(comb);
    init_grid(grid);

    printf("Welcome to Mastermind!\n");
    printf("You have to guess a combination of 4 numbers between 1 and 6\n");
    printf("You have %d attempts to find the right combination.\n", MAX_ATTEMPTS);

    while (attempts < MAX_ATTEMPTS) {
        printf("Enter your guess: ");
        read_comb(propos);

        check_result(comb, propos, result);
        update_grid(grid, attempts, propos, result);
        display_grid(grid);

        if (well_placed(comb, propos) == CODE_SIZE) {
            printf("Congratulations! You found the combination.\n");
            break;
        }
        attempts++;
    }

    if (attempts == MAX_ATTEMPTS) {
        printf("Game over! The correct combination was: ");
        display_comb(comb);
    }

    free_grid(grid);
    return 0;
}
