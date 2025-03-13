// [] opening and reading the weather from the CSV
// [] parsing the headers columns
// [] extracting values for each column
// []

#include <stdio.h>
#include <string.h>

int main() {
    FILE *file;
    char buffer[1024];

    // opening the csv file
    file = fopen("inputData/Timisoara.csv", "r");

    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // reading and processing the file line by line
    while (fgets(buffer, sizeof(buffer), file)) {
        // removing newline character if present
        buffer[strcspn(buffer, "\n")] = 0;

        // processing the CSV line
        char *token = strtok(buffer, ",");
        while (token != NULL) {
            printf("%s\n", token);
            token = strtok(NULL, ",");
        }
    }

    fclose(file);
    return 0;
}