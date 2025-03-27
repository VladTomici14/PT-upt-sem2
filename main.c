#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------------------------
// ----- DATA STRUCTURES -----
// ---------------------------
typedef struct {
    long dt;                       // Unix timestamp
    char dt_iso[64];               // ISO formatted date and time
    int timezone;                  // Timezone offset
    char city_name[100];           // City name
    double lat;                    // Latitude
    double lon;                    // Longitude
    double temp;                   // Temperature
    int visibility;                // Visibility
    double dew_point;              // Dew point
    double feels_like;             // Feels like temperature
    double temp_min;               // Minimum temperature
    double temp_max;               // Maximum temperature
    int pressure;                  // Atmospheric pressure
    int sea_level;                 // Sea level pressure (could be 0 if not provided)
    int grnd_level;                // Ground level pressure (could be 0 if not provided)
    int humidity;                  // Humidity percentage
    double wind_speed;             // Wind speed
    int wind_deg;                  // Wind direction (degrees)
    double wind_gust;              // Wind gust speed
    double rain_1h;                // Rain in the last 1 hour
    double rain_3h;                // Rain in the last 3 hours
    double snow_1h;                // Snow in the last 1 hour
    double snow_3h;                // Snow in the last 3 hours
    int clouds_all;                // Cloud cover percentage
    int weather_id;                // Weather condition ID
    char weather_main[50];         // Main weather condition
    char weather_description[100]; // Weather description
    char weather_icon[10];         // Weather icon code
} DataEntry;

typedef struct {
    double avg_temp;
    double avg_humidity;
    double avg_pressure;
    struct {
        char type[50];
        int count;
    } weather_types[10];
    int weather_type_count;
} BasicStatistics;

typedef struct {
    DataEntry *entries;
    int count;
} FilteredResults;

typedef struct {
    DataEntry highest_temp;
    DataEntry lowest_temp;
    DataEntry strongest_wind;
    DataEntry highest_humidity;
} ExtremeValues;

typedef struct {
    int hour;
    double avg_temp;
    double temp_trend;
} HourlyAnalysis;

// ----------------------------
// ----- BASIC STATISTICS -----
// ----------------------------
BasicStatistics calculateBasicStatistics(DataEntry *entries, int numEntries) {
    /*
     * function for computing the basic statistics
     * */
    BasicStatistics stats = {0};
    double total_temp = 0, total_humidity = 0, total_pressure = 0;

    // Initialize weather type counts
    stats.weather_type_count = 0;

    // Temporary arrays to track weather type counts
    char weather_types[10][50] = {0};
    int weather_counts[10] = {0};

    for (int i = 0; i < numEntries; i++) {
        // Accumulate totals
        total_temp += entries[i].temp;
        total_humidity += entries[i].humidity;
        total_pressure += entries[i].pressure;

        int found = 0;
        for (int j = 0; j < stats.weather_type_count; j++) {
            if (strcmp(weather_types[j], entries[i].weather_main) == 0) {
                weather_counts[j]++;
                found = 1;
                break;
            }
        }

        // If not found, add new weather type
        if (!found) {
            strcpy(weather_types[stats.weather_type_count], entries[i].weather_main);
            weather_counts[stats.weather_type_count] = 1;
            stats.weather_type_count++;
        }
    }

    // --- computing averages ---
    stats.avg_temp = total_temp / numEntries;
    stats.avg_humidity = total_humidity / numEntries;
    stats.avg_pressure = total_pressure / numEntries;

    for (int i = 0; i < stats.weather_type_count; i++) {
        strcpy(stats.weather_types[i].type, weather_types[i]);
        stats.weather_types[i].count = weather_counts[i];
    }

    return stats;
}

