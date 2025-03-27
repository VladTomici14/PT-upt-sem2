#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Define the DataEntry struct to match the CSV columns
typedef struct {
    long dt;               // Unix timestamp
    char dt_iso[64];       // ISO formatted date and time
    int timezone;          // Timezone offset
    char city_name[100];   // City name
    double lat;            // Latitude
    double lon;            // Longitude
    double temp;           // Temperature
    int visibility;        // Visibility
    double dew_point;      // Dew point
    double feels_like;     // Feels like temperature
    double temp_min;       // Minimum temperature
    double temp_max;       // Maximum temperature
    int pressure;          // Atmospheric pressure
    int sea_level;         // Sea level pressure (could be 0 if not provided)
    int grnd_level;        // Ground level pressure (could be 0 if not provided)
    int humidity;          // Humidity percentage
    double wind_speed;     // Wind speed
    int wind_deg;          // Wind direction (degrees)
    double wind_gust;      // Wind gust speed
    double rain_1h;        // Rain in the last 1 hour
    double rain_3h;        // Rain in the last 3 hours
    double snow_1h;        // Snow in the last 1 hour
    double snow_3h;        // Snow in the last 3 hours
    int clouds_all;        // Cloud cover percentage
    int weather_id;        // Weather condition ID
    char weather_main[50]; // Main weather condition
    char weather_description[100]; // Weather description
    char weather_icon[10]; // Weather icon code
} DataEntry;

// Function to print a single DataEntry
void printDataEntry(const DataEntry* entry) {
    printf("Unix Timestamp: %ld\n", entry->dt);
    printf("ISO Date/Time: %s\n", entry->dt_iso);
    printf("Timezone: %d\n", entry->timezone);
    printf("City: %s\n", entry->city_name);
    printf("Location: (%.6f, %.6f)\n", entry->lat, entry->lon);
    printf("Temperature: %.2f°C\n", entry->temp);
    printf("Feels Like: %.2f°C\n", entry->feels_like);
    printf("Min/Max Temp: %.2f/%.2f°C\n", entry->temp_min, entry->temp_max);
    printf("Visibility: %d\n", entry->visibility);
    printf("Dew Point: %.2f°C\n", entry->dew_point);
    printf("Pressure: %d hPa\n", entry->pressure);
    printf("Humidity: %d%%\n", entry->humidity);
    printf("Wind: %.2f m/s, Direction: %d°\n", entry->wind_speed, entry->wind_deg);

    // Print precipitation if available
    if (entry->rain_1h > 0)
        printf("Rain (1h): %.2f mm\n", entry->rain_1h);
    if (entry->rain_3h > 0)
        printf("Rain (3h): %.2f mm\n", entry->rain_3h);
    if (entry->snow_1h > 0)
        printf("Snow (1h): %.2f mm\n", entry->snow_1h);
    if (entry->snow_3h > 0)
        printf("Snow (3h): %.2f mm\n", entry->snow_3h);

    printf("Cloud Cover: %d%%\n", entry->clouds_all);
    printf("Weather: %s (%s)\n", entry->weather_main, entry->weather_description);
    printf("Weather Icon: %s\n", entry->weather_icon);
    printf("\n");
}

// Function to read CSV file and parse entries
DataEntry* readCSVFile(const char* filename, int* numEntries) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // Allocate initial memory for entries
    int capacity = 10;
    DataEntry* entries = malloc(capacity * sizeof(DataEntry));
    *numEntries = 0;

    // Buffer for reading lines
    char line[1024];
    // Skip header line
    fgets(line, sizeof(line), file);

    // Read each line
    while (fgets(line, sizeof(line), file)) {
        // Resize array if needed
        if (*numEntries >= capacity) {
            capacity *= 2;
            entries = realloc(entries, capacity * sizeof(DataEntry));
        }

        // Parse the line
        DataEntry* entry = &entries[*numEntries];

        // Use sscanf to parse CSV line
        // Note: This is a basic parser and might need refinement for complex CSV files
        sscanf(line, "%ld,%63[^,],%d,%99[^,],%lf,%lf,%lf,%d,%lf,%lf,%lf,%lf,%d,%*[^,],%*[^,],%d,%lf,%d,%lf,%lf,%lf,%lf,%lf,%d,%d,%49[^,],%99[^,],%9[^\n]",
               &entry->dt, entry->dt_iso, &entry->timezone, entry->city_name,
               &entry->lat, &entry->lon, &entry->temp, &entry->visibility,
               &entry->dew_point, &entry->feels_like, &entry->temp_min,
               &entry->temp_max, &entry->pressure, &entry->humidity,
               &entry->wind_speed, &entry->wind_deg, &entry->wind_gust,
               &entry->rain_1h, &entry->rain_3h, &entry->snow_1h,
               &entry->snow_3h, &entry->clouds_all, &entry->weather_id,
               entry->weather_main, entry->weather_description, entry->weather_icon);

        (*numEntries)++;
    }

    fclose(file);
    return entries;
}

int main() {
    int numEntries;
    DataEntry* entries = readCSVFile("inputData/Timisoara.csv", &numEntries);

    if (entries) {
        // Print each entry
        for (int i = 0; i < numEntries; i++) {
            printDataEntry(&entries[i]);
        }

        // Free allocated memory
        free(entries);
    }

    return 0;
}