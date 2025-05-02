#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_TITLE_LENGTH 256

// -----
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

// ------------------------------
// ----- SEARCHING FUNCTION -----
// ------------------------------

// Function to search movies by year
void search_by_year(const MovieNode* head, int year) {
    const MovieNode* current = head;
    int found = 0;

    printf("\nMovies released in %d:\n", year);
    printf("----------------------------------------------------------------------\n");

    while (current != NULL) {
        if (current->year == year) {
            char budget_str[32];
            if (current->budget >= 1000000) {
                sprintf(budget_str, "$%.2f million", current->budget / 1000000.0);
            } else if (current->budget > 0) {
                sprintf(budget_str, "$%.2f", current->budget);
            } else {
                strcpy(budget_str, "N/A");
            }

            printf("%-50.50s %s\n", current->title, budget_str);
            found = 1;
        }
        current = current->next;
    }

    if (!found) {
        printf("No movies found for year %d\n", year);
    }
}


// Function to search movies by title (partial match)
void search_by_title(const MovieNode* head, const char* search_term) {
    const MovieNode* current = head;
    int found = 0;

    printf("\nMovies with title containing \"%s\":\n", search_term);
    printf("----------------------------------------------------------------------\n");

    while (current != NULL) {
        // Case-insensitive search
        if (strcasestr(current->title, search_term) != NULL) {
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
            found = 1;
        }
        current = current->next;
    }

    if (!found) {
        printf("No movies found with \"%s\" in the title\n", search_term);
    }
}

// -------------------------------
// ----- COMPUTING FUNCTIONS -----
// -------------------------------

// Function to find average budget by year range
void average_budget_by_year_range(const MovieNode* head, int start_year, int end_year) {
    const MovieNode* current = head;
    double total_budget = 0.0;
    int count = 0;

    while (current != NULL) {
        if (current->year >= start_year && current->year <= end_year && current->budget > 0) {
            total_budget += current->budget;
            count++;
        }
        current = current->next;
    }

    printf("\nAverage budget for movies between %d and %d:\n", start_year, end_year);
    if (count > 0) {
        double average = total_budget / count;
        if (average >= 1000000) {
            printf("$%.2f million (based on %d movies)\n", average / 1000000.0, count);
        } else {
            printf("$%.2f (based on %d movies)\n", average, count);
        }
    } else {
        printf("No movies with budget information found for this period\n");
    }
}

// Function to count movies per decade
void count_movies_per_decade(const MovieNode* head) {
    const MovieNode* current = head;
    int decades[15] = {0}; // For decades from 1900s to 2040s
    int min_decade = 21;   // Initialize to a high value
    int max_decade = 0;    // Initialize to a low value

    while (current != NULL) {
        int decade = current->year / 10;
        int decade_index = decade - 190; // Adjust for array index (1900s -> 0)

        if (decade_index >= 0 && decade_index < 15) {
            decades[decade_index]++;
            if (decade_index < min_decade) min_decade = decade_index;
            if (decade_index > max_decade) max_decade = decade_index;
        }
        current = current->next;
    }

    printf("\nMovies per decade:\n");
    printf("----------------------------------------------------------------------\n");

    for (int i = min_decade; i <= max_decade; i++) {
        int decade_year = 1900 + (i * 10);
        printf("%ds: %d movies\n", decade_year, decades[i]);
    }
}

int main() {

    int current_choice = 0;
    MovieNode* movie_list = parse_csv_file("inputData/movies.csv");

    while (1) {
        printf("\n[MENU]\n");
        printf("1. Display all movies (sorted by year and title)\n");
        printf("2. Search movies by year\n");
        printf("3. Search movies by title\n");
        printf("4. Calculate average budget by year range\n");
        printf("5. Count movies per decade\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &current_choice);

        if (current_choice == 1) {
            printf("\n=== All Movies (Sorted by Year and Title) ===\n");
            print_list(movie_list);

        } else if (current_choice == 2) {
            // --- reading the year data from the user ---
            int year;
            scanf("%d", &year);

            // --- calling the search by year function ---
            search_by_year(movie_list, year);

        } else if (current_choice == 3) {
            // --- reading title data from the user ---
            char title[MAX_TITLE_LENGTH];
            scanf("%s", title);

            // --- removing newline character from title ---
            title[strcspn(title, "\n")] = 0;

            // --- calling the search by title function ---
            search_by_title(movie_list, title);

        } else if (current_choice == 4) {
            // --- reading the year range data from the user ---
            int start_year, end_year;
            printf("Enter start year: ");
            scanf("%d", &start_year);

            printf("Enter end year: ");
            scanf("%d", &end_year);

            // --- calling the average budget by year range function ---
            average_budget_by_year_range(movie_list, start_year, end_year);

        } else if (current_choice == 5) {
            // --- calling the count movies per decade function ---
            count_movies_per_decade(movie_list);

        } else if (current_choice == 0) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    // ----- free all allocated memory -----
    free_list(movie_list);

    return EXIT_SUCCESS;
}