void printBasicStatistics(const BasicStatistics *stats) {
    /*
     * function for printing basic statistics
     * */
    printf("Basic Statistics:\n");
    printf("Average Temperature: %.2f°C\n", stats->avg_temp);
    printf("Average Humidity: %.2f%%\n", stats->avg_humidity);
    printf("Average Pressure: %.2f hPa\n", stats->avg_pressure);

    printf("\nWeather Type Counts:\n");
    for (int i = 0; i < stats->weather_type_count; i++) {
        printf("%s: %d\n", stats->weather_types[i].type, stats->weather_types[i].count);
    }
}

// -------------------------------
// ----- FILTERING FUNCTIONS -----
// -------------------------------
FilteredResults findRecordsByDateRange(DataEntry *entries, int numEntries, long start_timestamp, long end_timestamp) {
    /*
     * function for finding records by date range
     * */
    FilteredResults results = {NULL, 0};

    // --- allocating memory --
    results.entries = malloc(numEntries * sizeof(DataEntry));

    for (int i = 0; i < numEntries; i++) {
        if (entries[i].dt >= start_timestamp && entries[i].dt <= end_timestamp) {
            results.entries[results.count] = entries[i];
            results.count++;
        }
    }

    results.entries = realloc(results.entries, results.count * sizeof(DataEntry));

    return results;
}

FilteredResults findRecordsByWeatherType(DataEntry *entries, int numEntries, const char *weather_type) {
    /*
     * function for finding records by weather type
     * */
    FilteredResults results = {NULL, 0};

    // --- allocating memory ---
    results.entries = malloc(numEntries * sizeof(DataEntry));

    for (int i = 0; i < numEntries; i++) {
        if (strcasecmp(entries[i].weather_main, weather_type) == 0) {
            results.entries[results.count] = entries[i];
            results.count++;
        }
    }

    results.entries = realloc(results.entries, results.count * sizeof(DataEntry));

    return results;
}

// --------------------------
// ----- EXTREME VALUES -----
// --------------------------
ExtremeValues findExtremeValues(DataEntry *entries, int numEntries) {
    ExtremeValues extremes;

    // --- initialising with a first entry ---
    extremes.highest_temp = entries[0];
    extremes.lowest_temp = entries[0];
    extremes.strongest_wind = entries[0];
    extremes.highest_humidity = entries[0];

    for (int i = 1; i < numEntries; i++) {
        // --- highest temperature ---
        if (entries[i].temp > extremes.highest_temp.temp) {
            extremes.highest_temp = entries[i];
        }

        // --- lowest temperature ---
        if (entries[i].temp < extremes.lowest_temp.temp) {
            extremes.lowest_temp = entries[i];
        }

        // --- strongest wind ---
        if (entries[i].wind_speed > extremes.strongest_wind.wind_speed) {
            extremes.strongest_wind = entries[i];
        }

        // --- highest humidity ---
        if (entries[i].humidity > extremes.highest_humidity.humidity) {
            extremes.highest_humidity = entries[i];
        }
    }

    return extremes;
}

// ---------------------------
// ----- HOURLY ANALYSIS -----
// ---------------------------
HourlyAnalysis *calculateHourlyTemperatures(DataEntry *entries, int numEntries, int *num_hours) {
    HourlyAnalysis *hourly_temps = malloc(24 * sizeof(HourlyAnalysis));
    *num_hours = 0;

    for (int i = 0; i < 24; i++) {
        hourly_temps[i].hour = i;
        hourly_temps[i].avg_temp = 0;
        hourly_temps[i].temp_trend = 0;
    }

    int hour_counts[24] = {0};
    for (int i = 0; i < numEntries; i++) {
        // extracting hour from the dt_iso
        struct tm tm;
        memset(&tm, 0, sizeof(struct tm));
        strptime(entries[i].dt_iso, "%Y-%m-%d %H:%M:%S", &tm);

        int hour = tm.tm_hour;
        hourly_temps[hour].avg_temp += entries[i].temp;
        hour_counts[hour]++;
    }

    // --- computing averages ---
    for (int i = 0; i < 24; i++) {
        if (hour_counts[i] > 0) {
            hourly_temps[i].avg_temp /= hour_counts[i];
            *num_hours += 1;
        }
    }

    // --- temperature trend with simple linear regression ---
    if (*num_hours > 1) {
        double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;

        for (int i = 0; i < 24; i++) {
            if (hour_counts[i] > 0) {
                sum_x += i;
                sum_y += hourly_temps[i].avg_temp;
                sum_xy += i * hourly_temps[i].avg_temp;
                sum_x2 += i * i;
            }
        }

        // --- linear regression slope ---
        double slope = ((*num_hours * sum_xy) - (sum_x * sum_y)) / ((*num_hours * sum_x2) - (sum_x * sum_x));

        // --- assigning trend to each hour ---
        for (int i = 0; i < 24; i++) {
            hourly_temps[i].temp_trend = slope;
        }
    }

    return hourly_temps;
}

