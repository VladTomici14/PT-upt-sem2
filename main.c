// [] opening and reading the weather from the CSV
// [] parsing the headers columns
// [] extracting values for each column

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 2048
#define MAX_CITY_NAME_LENGTH 100
#define MAX_WEATHER_DESCRIPTION_LENGTH 100
#define MAX_TIMEZONE_LENGTH 10

# define MAX_FIELDS 28
# define MAX_FIELD_LENGTH 256

typedef struct {
    int year;                                   // Year (e.g., 1979)
    int month;                                  // Month (1-12)
    int day;                                    // Day of the month (1-31)
    int hour;                                   // Hour (0-23)
    int minute;                                 // Minute (0-59)
    int second;                                 // Second (0-59)
    int timezone_offset;                        // Timezone offset in seconds (e.g., +0000 UTC -> 0, or +0200 -> 7200)
    char timezone[MAX_TIMEZONE_LENGTH];         // Timezone abbreviation (e.g., UTC)
} DateTimeISO;

typedef struct {
    long dt;                                                   // Timestamp (seconds since epoch)
    DateTimeISO dt_iso;                                        // ISO 8601 datetime structure
    int timezone;                                              // Timezone offset in seconds
    char city_name[MAX_CITY_NAME_LENGTH];                      // City name (e.g., Timișoara)
    double lat;                                                // Latitude
    double lon;                                                // Longitude
    float temp;                                                // Temperature (e.g., 9.11)
    int visibility;                                            // Visibility (e.g., 6000)
    float dew_point;                                           // Dew point (e.g., 9.11)
    float feels_like;                                          // Feels like temperature (e.g., 8.16)
    float temp_min;                                            // Minimum temperature (e.g., 8.12)
    float temp_max;                                            // Maximum temperature (e.g., 9.11)
    int pressure;                                              // Pressure (e.g., 991)
    int sea_level;                                             // Sea level pressure (optional, may be empty in data)
    int grnd_level;                                            // Ground level pressure (optional, may be empty in data)
    int humidity;                                              // Humidity percentage (e.g., 100)
    float wind_speed;                                          // Wind speed (e.g., 2.27)
    int wind_deg;                                              // Wind degree (e.g., 230)
    float wind_gust;                                           // Wind gust (optional, may be empty)
    float rain_1h;                                             // Rain in the last 1 hour (optional, may be empty)
    float rain_3h;                                             // Rain in the last 3 hours (optional, may be empty)
    float snow_1h;                                             // Snow in the last 1 hour (optional, may be empty)
    float snow_3h;                                             // Snow in the last 3 hours (optional, may be empty)
    int clouds_all;                                            // Cloud coverage percentage (e.g., 100)
    int weather_id;                                            // Weather condition ID (e.g., 500)
    char weather_main[MAX_WEATHER_DESCRIPTION_LENGTH];         // Main weather description (e.g., Rain)
    char weather_description[MAX_WEATHER_DESCRIPTION_LENGTH];  // Full weather description (e.g., light rain)
    char weather_icon[10];                                     // Weather icon (e.g., 10n)
} DataEntry;

void printDateTimeISO(DateTimeISO dt) {
    printf("%04d-%02d-%02d %02d:%02d:%02d %s%02d\n",
           dt.year,
           dt.month,
           dt.day,
           dt.hour,
           dt.minute,
           dt.second,
           dt.timezone_offset >= 0 ? "+" : "-",
           abs(dt.timezone_offset) / 3600);
}

void printDataEntry(const DataEntry* entry) {
    if (entry == NULL) {
        printf("Error: NULL DataEntry pointer\n");
        return;
    }

    printf("\nData Entry Details:\n");
    printf("----------------\n");
    printf("Timestamp (dt):         %ld\n", entry->dt);
    printf("ISO DateTime:           ");
    printDateTimeISO(entry->dt_iso);
    printf("Timezone:               %d seconds\n", entry->timezone);
    printf("City:                   %s\n", entry->city_name);
    printf("Location:               (%.6f, %.6f)\n", entry->lat, entry->lon);

    printf("\nTemperature:\n");
    printf("  Current:              %.2f\n", entry->temp);
    printf("  Feels Like:           %.2f\n", entry->feels_like);
    printf("  Min:                  %.2f\n", entry->temp_min);
    printf("  Max:                  %.2f\n", entry->temp_max);

    printf("\nWeather Conditions:\n");
    printf("  Visibility:           %d\n", entry->visibility);
    printf("  Dew Point:            %.2f\n", entry->dew_point);
    printf("  Pressure:             %d\n", entry->pressure);
    printf("  Humidity:             %d%%\n", entry->humidity);

    printf("\nWind:\n");
    printf("  Speed:                %.2f\n", entry->wind_speed);
    printf("  Direction:            %d degrees\n", entry->wind_deg);
    printf("  Gust:                 %.2f\n", entry->wind_gust);

    printf("\nPrecipitation:\n");
    printf("  Rain (1h):            %.2f\n", entry->rain_1h);
    printf("  Rain (3h):            %.2f\n", entry->rain_3h);
    printf("  Snow (1h):            %.2f\n", entry->snow_1h);
    printf("  Snow (3h):            %.2f\n", entry->snow_3h);

    printf("\nClouds:\n");
    printf("  Cloud Coverage:       %d%%\n", entry->clouds_all);

    printf("\nWeather Details:\n");
    printf("  ID:                   %d\n", entry->weather_id);
    printf("  Main:                 %s\n", entry->weather_main);
    printf("  Description:          %s\n", entry->weather_description);
    printf("  Icon:                 %s\n", entry->weather_icon);
}

