#include <stdio.h>
#include <stdbool.h>

#define MAX_SIZE 1000


// ----------------------------------
// ----- VARIABLES & STRUCTURES -----
// ----------------------------------

// ----- defining the maze structure -----
typedef struct {
    char grid[MAX_SIZE][MAX_SIZE];
    int rows;
    int cols;
    int start_row;
    int start_col;
    int end_row;
    int end_col;
} Maze;

// ----- direction arrays -----
int dr[] = {-1, 0, 1, 0};
int dc[] = {0, 1, 0, -1};

// ---------------------
// ----- FUNCTIONS -----
// ---------------------
bool read_maze(const char *filename, Maze *maze) {
    /*
     * Function to read a maze from a file
     *  @param filename: name of the input file
     *  @param maze: pointer to the maze structure
     * @return: true if successful, false otherwise
     * */
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening input file: %s\n", filename);
        return false;
    }

    // ----- reading the maze line by line -----
    char line[MAX_SIZE + 2];
    int row = 0;

    while (fgets(line, sizeof(line), file) && row < MAX_SIZE) {
        int col = 0;

        // ----- processing each character in the line -----
        for (int i = 0; line[i] != '\0' && line[i] != '\n' && col < MAX_SIZE; i++, col++) {
            maze->grid[row][col] = line[i];

            if (line[i] == 'S') {
                // ----- recording the start position -----
                maze->start_row = row;
                maze->start_col = col;
            } else if (line[i] == 'E') {
                // ----- recording the end position -----
                maze->end_row = row;
                maze->end_col = col;
            }
        }

        // ----- updating columns count (for the first row only) -----
        if (row == 0) {
            maze->cols = col;
        }

        row++;
    }

    maze->rows = row;
    fclose(file);

    // ----- validating that we found start and end positions -----
    if (maze->grid[maze->start_row][maze->start_col] != 'S' || maze->grid[maze->end_row][maze->end_col] != 'E') {
        printf("Error: Could not find start 'S' or end 'E' in the maze.\n");
        return false;
    }

    return true;
}

bool write_maze(const char *filename, const Maze *maze) {
    /*
     * Function to write the maze to a file
     *  @param filename: name of the output file
     *  @param maze: pointer to the maze structure
     * @return: true if successful, false otherwise
     * */
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening output file: %s\n", filename);
        return false;
    }

    for (int row = 0; row < maze->rows; row++) {
        for (int col = 0; col < maze->cols; col++) {
            fprintf(file, "%c", maze->grid[row][col]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return true;
}

bool is_valid_move(const Maze *maze, int row, int col, bool visited[MAX_SIZE][MAX_SIZE]) {
    /*
     * Function for checking if a move is valid
     *  @param maze: pointer to the maze structure
     *  @param row: row index
     *  @param col: column index
     *  @param visited: 2D array to track visited cells
     * @return: true if valid, false otherwise
     * */

    // ----- checking if within bounds -----
    if (row < 0 || row >= maze->rows || col < 0 || col >= maze->cols) {
        return false;
    }

    // ----- checking if it's a wall or already visited -----
    if (maze->grid[row][col] == '#' || visited[row][col]) {
        return false;
    }

    return true;
}

bool solve_maze(Maze *maze, int row, int col, bool visited[MAX_SIZE][MAX_SIZE]) {
    /*
     * Function for solving the maze using backtracking
     *  @param maze: pointer to the maze structure
     *  @param row: current row index
     *  @param col: current column index
     *  @param visited: 2D array to track visited cells
     * @return: true if a solution is found, false otherwise
     * */

    // ----- marking the current cell as visited -----
    visited[row][col] = true;

    // ----- once we have reached the exit, we're done -----
    if (row == maze->end_row && col == maze->end_col) {
        return true;
    }

    // ----- trying all possible moves -----
    for (int i = 0; i < 4; i++) {
        int new_row = row + dr[i];
        int new_col = col + dc[i];

        if (is_valid_move(maze, new_row, new_col, visited)) {
            // ----- marking the path with a dot if it's not start or end -----
            if (maze->grid[new_row][new_col] != 'S' && maze->grid[new_row][new_col] != 'E') {
                maze->grid[new_row][new_col] = '.';
            }

            // ----- recursive calling to continue path finding -----
            if (solve_maze(maze, new_row, new_col, visited)) {
                return true;
            }

            if (maze->grid[new_row][new_col] == '.') {
                maze->grid[new_row][new_col] = ' ';
            }
        }
    }

    return false;
}

int main() {

    // ----- reading the maze from input file -----
    char *input_file = "inputData/small_maze.dat";
    Maze maze;
    bool visited[MAX_SIZE][MAX_SIZE] = {false};
    if (!read_maze(input_file, &maze)) {
        return 1;
    }

    // ----- solving the maze using backtracking -----
    if (!solve_maze(&maze, maze.start_row, maze.start_col, visited)) {
        printf("No solution found for the maze.\n");
        return 1;
    }

    // ----- writing the solution to an output file -----
    if (!write_maze("output_maze.dat", &maze)) {
        return 1;
    }

    printf("Maze solved successfully!\n");
    return 0;
}