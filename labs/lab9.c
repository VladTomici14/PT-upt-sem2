#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUM_FUNCTIONS 7

// ----- array of function pointers -----
double (*functions[NUM_FUNCTIONS])(double) = {
        sin,
        cos,
        ceil,
        floor,
        fabs,
        log,
        sqrt
};

// ----- array of function names  -----
char *function_names[NUM_FUNCTIONS] = {
        "sin",
        "cos",
        "ceil",
        "floor",
        "fabs",
        "log",
        "sqrt"
};

void table() {
    // ----- printing the header -----
    printf("num\t\t");
    for (int i = 0; i < NUM_FUNCTIONS; i++) {
        printf("%s\t\t", function_names[i]);
    }

    // ----- printing the table content -----
    for (double i = 3.14; i >= -3.14; i -= 0.05) {
        printf("\n");
        printf("%.2f\t\t", i);
        for (int j = 0; j < NUM_FUNCTIONS; j++) {
            double result = functions[j](i);
            printf("%.2f\t\t", result);
        }
    }

}

int main() {

    int choice;
    double number, result;

    table();

    while (1) {
        // ----- display menu -----
        printf("\n=== Mathematical Function Calculator ===\n");
        printf("Select a function to apply:\n");
        for (int i = 0; i < NUM_FUNCTIONS; i++) {
            printf("%d. %s()\n", i + 1, function_names[i]);
        }
        printf("0. Exit\n");
        printf("Enter your choice (0-%d): ", NUM_FUNCTIONS);

        // ----- reading the user choice -----
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        // ----- exiting condition -----
        if (choice == 0) {
            printf("Goodbye!\n");
            break;
        }

        // ----- validating choice -----
        if (choice < 1 || choice > NUM_FUNCTIONS) {
            printf("Invalid choice! Please select a number between 1 and %d.\n", NUM_FUNCTIONS);
            continue;
        }

        // ----- reading the number to apply function to -----
        printf("Enter a number: ");
        if (scanf("%lf", &number) != 1) {
            printf("Invalid input! Please enter a valid number.\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue;
        }

        // ----- special validation for specific functions -----
        if (choice == 6 && number <= 0) { // log function
            printf("Error: log() requires a positive number.\n");
            continue;
        }

        if (choice == 7 && number < 0) { // sqrt function
            printf("Error: sqrt() requires a non-negative number.\n");
            continue;
        }

        // ----- calling the selected function using function pointer -----
        result = functions[choice - 1](number);

        // ----- displaying the result -----
        printf("Result: %s(%.6f) = %.6f\n",
               function_names[choice - 1], number, result);
    }

    return 0;
}