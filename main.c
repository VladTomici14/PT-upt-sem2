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

typedef struct {
    int year;                     // Year (e.g., 1979)
    int month;                    // Month (1-12)
    int day;                      // Day of the month (1-31)
    int hour;                     // Hour (0-23)
    int minute;                   // Minute (0-59)
    int second;                   // Second (0-59)
    int timezone_offset;          // Timezone offset in seconds (e.g., +0000 UTC -> 0, or +0200 -> 7200)
    char timezone[MAX_TIMEZONE_LENGTH]; // Timezone abbreviation (e.g., UTC)
} DateTimeISO;

typedef struct {
    long dt;                     // Timestamp (seconds since epoch)
    DateTimeISO dt_iso;   // ISO 8601 datetime structure
    int timezone;                // Timezone offset in seconds
    char city_name[MAX_CITY_NAME_LENGTH]; // City name (e.g., Timișoara)
    double lat;                   // Latitude
    double lon;                   // Longitude
    float temp;                   // Temperature (e.g., 9.11)
    int visibility;              // Visibility (e.g., 6000)
    float dew_point;             // Dew point (e.g., 9.11)
    float feels_like;            // Feels like temperature (e.g., 8.16)
    float temp_min;              // Minimum temperature (e.g., 8.12)
    float temp_max;              // Maximum temperature (e.g., 9.11)
    int pressure;                // Pressure (e.g., 991)
    int sea_level;               // Sea level pressure (optional, may be empty in data)
    int grnd_level;              // Ground level pressure (optional, may be empty in data)
    int humidity;                // Humidity percentage (e.g., 100)
    float wind_speed;            // Wind speed (e.g., 2.27)
    int wind_deg;                // Wind degree (e.g., 230)
    float wind_gust;             // Wind gust (optional, may be empty)
    float rain_1h;               // Rain in the last 1 hour (optional, may be empty)
    float rain_3h;               // Rain in the last 3 hours (optional, may be empty)
    float snow_1h;               // Snow in the last 1 hour (optional, may be empty)
    float snow_3h;               // Snow in the last 3 hours (optional, may be empty)
    int clouds_all;              // Cloud coverage percentage (e.g., 100)
    int weather_id;              // Weather condition ID (e.g., 500)
    char weather_main[MAX_WEATHER_DESCRIPTION_LENGTH];  // Main weather description (e.g., Rain)
    char weather_description[MAX_WEATHER_DESCRIPTION_LENGTH];  // Full weather description (e.g., light rain)
    char weather_icon[10];       // Weather icon (e.g., 10n)
} DataEntry;

void parse_datetime(const char *dt_iso, DateTimeISO *dt) {
    sscanf(dt_iso, "%4d-%2d-%2d %2d:%2d:%2d %4d %s",
           &dt->year,
           &dt->month,
           &dt->day,
           &dt->hour,
           &dt->minute,
           &dt->second,
           &dt->timezone_offset,
           dt->timezone);

    // If the timezone is in the format "UTC" or something like "GMT+0200", we need to process it separately
    if (strstr(dt->timezone, "UTC") != NULL || strstr(dt->timezone, "GMT") != NULL) {
        // Handling the timezone part, e.g., UTC or GMT
        sscanf(dt_iso + 19, "%s", dt->timezone); // Extracts the abbreviation (UTC, GMT, etc.)
    }
}

// --- function for printing one single data entry ---
void printDataEntry(DataEntry input_entry) {
    printf("Entry: City: %s, Temp: %.2f\n",
           input_entry.city_name,
           input_entry.temp);
}


// --- processing each line to the DataEntry ---
int process_csv(const char *filename, DataEntry *entries, int max_entries) {
    // opening the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int entry_count = 0;



    while (fgets(line, sizeof(line), file) != NULL) {
        if (entry_count >= max_entries) {
            break;
        }

        if (entry_count == 0) {
            entry_count++;
            continue;
        }

        DataEntry lineEntry;
//        printf("1%s\n", line);

        // FIXME: the reading is not done -- the line entry variable does not save any value at all

        char dt_str[20];
        sscanf(line,
               "%9s,%d,%99[^,],%lf,%lf,%f,%d,%f,%f,%f,%f,%d,%d,%d,%d,%f,%d,%f,%f,%f,%f,%f,%d,%d,%99[^,],%99[^,],%9s",
               dt_str,
               &lineEntry.timezone,
               lineEntry.city_name,
               &lineEntry.lat,
               &lineEntry.lon,
               &lineEntry.temp,
               &lineEntry.visibility,
               &lineEntry.dew_point,
               &lineEntry.feels_like,
               &lineEntry.temp_min,
               &lineEntry.temp_max,
               &lineEntry.pressure,
               &lineEntry.sea_level,
               &lineEntry.grnd_level,
               &lineEntry.humidity,
               &lineEntry.wind_speed,
               &lineEntry.wind_deg,
               &lineEntry.wind_gust,
               &lineEntry.rain_1h,
               &lineEntry.rain_3h,
               &lineEntry.snow_1h,
               &lineEntry.snow_3h,
               &lineEntry.clouds_all,
               &lineEntry.weather_id,
               lineEntry.weather_main,
               lineEntry.weather_description,
               lineEntry.weather_icon);

        printDataEntry(lineEntry);

        // converting the time entry string to a DateTimeISO variable
        parse_datetime(dt_str, &lineEntry.dt_iso);

        // storing the entry in the array
        entries[entry_count++] = lineEntry;
    }

    fclose(file);

    return entry_count;
}

int main() {
    DataEntry entries[100];

    const char *filename = "inputData/Timisoara.csv";

    // processing each line
    int num_entries = process_csv(filename, entries, 100);
    if (num_entries < 0) {
        return 1;
    }

    // testing the data entry by printing
    for (int i = 0; i < num_entries; i++) {
        printf("Entry %d: City: %s, Temp: %.2f, Date: %d-%02d-%02d %02d:%02d:%02d\n",
               i + 1, entries[i].city_name, entries[i].temp,
               entries[i].dt_iso.year, entries[i].dt_iso.month, entries[i].dt_iso.day,
               entries[i].dt_iso.hour, entries[i].dt_iso.minute, entries[i].dt_iso.second);

//        printf("%19s,%d,%99[^,],%lf,%lf,%f,%d,%f,%f,%f,%f,%f,%d,%d,%d,%d,%f,%d,%f,%f,%f,%f,%d,%d,%99[^,],%99[^,],%9s\n",
//               entries[i].dt,
//               &entries[i].timezone,
//               entries[i].city_name,
//               &entries[i].lat,
//               &entries[i].lon,
//               &entries[i].temp,
//               &entries[i].visibility,
//               &entries[i].dew_point,
//               &entries[i].feels_like,
//               &entries[i].temp_min,
//               &entries[i].temp_max,
//               &entries[i].pressure,
//               &entries[i].sea_level,
//               &entries[i].grnd_level,
//               &entries[i].humidity,
//               &entries[i].wind_speed,
//               &entries[i].wind_deg,
//               &entries[i].wind_gust,
//               &entries[i].rain_1h,
//               &entries[i].rain_3h,
//               &entries[i].snow_1h,
//               &entries[i].snow_3h,
//               &entries[i].clouds_all,
//               &entries[i].weather_id,
//               entries[i].weather_main,
//               entries[i].weather_description,
//               entries[i].weather_icon);
    }

    return 0;
}