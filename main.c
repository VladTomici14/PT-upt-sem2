#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
#define MAX_RECORDS 1000
#define MAX_FIELDS 32
#define MAX_FIELD_SIZE 256

// ----- structure for storing a single record from the csv -----
typedef struct {
    char fields[MAX_FIELDS][MAX_FIELD_SIZE];
    int field_count;
} Record;

typedef int (*CompareFunc)(const void *, const void *);

void my_qsort(void *base, int left, int right, size_t size, CompareFunc cmp) {
    if (left >= right) return;

    char *arr = (char *)base;
    int i = left;
    int j = right;
    void *pivot = malloc(size);
    memcpy(pivot, arr + (left + (right - left) / 2) * size, size);

    while (i <= j) {
        while (cmp(arr + i * size, pivot) < 0) i++;
        while (cmp(arr + j * size, pivot) > 0) j--;

        if (i <= j) {
            void *temp = malloc(size);
            memcpy(temp, arr + i * size, size);
            memcpy(arr + i * size, arr + j * size, size);
            memcpy(arr + j * size, temp, size);
            free(temp);
            i++;
            j--;
        }
    }

    free(pivot);
    my_qsort(base, left, j, size, cmp);
    my_qsort(base, i, right, size, cmp);
}

// ----- function to parse a line from the csv into record structure -----
void parse_csv_line(char *line, Record *record) {
    int field_index = 0;
    int char_index = 0;
    int in_quotes = 0;

    record->field_count = 0;

    for (int i = 0; line[i] != '\0' && line[i] != '\n' && line[i] != '\r'; i++) {
        if (line[i] == '"') {
            in_quotes = !in_quotes;
        } else if (line[i] == ',' && !in_quotes) {
            record->fields[field_index][char_index] = '\0';
            field_index++;
            char_index = 0;
            if (field_index >= MAX_FIELDS) break;
        } else {
            record->fields[field_index][char_index] = line[i];
            char_index++;
            if (char_index >= MAX_FIELD_SIZE - 1) char_index = MAX_FIELD_SIZE - 2;
        }
    }

    // Add the last field
    record->fields[field_index][char_index] = '\0';
    record->field_count = field_index + 1;
}

// Custom comparison function for sorting by multiple fields
int compare_records(const void *a, const void *b, int *sort_fields, int sort_count) {
    const Record *record_a = (const Record *) a;
    const Record *record_b = (const Record *) b;

    // Compare each sort field in order
    for (int i = 0; i < sort_count; i++) {
        int field_index = sort_fields[i];

        // Check if the field index is valid for both records
        if (field_index >= record_a->field_count || field_index >= record_b->field_count) {
            continue;
        }

        // Try numeric comparison first
        char *end_a, *end_b;
        double num_a = strtod(record_a->fields[field_index], &end_a);
        double num_b = strtod(record_b->fields[field_index], &end_b);

        // If both fields are valid numbers
        if (*end_a == '\0' && *end_b == '\0') {
            if (num_a < num_b) return -1;
            if (num_a > num_b) return 1;
        } else {
            // String comparison
            int cmp = strcmp(record_a->fields[field_index], record_b->fields[field_index]);
            if (cmp != 0) return cmp;
        }
    }

    // If all fields are equal
    return 0;
}

// Wrapper function for qsort
int compare_wrapper(const void *a, const void *b) {
    // These should match the names in sort_records
    static int sort_fields_static[MAX_FIELDS];
    static int sort_count_static;

    return compare_records(a, b, sort_fields_static, sort_count_static);
}

// Function to set sort parameters and call qsort
void sort_records(Record records, int record_count, int *sort_fields, int sort_count) {
    // Set static variables for the compare_wrapper
    extern int compare_wrapper(const void *, const void *);

    // The static variables in compare_wrapper need to be updated
    static int sort_fields_static[MAX_FIELDS];
    static int sort_count_static;

    // Copy the sort fields and count to static variables
    for (int i = 0; i < sort_count; i++) {
        sort_fields_static[i] = sort_fields[i];
    }
    sort_count_static = sort_count;

    // Call qsort with our wrapper function
    my_qsort(&records, 0, record_count - 1, sizeof(Record), compare_wrapper);
}

void displayRecords(Record *records, Record header, int record_count) {
    printf("\nData Records (%d total):\n", record_count);

    // ----- displaying the header -----
    for (int j = 0; j < header.field_count; j++) {
        if (j == 0) {
            printf("%-3s", header.fields[j]);
        } else if (j == 1) {
            printf("%-40s", header.fields[j]);
        } else {
            printf("%-25s", header.fields[j]);
        }
    }
    printf("\n");

    for (int j = 0; j < header.field_count; j++) {
        if (j == 0) {
            for (int k = 0; k < 3; k++) {
                printf("-");
            }

        } else if (j == 1) {
            for (int k = 0; k < 40; k++) {
                printf("-");
            }
        } else {
            for (int k = 0; k < 25; k++) {
                printf("-");
            }
        }
    }
    printf("\n");

    // ----- displaying the records with some formatting -----
    for (int i = 0; i < record_count; i++) {
        for (int j = 0; j < records[i].field_count; j++) {
            if (j == 0) {
                printf("%-3s", records[i].fields[j]);
            } else if (j == 1) {
                printf("%-40s", records[i].fields[j]);
            } else {
                printf("%-25s", records[i].fields[j]);
            }
        }
        printf("\n");
    }
}

