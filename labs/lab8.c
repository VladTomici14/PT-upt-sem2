#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#define INF INT_MAX
#define MAX_CITIES 100
#define MAX_NAME_LENGTH 50

int NUM_CITIES = 0;
char city_names[MAX_CITIES][MAX_NAME_LENGTH];

// ----- function to find the nearest unvisited city -----
int findNearestCity(int distances[MAX_CITIES][MAX_CITIES], int current, bool visited[MAX_CITIES]) {
    int minDistance = INF;
    int nearestCity = -1;

    for (int i = 0; i < NUM_CITIES; i++) {
        // ----- checking if the city is unvisited and there is a path from current to i -----
        if (!visited[i] && distances[current][i] != INF && distances[current][i] != 0) {
            if (distances[current][i] < minDistance) {
                minDistance = distances[current][i];
                nearestCity = i;
            }
        }
    }

    return nearestCity;
}

// ----- function to solve using greedy approach -----
void solveTSP(int distances[MAX_CITIES][MAX_CITIES], int startCity) {
    bool visited[MAX_CITIES];
    for (int i = 0; i < NUM_CITIES; i++) {
        visited[i] = false;
    }
    int path[MAX_CITIES + 1];
    int totalDistance = 0;

    // ----- starting with the given city -----
    int currentCity = startCity;
    path[0] = startCity;
    visited[startCity] = true;

    // ----- visiting all cities -----
    for (int i = 1; i < NUM_CITIES; i++) {
        int nextCity = findNearestCity(distances, currentCity, visited);

        if (nextCity == -1) {
            printf("No valid path to unvisited cities from %s\n\n", city_names[currentCity]);
            return;
        }

        path[i] = nextCity;
        visited[nextCity] = true;
        totalDistance += distances[currentCity][nextCity];

        currentCity = nextCity;
    }

    // ----- returning to the starting city to complete the cycle -----
    if (distances[currentCity][startCity] != INF && distances[currentCity][startCity] != 0) {
        path[NUM_CITIES] = startCity;
    } else {
        printf("No direct path back to the starting city %s\n\n", city_names[startCity]);
        return;
    }

    for (int i = 0; i < NUM_CITIES; i++) {
        printf("%s", city_names[path[i]]);
        if (i < NUM_CITIES - 1) printf(" -> ");
    }
    printf(" (Total distance: %dkm)\n\n", totalDistance);
}

// ----- function to read distance matrix from file -----
int readDistanceMatrix(const char* filename, int distances[MAX_CITIES][MAX_CITIES]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", filename);
        return 0;
    }

    char line[1024];

    // ----- reading the first line with city names -----
    if (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, " \n");
        while (token != NULL && NUM_CITIES < MAX_CITIES) {
            strcpy(city_names[NUM_CITIES], token);
            NUM_CITIES++;
            token = strtok(NULL, " \n");
        }
    }

    // ----- initialising the distance matrix with INF -----
    for (int i = 0; i < NUM_CITIES; i++) {
        for (int j = 0; j < NUM_CITIES; j++) {
            if (i == j) {
                distances[i][j] = 0;
            } else {
                distances[i][j] = INF;
            }
        }
    }

    // ----- reading distance matrix -----
    int row = 0;
    while (fgets(line, sizeof(line), file) && row < NUM_CITIES) {
        char *token = strtok(line, " \n");
        int col = 0;

        while (token != NULL && col < NUM_CITIES) {
            int distance = atoi(token);
            if (distance == 0 && row != col) {
                distances[row][col] = INF;
            } else {
                distances[row][col] = distance;
            }
            col++;
            token = strtok(NULL, " \n");
        }
        row++;
    }

    fclose(file);
    return 1;
}

int main() {
    int distances[MAX_CITIES][MAX_CITIES];

    // ----- reading distance matrix from file -----
    const char* input_file_path = "../inputData/input-city.txt";
    if (!readDistanceMatrix(input_file_path, distances)) {
        printf("Failed to read distance matrix. Check if the file path is correct.\n");
        printf("Current path: %s\n", input_file_path);
        return 1;
    }

    // ---- finding the solutions starting from each city -----
    for (int i = 0; i < NUM_CITIES; i++) {
        printf("%s: ", city_names[i]);
        solveTSP(distances, i);
    }

    return 0;
}