// ----------------------------------
// ----- DATA ENTRIES FUNCTIONS -----
// ----------------------------------
void printDataEntry(const DataEntry *entry) {
    /*
     * Function for printing a single data entry
     * */
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

    // --- printing precipitation if available ---
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

DataEntry *readCSVFile(const char *filename, int *numEntries) {
    /*
     * function for parsing the CSV
     * */
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    // --- allocating initial memory for initial entries ---
    int capacity = 10;
    DataEntry *entries = malloc(capacity * sizeof(DataEntry));
    *numEntries = 0;

    char line[1024];

    // --- skipping header line ---
    fgets(line, sizeof(line), file);

    // --- reading each line ---
    while (fgets(line, sizeof(line), file)) {
        if (*numEntries >= capacity) {
            capacity *= 2;
            entries = realloc(entries, capacity * sizeof(DataEntry));
        }

        DataEntry *entry = &entries[*numEntries];

        sscanf(line,
               "%ld,%63[^,],%d,%99[^,],%lf,%lf,%lf,%d,%lf,%lf,%lf,%lf,%d,%*[^,],%*[^,],%d,%lf,%d,%lf,%lf,%lf,%lf,%lf,%d,%d,%49[^,],%99[^,],%9[^\n]",
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
    DataEntry *entries = readCSVFile("inputData/Timisoara.csv", &numEntries);

    if (entries) {
        // --- showing basic statistics ---
        BasicStatistics stats = calculateBasicStatistics(entries, numEntries);
        printBasicStatistics(&stats);

        // --- extreme values example ---
        ExtremeValues extremes = findExtremeValues(entries, numEntries);
        printf("\nExtreme Values:\n");
        printf("Highest Temperature: %.2f°C at %s\n",
               extremes.highest_temp.temp, extremes.highest_temp.dt_iso);
        printf("Lowest Temperature: %.2f°C at %s\n",
               extremes.lowest_temp.temp, extremes.lowest_temp.dt_iso);
        printf("Strongest Wind: %.2f m/s at %s\n",
               extremes.strongest_wind.wind_speed, extremes.strongest_wind.dt_iso);

        // --- filtering example ---
        FilteredResults rain_records = findRecordsByWeatherType(entries, numEntries, "Rain");
        printf("\nRain Records Count: %d\n", rain_records.count);

        // --- hourly temperature analysis ---
        int num_hours;
        HourlyAnalysis *hourly_temps = calculateHourlyTemperatures(entries, numEntries, &num_hours);

        printf("\nHourly Temperature Analysis:\n");
        for (int i = 0; i < num_hours; i++) {
            printf("Hour %d: Avg Temp = %.2f°C, Trend = %.4f\n",
                   hourly_temps[i].hour,
                   hourly_temps[i].avg_temp,
                   hourly_temps[i].temp_trend);
        }

        // --- freeing memory ---
        free(entries);
        free(rain_records.entries);
        free(hourly_temps);
    }

    return 0;
}
