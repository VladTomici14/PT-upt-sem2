#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_TITLE_LENGTH 256

// Structure for a movie node in the linked list
typedef struct MovieNode {
    int year;
    char* title;
    double budget;
    struct MovieNode* next;
} MovieNode;

// Function to trim whitespace from a string
void trim(char* str) {
    if (str == NULL) return;

    // Trim leading whitespace
    char* start = str;
    while (isspace((unsigned char)*start)) start++;

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // Trim trailing whitespace
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

// Function to find the index of a column in the header
int find_column_index(char* header_line, const char* column_name) {
    char* token;
    char* rest = header_line;
    int index = 0;

    while ((token = strtok_r(rest, ",", &rest)) != NULL) {
        trim(token);
        if (strcasecmp(token, column_name) == 0) {
            return index;
        }
        index++;
    }

    return -1; // Column not found
}

// Function to create a new movie node
MovieNode* create_movie_node(int year, const char* title, double budget) {
    MovieNode* new_node = (MovieNode*)malloc(sizeof(MovieNode));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    new_node->year = year;
    new_node->title = strdup(title);
    if (new_node->title == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    new_node->budget = budget;
    new_node->next = NULL;

    return new_node;
}

// Function to insert a movie node into the sorted linked list
void insert_sorted(MovieNode** head, MovieNode* new_node) {
    MovieNode* current;

    // If list is empty or the new node should be the head
    if (*head == NULL || (*head)->year > new_node->year ||
        ((*head)->year == new_node->year && strcmp((*head)->title, new_node->title) > 0)) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    // Find the position to insert
    current = *head;
    while (current->next != NULL &&
           (current->next->year < new_node->year ||
            (current->next->year == new_node->year && strcmp(current->next->title, new_node->title) < 0))) {
        current = current->next;
    }

    // Insert the new node
    new_node->next = current->next;
    current->next = new_node;
}

// Function to free all allocated memory
void free_list(MovieNode* head) {
    MovieNode* current = head;
    MovieNode* next;

    while (current != NULL) {
        next = current->next;
        free(current->title);
        free(current);
        current = next;
    }
}

// Function to parse the CSV file and build the linked list
MovieNode* parse_csv_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    char header_copy[MAX_LINE_LENGTH];
    MovieNode* head = NULL;

    // Read the header line
    if (fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Empty file or error reading header\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Remove newline character if present
    size_t len = strlen(line);
    if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
        line[len-1] = '\0';
    }

    // Make a copy of the header for tokenization
    strcpy(header_copy, line);

    // Find the indices of the columns we need
    int year_index = find_column_index(header_copy, "year");
    strcpy(header_copy, line); // Reset for the next search
    int title_index = find_column_index(header_copy, "title");
    strcpy(header_copy, line);
    int budget_index = find_column_index(header_copy, "budget");

    if (year_index == -1 || title_index == -1 || budget_index == -1) {
        fprintf(stderr, "Required columns not found in CSV header\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Read the data lines
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove newline character if present
        len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
        }

        char* token;
        char* rest = line;
        int column_index = 0;
        int year = 0;
        char title[MAX_TITLE_LENGTH] = "";
        double budget = 0.0;

        // Parse the line
        while ((token = strtok_r(rest, ",", &rest)) != NULL) {
            trim(token);

            if (column_index == year_index) {
                year = atoi(token);
            } else if (column_index == title_index) {
                // Check if this is a quoted field (which may contain commas)
                if (token[0] == '"' && token[strlen(token)-1] != '"') {
                    // We have an opening quote but no closing quote - the field contains commas
                    char quoted_field[MAX_TITLE_LENGTH] = "";
                    strncpy(quoted_field, token + 1, MAX_TITLE_LENGTH - 1); // Skip the opening quote
                    strcat(quoted_field, ",");

                    // Keep reading tokens until we find the closing quote
                    while ((token = strtok_r(rest, ",", &rest)) != NULL) {
                        column_index++; // Important to increment for each additional token consumed

                        size_t token_len = strlen(token);
                        if (token[token_len-1] == '"') {
                            // We found the closing quote
                            token[token_len-1] = '\0'; // Remove the closing quote
                            strncat(quoted_field, token, MAX_TITLE_LENGTH - strlen(quoted_field) - 1);
                            strncpy(title, quoted_field, MAX_TITLE_LENGTH - 1);
                            break;
                        } else {
                            // Not the end yet, append with comma
                            strncat(quoted_field, token, MAX_TITLE_LENGTH - strlen(quoted_field) - 1);
                            strcat(quoted_field, ",");
                        }
                    }
                } else {
                    // Regular field without commas or properly quoted
                    if (token[0] == '"' && token[strlen(token)-1] == '"') {
                        // Remove quotes if present
                        token[strlen(token)-1] = '\0'; // Remove closing quote
                        strncpy(title, token + 1, MAX_TITLE_LENGTH - 1); // Skip opening quote
                    } else {
                        strncpy(title, token, MAX_TITLE_LENGTH - 1);
                    }
                }
                title[MAX_TITLE_LENGTH - 1] = '\0';
            } else if (column_index == budget_index) {
                budget = atof(token);
            }

            column_index++;
        }

        // Create and insert a new node if we have valid data
        if (year > 0 && strlen(title) > 0) {
            MovieNode* new_node = create_movie_node(year, title, budget);
            insert_sorted(&head, new_node);
        }
    }

    fclose(file);
    return head;
}

// Function to print the linked list
void print_list(const MovieNode* head) {
    const MovieNode* current = head;
    printf("Year    Title                                                Budget\n");
    printf("----------------------------------------------------------------------\n");

    while (current != NULL) {
        // Format budget with comma separators for thousands
        char budget_str[32];
        if (current->budget >= 1000000) {
            sprintf(budget_str, "$%.2f million", current->budget / 1000000.0);
        } else if (current->budget > 0) {
            sprintf(budget_str, "$%.2f", current->budget);
        } else {
            strcpy(budget_str, "N/A");
        }

        printf("%-7d %-50.50s %s\n",
               current->year,
               current->title,
               budget_str);
        current = current->next;
    }
}

int main() {

    MovieNode* movie_list = parse_csv_file("inputData/movies.csv");

    // Print the sorted list
    print_list(movie_list);

    // Free all allocated memory
    free_list(movie_list);

    return EXIT_SUCCESS;
}