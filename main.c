#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stddef.h>

#define BIN_FILE "weather.bin"

#define MAGIC "WBIN"
#define VERSION 1.0
#define CITY_NAME_LEN 50
#define DESC_LEN 30

// ---------------------------
// ----- DATA STRUCTURES -----
// ---------------------------
typedef struct {
    char magic[4];            // "WBIN"
    float version;            // Version number
    time_t timestamp;         // File creation time
    int32_t record_count;     // Number of records
    char city[CITY_NAME_LEN]; // City Name
    float lat, lon;           // Geographical coordinates
} FileHeader;

typedef struct {
    long dt;                        // Unix timestamp
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

void write_header(FILE *binFile, const char *city, float lat, float lon) {
    FileHeader header;
    strcpy(header.magic, MAGIC);
//    header.version = VERSION;
//    header.timestamp = time(NULL);
    header.record_count = 0;  // Will update later
    strncpy(header.city, city, CITY_NAME_LEN);
    header.lat = lat;
    header.lon = lon;

    fwrite(&header, sizeof(FileHeader), 1, binFile);
}

void appendRecord(FILE *binFile, DataEntry *entry) {
    fseek(binFile, 0, SEEK_END); // Move to end
    fwrite(entry, sizeof(DataEntry), 1, binFile);
}

void writeHeader(FILE *binFile, const char *city, float lat, float lon) {
    FileHeader header;
    strcpy(header.magic, MAGIC);
    header.version = VERSION;
    header.timestamp = time(NULL);
    header.record_count = 0;  // Will update later
    strncpy(header.city, city, CITY_NAME_LEN);
    header.lat = lat;
    header.lon = lon;

    fwrite(&header, sizeof(FileHeader), 1, binFile);
}

void updateRecordCount(FILE *binFile, int count) {
    fseek(binFile, offsetof(FileHeader, record_count), SEEK_SET);
    fwrite(&count, sizeof(int32_t), 1, binFile);
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
               &entry->dt,
               entry->dt_iso,
               &entry->timezone,
               entry->city_name,
               &entry->lat,
               &entry->lon,
               &entry->temp,
               &entry->visibility,
               &entry->dew_point,
               &entry->feels_like,
               &entry->temp_min,
               &entry->temp_max,
               &entry->pressure,
               &entry->humidity,
               &entry->wind_speed,
               &entry->wind_deg,
               &entry->wind_gust,
               &entry->rain_1h,
               &entry->rain_3h,
               &entry->snow_1h,
               &entry->snow_3h,
               &entry->clouds_all,
               &entry->weather_id,
               entry->weather_main,
               entry->weather_description,
               entry->weather_icon);

        (*numEntries)++;
    }

    fclose(file);
    return entries;
}

void writeBinaryFile(const char *binaryFilename, DataEntry *entries, int numEntries) {
    FILE *binFile = fopen(binaryFilename, "wb");
    if (!binFile) {
        perror("Error opening binary file");
        return;
    }

    // Write file header
    FileHeader header;
    strcpy(header.magic, "WBIN");
    header.version = 1.0;
    header.timestamp = time(NULL);
    header.record_count = numEntries;
    fwrite(&header, sizeof(FileHeader), 1, binFile);

    // Write weather records
    for (int i = 0; i < numEntries; i++) {
        DataEntry record;

        // Copy data from DataEntry to WeatherRecord
        record.dt = entries[i].dt;
        record.timezone = entries[i].timezone;
        strncpy(record.city_name, entries[i].city_name, CITY_NAME_LEN);
        record.lat = entries[i].lat;
        record.lon = entries[i].lon;
        record.temp = (int16_t)(entries[i].temp * 10); // Convert to int16 (store with 1 decimal place)
        record.humidity = entries[i].humidity;
        record.pressure = entries[i].pressure;
        record.wind_speed = (int16_t)(entries[i].wind_speed * 10); // Convert to int16
        record.wind_deg = entries[i].wind_deg;
        record.clouds_all = entries[i].clouds_all;
        record.weather_id = entries[i].weather_id;
        strncpy(record.weather_main, entries[i].weather_main, DESC_LEN);
        strncpy(record.weather_description, entries[i].weather_description, DESC_LEN);
//        strncpy(record.weather_icon, entries[i].weather_icon, ICON_LEN);

        fwrite(&record, sizeof(DataEntry), 1, binFile);
    }

    fclose(binFile);
    printf("Binary file written successfully!\n");
}

void readBinaryFile() {
    FILE *binFile = fopen(BIN_FILE, "rb");
    if (!binFile) {
        perror("Error opening binary file");
        return;
    }

    // Read and display file header
    FileHeader header;
    fread(&header, sizeof(FileHeader), 1, binFile);

    printf("File Header:\n");
    printf("Magic: %s\nVersion: %.1f\nCreated: %sRecords: %d\nCity: %s (%.2f, %.2f)\n",
           header.magic, header.version, ctime(&header.timestamp),
           header.record_count, header.city, header.lat, header.lon);

    // Read records
    DataEntry entry;
    printf("\nWeather Records:\n");
    for (int i = 0; i < header.record_count; i++) {
        fread(&entry, sizeof(DataEntry), 1, binFile);
//        printf("Timestamp: %ld, Temp: %.1f°C, Pressure: %d hPa, Humidity: %d%%, Desc: %s\n",
//               entry.timestamp, record.temp / 10.0, record.pressure, record.humidity, record.description);
    }

    fclose(binFile);
}


int main() {

    int current_choice = -1;
    while(current_choice != 0) {
        printf("\n1. Convert CSV to Binary\n2. Read Records\n0. Exit\n");
        scanf("%d", &current_choice);

        if (current_choice == 1) {
            // --- converting csv to binary ---
            int numEntries;

//            printf("[CSV 2 BINARY] Enter a file path: ");
//            scanf("%s", filename);

            DataEntry *entries = readCSVFile("inputData/test.csv", &numEntries);

            printf("da");

            if (entries) {
                writeBinaryFile("weather.bin", entries, numEntries);
                free(entries);
            } else {
                printf("Error: Could not read CSV file.\n");
            }



        } else if (current_choice == 2) {
            // --- reading records ---
            readBinaryFile();

        } else if (current_choice == 0) {
            // --- exiting ---

            break;
        } else {
            printf("\n[ACTION] Exiting the program...\n");
        }


    }

    return 0;
}