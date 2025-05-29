#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_TITLE_LENGTH 256

// ---------------------------
// ----- DATA STRUCTURES -----
// ---------------------------
typedef struct MovieNode {
    int year;
    char* title;
    double budget;
    struct MovieNode* next;
} MovieNode;

// -------------------------------
// ----- AUXILIARY FUNCTIONS -----
// -------------------------------
void trim(char* str) {
    /**
     * @brief Trims leading and trailing whitespace from a string.
     *
     *  @param str Pointer to the null-terminated string to be trimmed. The string is modified in place.
     */

    if (str == NULL) return;

    // --- trimming the leading whitespace ---
    char* start = str;
    while (isspace((unsigned char)*start)) start++;

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // --- trimming the trailing whitespace ---
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

int find_column_index(char* header_line, const char* column_name) {
    /**
     * @brief Finds the index of a specified column name in a CSV header line.
     *
     *  @param header_line Pointer to the CSV header line string to be parsed. This string is modified by strtok_r.
     *  @param column_name The column name to search for (case-insensitive).
     *
     * @return The zero-based index of the column if found, or -1 if not found.
     */

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

    return -1;
}

// ---------------------------
// ----- MOVIE FUNCTIONS -----
// ---------------------------

MovieNode* create_movie_node(int year, const char* title, double budget) {
    /**
     * @brief Creates a new movie node with the given year, title, and budget.
     *
     *  @param year   The release year of the movie.
     *  @param title  The title of the movie (string will be duplicated).
     *  @param budget The budget of the movie.
     *
     * @return A pointer to the newly created MovieNode.
     */

    MovieNode* new_node = (MovieNode*)malloc(sizeof(MovieNode));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    new_node->year = year;
    new_node->title = strdup(title);
    if (new_node->title == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    new_node->budget = budget;
    new_node->next = NULL;

    return new_node;
}

void insert_sorted(MovieNode** head, MovieNode* new_node) {
    /**
     * @brief Inserts a movie node into the linked list in sorted order.
     *
     *  @param head      Pointer to the head pointer of the linked list.
     *  @param new_node  Pointer to the MovieNode to be inserted.
     */

    MovieNode* current;

    // --- checking if the list is empty or the new node should be the head ---
    if (*head == NULL || (*head)->year > new_node->year ||
        ((*head)->year == new_node->year && strcmp((*head)->title, new_node->title) > 0)) {
        new_node->next = *head;
        *head = new_node;
        return;
    }

    // --- finding the position to insert ---
    current = *head;
    while (current->next != NULL &&
           (current->next->year < new_node->year ||
            (current->next->year == new_node->year && strcmp(current->next->title, new_node->title) < 0))) {
        current = current->next;
    }

    // --- inserting the new node ---
    new_node->next = current->next;
    current->next = new_node;
}

void free_list(MovieNode* head) {
    /**
    * @brief Frees all nodes in the linked list, including movie titles.
    *
    *   @param head Pointer to the head of the linked list to be freed.
    */

    MovieNode* current = head;
    MovieNode* next;

    while (current != NULL) {
        next = current->next;
        free(current->title);
        free(current);
        current = next;
    }
}

MovieNode* parse_csv_file(const char* filename) {
    /**
     * @brief Parses a CSV file and builds a sorted linked list of movies.
     *
     *   @param filename Path to the CSV file containing movie data.
     *
     * @return Pointer to the head of the linked list containing movie nodes.
     */

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    char header_copy[MAX_LINE_LENGTH];
    MovieNode* head = NULL;

    // --- reading the header line ---
    if (fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Empty file or error reading header\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // --- removing newline character if present ---
    size_t len = strlen(line);
    if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
        line[len-1] = '\0';
    }

    // --- making a copy of the header for tokenization ---
    strcpy(header_copy, line);

    // --- finding the indices of the columns we need ---
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

    // --- reading the data lines ---
    while (fgets(line, sizeof(line), file) != NULL) {
        // removing newline character if present
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

        // parsing the line
        while ((token = strtok_r(rest, ",", &rest)) != NULL) {
            trim(token);

            if (column_index == year_index) {
                year = atoi(token);

            } else if (column_index == title_index) {
                // checking if this is a quoted field (which may contain commas)
                if (token[0] == '"' && token[strlen(token)-1] != '"') {
                    char quoted_field[MAX_TITLE_LENGTH] = "";
                    strncpy(quoted_field, token + 1, MAX_TITLE_LENGTH - 1);
                    strcat(quoted_field, ",");

                    // keeping to read tokens until we find the closing quote
                    while ((token = strtok_r(rest, ",", &rest)) != NULL) {
                        column_index++;

                        size_t token_len = strlen(token);
                        if (token[token_len-1] == '"') {
                            token[token_len-1] = '\0';
                            strncat(quoted_field, token, MAX_TITLE_LENGTH - strlen(quoted_field) - 1);
                            strncpy(title, quoted_field, MAX_TITLE_LENGTH - 1);
                            break;
                        } else {
                            strncat(quoted_field, token, MAX_TITLE_LENGTH - strlen(quoted_field) - 1);
                            strcat(quoted_field, ",");
                        }
                    }
                } else {
                    if (token[0] == '"' && token[strlen(token)-1] == '"') {
                        // removing quotes if present
                        token[strlen(token)-1] = '\0';
                        strncpy(title, token + 1, MAX_TITLE_LENGTH - 1);
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

        // --- creating and insert a new node if we have valid data ---
        if (year > 0 && strlen(title) > 0) {
            MovieNode* new_node = create_movie_node(year, title, budget);
            insert_sorted(&head, new_node);
        }
    }

    fclose(file);
    return head;
}

// ------------------------------
// ----- PRINTING FUNCTIONS -----
// ------------------------------

void print_list(const MovieNode* head) {
    /**
     * @brief Prints the contents of the movie linked list in a formatted table.
     *
     *   @param head Pointer to the head of the linked list.
     */

    const MovieNode* current = head;
    printf("Year    Title                                                Budget\n");
    printf("----------------------------------------------------------------------\n");

    while (current != NULL) {
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

void search_by_year(const MovieNode* head, int year) {
    /**
    * @brief Searches and prints all movies from the list that match a specific year.
    *
    *   @param head Pointer to the head of the linked list.
    *   @param year The year to search for.
    */

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

void search_by_title(const MovieNode* head, const char* search_term) {
    /**
    * @brief Searches and prints movies whose titles contain the given search term.
    *
    *   @param head        Pointer to the head of the linked list.
    *   @param search_term Substring to search for in movie titles (case-insensitive).
    */

    const MovieNode* current = head;
    int found = 0;

    printf("\nMovies with title containing \"%s\":\n", search_term);
    printf("----------------------------------------------------------------------\n");

    while (current != NULL) {
        // --- case-insensitive search ---
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

void average_budget_by_year_range(const MovieNode* head, int start_year, int end_year) {
    /**
     * @brief Calculates and prints the average budget of movies within a given year range.
     *
     *  @param head       Pointer to the head of the linked list.
     *  @param start_year The starting year of the range (inclusive).
     *  @param end_year   The ending year of the range (inclusive).
     */

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

void count_movies_per_decade(const MovieNode* head) {
    /**
     * @brief Counts and prints the number of movies released per decade.
     *
     *  @param head Pointer to the head of the linked list.
     */

    const MovieNode* current = head;
    int decades[15] = {0};     // for decades from 1900s to 2040s
    int min_decade = 21;       // initialize to a high value
    int max_decade = 0;        // initialize to a low value

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
            // --- displaying all movies sorted by year and title ---
            printf("\n=== All Movies (Sorted by Year and Title) ===\n");
            print_list(movie_list);

        } else if (current_choice == 2) {
            // --- searching movies by year ---
            int year;
            scanf("%d", &year);

            search_by_year(movie_list, year);

        } else if (current_choice == 3) {
            // --- searching movies by title ---
            char title[MAX_TITLE_LENGTH];
            scanf("%s", title);

            title[strcspn(title, "\n")] = 0;

            search_by_title(movie_list, title);

        } else if (current_choice == 4) {
            // --- computing the average budget by year range ---
            int start_year, end_year;
            printf("Enter start year: ");
            scanf("%d", &start_year);

            printf("Enter end year: ");
            scanf("%d", &end_year);

            average_budget_by_year_range(movie_list, start_year, end_year);

        } else if (current_choice == 5) {
            // --- computing the count for movies per decade ---
            count_movies_per_decade(movie_list);

        } else if (current_choice == 0) {
            // --- exiting the program ---
            printf("Exiting...\n");
            break;

        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    // ----- freeing all allocated memory -----
    free_list(movie_list);

    return EXIT_SUCCESS;
}