DateTimeISO parse_datetime(const char *dt_iso) {
    DateTimeISO dt;
    sscanf(dt_iso, "%4d-%2d-%2d %2d:%2d:%2d %4d %s",
           &dt.year,
           &dt.month,
           &dt.day,
           &dt.hour,
           &dt.minute,
           &dt.second,
           &dt.timezone_offset,
           dt.timezone);

    // If the timezone is in the format "UTC" or something like "GMT+0200", we need to process it separately
    if (strstr(dt.timezone, "UTC") != NULL || strstr(dt.timezone, "GMT") != NULL) {
        // Handling the timezone part, e.g., UTC or GMT
        sscanf(dt_iso + 19, "%s", dt.timezone); // Extracts the abbreviation (UTC, GMT, etc.)
    }

    return dt; // Return the parsed DateTimeISO structure
}

int splitLineAdvanced(char *line, char fields[][MAX_FIELD_LENGTH], DataEntry *entries, int max_entries) {
    int field_count = 0;
    int start = 0;
    int inside_quotes = 0;

    for (int i = 0; line[i] != '\0' && field_count < MAX_FIELDS; i++) {
        if (line[i] == '"') {
            inside_quotes = !inside_quotes;
        } else if (line[i] == ',' && !inside_quotes) {
            int len = i - start;
            if (len >= MAX_FIELD_LENGTH) {
                len = MAX_FIELD_LENGTH - 1;
            }

            strncpy(fields[field_count], &line[start], len);

            fields[field_count][len] = '\0';
            field_count++;
            start = i + 1;
        }
    }

    if (field_count < MAX_FIELDS) {
        int len = strlen(line) - start;
        if (len >= MAX_FIELD_LENGTH) {
            len = MAX_FIELD_LENGTH - 1;
        }

        strncpy(fields[field_count], &line[start], len);

        fields[field_count][len] = '\0';
        field_count++;
    }

    return field_count;
}

void process_field(char *field) {
    int len = strlen(field);
    if (len >= 2 && field[0] == ',' && field[len - 1] == '"') {
        memmove(field, field + 1, len - 2);
        field[len - 2] = '\0';
        len -= 2;

        int j = 0;
        for (int i = 0; i < len; i++) {
            if (field[i] == '"' && field[i + 1] == '"') {
                field[j++] = '"';
                i++;
            } else {
                field[j++] = field[i];
            }
        }
        field[j] = '\0';
    }
}


int main() {
    int max_entries = 100;
    DataEntry entries[max_entries];

    // ----- opening the input file -----
    const char *filename = "inputData/Test.csv";
    FILE *csv_file = fopen(filename, "r");
    if (!csv_file) {
        perror("Error opening the file!");
        return 1;
    }

    // ----- going line by line -----
    char line[1024];
    int k = 0;
    while (fgets(line, sizeof(line), csv_file)) {
        line[strcspn(line, "\n")] = '\0';
        char fields[MAX_FIELDS][MAX_FIELD_LENGTH];

//        int count = splitLineAdvanced(line, fields, entries, max_entries);

        entries[k].dt = atol(fields[0]);                       // Convert to long
        entries[k].dt_iso = parse_datetime(fields[1]);  // Convert to DateTimeISO
        entries[k].timezone = atoi(fields[2]);                 // Convert to int
        strcpy(entries[k].city_name, fields[3]);               // Copy city name string
        entries[k].lat = atof(fields[4]);                      // Convert to double
        entries[k].lon = atof(fields[5]);                      // Convert to double
        entries[k].temp = atof(fields[6]);                     // Convert to float
        entries[k].visibility = atoi(fields[7]);               // Convert to int
        entries[k].dew_point = atof(fields[8]);                // Convert to float
        entries[k].feels_like = atof(fields[9]);               // Convert to float
        entries[k].temp_min = atof(fields[10]);                // Convert to float
        entries[k].temp_max = atof(fields[11]);                // Convert to float
        entries[k].pressure = atoi(fields[12]);                // Convert to int
        entries[k].sea_level = atoi(fields[13]);               // Convert to int
        entries[k].grnd_level = atoi(fields[14]);              // Convert to int
        entries[k].humidity = atoi(fields[15]);                // Convert to int
        entries[k].wind_speed = atof(fields[16]);              // Convert to float
        entries[k].wind_deg = atoi(fields[17]);                // Convert to int
        entries[k].wind_gust = atof(fields[18]);               // Convert to float
        entries[k].rain_1h = atof(fields[19]);                 // Convert to float
        entries[k].rain_3h = atof(fields[20]);                 // Convert to float
        entries[k].snow_1h = atof(fields[21]);                 // Convert to float
        entries[k].snow_3h = atof(fields[22]);                 // Convert to float
        entries[k].clouds_all = atoi(fields[23]);              // Convert to int
        entries[k].weather_id = atoi(fields[24]);              // Convert to int
        strcpy(entries[k].weather_main, fields[25]);           // Copy string
        strcpy(entries[k].weather_description, fields[26]);    // Copy string
        strcpy(entries[k].weather_icon, fields[27]);           // Copy string

        k++;

    }

    // ----- printing elements for testing -----
    for (int i = 0; i < 100; i++) {
        printDataEntry(&entries[i]);
    }

    return 0;
}