int main() {
    Record records[MAX_RECORDS];
    Record header;
    char filename[256];
    int record_count = 0;
    int choice = 0;
    int sort_fields[MAX_FIELDS];
    int sort_count = 0;
    int has_loaded_file = 0;
    FILE *file = NULL;

    while (1) {
        printf("\n----- CSV Sorting Program -----\n");
        printf("1. Load CSV file\n");
        printf("2. Display data\n");
        printf("3. Sort data\n");
        printf("4. Save sorted data\n");
        printf("0. Exit\n");
        printf("\nEnter your choice: ");

        if (scanf("%d", &choice) != 1) {
            // Clear input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input.\n");
            continue;
        }

        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (choice == 1) {
            // --------------------------------
            // ----- loading the csv file -----
            // --------------------------------
            printf("Input csv path: ");
            if (scanf("%s", filename) != 1) {
                printf("Error reading filename\n");
                continue;
            }

            file = fopen(filename, "r");
            if (!file) {
                perror("Error opening file");
            }

            // Reset record count
            record_count = 0;
            char line[MAX_LINE_SIZE];

            // Read header
            if (fgets(line, MAX_LINE_SIZE, file)) {
                parse_csv_line(line, &header);
                printf("\nHeader: ");
                for (int i = 0; i < header.field_count; i++) {
                    printf("%s%s", header.fields[i], i < header.field_count - 1 ? ", " : "\n");
                }
            }

            // Read data records
            while (fgets(line, MAX_LINE_SIZE, file) && record_count < MAX_RECORDS) {
                parse_csv_line(line, &records[record_count]);
                record_count++;
            }

            fclose(file);
            has_loaded_file = 1;

            printf("Read %d records from CSV file\n", record_count);

        } else if (choice == 2) {
            // -------------------------------
            // ----- displaying the data -----
            // -------------------------------

            if (!has_loaded_file) {
                printf("No file loaded yet! Please load a file first.\n");
                continue;
            }

            displayRecords(records, header, record_count);

        } else if (choice == 3) {
            // ----------------------------
            // ----- sorting the data -----
            // ----------------------------

            if (!has_loaded_file) {
                printf("No file loaded yet! Please load a file first.\n");
                continue;
            }

            printf("Available fields:\n");
            for (int i = 0; i < header.field_count; i++) {
                printf("%d: %s\n", i + 1, header.fields[i]);
            }

            printf("\nHow many fields would you like to sort by? ");
            if (scanf("%d", &sort_count) != 1 || sort_count <= 0 || sort_count > MAX_FIELDS) {
                printf("Invalid number of sort fields\n");
                // Clear input buffer
                while ((c = getchar()) != '\n' && c != EOF);
                break;
            }

            // Clear input buffer
            while ((c = getchar()) != '\n' && c != EOF);

            printf("Enter field numbers to sort by in order of priority:\n");
            for (int i = 0; i < sort_count; i++) {  // Start from 0, not 1
                printf("Sort field %d: ", i + 1);
                if (scanf("%d", &sort_fields[i]) != 1 || sort_fields[i] <= 0 || sort_fields[i] > header.field_count) {  // Use header.field_count
                    printf("Invalid field number\n");
                    i--; // Retry this input
                    // Clear input buffer
                    while ((c = getchar()) != '\n' && c != EOF);
                    continue;
                }
                sort_fields[i]--; // Convert to 0-based index
            }

            // Clear input buffer
            while ((c = getchar()) != '\n' && c != EOF);

            // Sort the records
            sort_records(*records, record_count, sort_fields, sort_count);

            printf("\nData sorted successfully!\n");
            displayRecords(records, header, record_count);
            continue;

        } else if (choice == 4) {
            // ----------------------------------
            // ----- saving the sorted data -----
            // ----------------------------------
            if (!has_loaded_file) {
                printf("No file loaded yet! Please load a file first.\n");
                continue;
            }

            printf("Enter filename to save sorted data: ");
            char save_filename[256];
            if (scanf("%255s", save_filename) != 1) {
                printf("Error reading filename\n");
                break;
            }

            // Clear input buffer
            while ((c = getchar()) != '\n' && c != EOF);

            file = fopen(save_filename, "w");
            if (!file) {
                perror("Error opening file for writing");
                break;
            }

            // Write header
            for (int j = 0; j < header.field_count; j++) {
                fprintf(file, "%s%s", header.fields[j], j < header.field_count - 1 ? "," : "\n");
            }

            // Write data
            for (int i = 0; i < record_count; i++) {
                for (int j = 0; j < records[i].field_count; j++) {
                    fprintf(file, "%s%s", records[i].fields[j], j < records[i].field_count - 1 ? "," : "\n");
                }
            }

            fclose(file);
            printf("Data saved to %s\n", save_filename);
            break;

        } else if (choice == 0) {
            // -------------------------------
            // ----- exiting the program -----
            // -------------------------------
            printf("Exiting program...\n");
            return 0;

        } else {
            printf("Invalid choice. Please try again.\n");
        }


    }

    return 0;
}