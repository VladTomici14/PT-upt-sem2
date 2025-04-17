#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stddef.h>

#define MAGIC "WBIN"
#define CITY_NAME_LEN 50

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

// ----------------------------
// ----- PARSING FUNCTION -----
// ----------------------------
uint64_t parseDatetime(const char *datetime_str) {
    struct tm tm;
    strptime(datetime_str, "%Y-%m-%d %H:%M:%S", &tm);
    return (uint64_t) mktime(&tm);
}

// ---------------------------------
// ----- BINARY FILE FUNCTIONS -----
// ---------------------------------
void convertCsv2Binary(const char *csv_path, const char *bin_path) {
    FILE *CSV = fopen(csv_path, "r");
    FILE *BIN = fopen(bin_path, "wb");

    if (!CSV || !BIN) {
        perror("Error opening file");
        return;
    }

    FileHeader header = {
            .magic = MAGIC,
            .version = 1,
            .timestamp = (uint64_t) time(NULL),
            .record_count = 0,
            .lat = 45.7558f,
            .lon = 21.2322f
    };
    strcpy(header.city, "Timisoara");

    fwrite(&header, sizeof(FileHeader), 1, BIN);

    char line[1024];
    fgets(line, sizeof(line), CSV); // Skip header line
    while (fgets(line, sizeof(line), CSV)) {
        DataEntry entry;

        sscanf(line,
               "%ld,%63[^,],%d,%99[^,],%lf,%lf,%lf,%d,%lf,%lf,%lf,%lf,%d,%*[^,],%*[^,],%d,%lf,%d,%lf,%lf,%lf,%lf,%lf,%d,%d,%49[^,],%99[^,],%9[^\n]",
               &entry.dt,
               &entry.dt_iso,
               &entry.timezone,
               &entry.city_name,
               &entry.lat,
               &entry.lon,
               &entry.temp,
               &entry.visibility,
               &entry.dew_point,
               &entry.feels_like,
               &entry.temp_min,
               &entry.temp_max,
               &entry.pressure,
               &entry.humidity,
               &entry.wind_speed,
               &entry.wind_deg,
               &entry.wind_gust,
               &entry.rain_1h,
               &entry.rain_3h,
               &entry.snow_1h,
               &entry.snow_3h,
               &entry.clouds_all,
               &entry.weather_id,
               &entry.weather_main,
               &entry.weather_description,
               &entry.weather_icon);

        fwrite(&entry, sizeof(DataEntry), 1, BIN);

        header.record_count++;
    }

    fseek(BIN, 0, SEEK_SET); // Move to the beginning of the file
    fwrite(&header, sizeof(FileHeader), 1, BIN); // Write the header again with updated record count

    fclose(CSV);
    fclose(BIN);

    printf("Conversion complete. %d records written.\n", header.record_count);

}

void readFromBinary(const char *bin_path) {
    FILE *binFile = fopen(bin_path, "rb");
    if (!binFile) {
        perror("Error opening binary file");
        return;
    }

    // ----- reading and displaying the file header -----
    FileHeader header;
    fread(&header, sizeof(FileHeader), 1, binFile);

    printf("File Header:\n");
    printf("Magic: %s\nVersion: %.1f\nCreated: %sRecords: %d\nCity: %s (%.2f, %.2f)\n",
           header.magic, header.version, ctime(&header.timestamp),
           header.record_count, header.city, header.lat, header.lon);

    // ----- reading entries -----
    DataEntry *entries = malloc(header.record_count * sizeof(DataEntry));
    printf("\nWeather Records number: %d\n", header.record_count);
    for (int i = 0; i < header.record_count; i++) {
        fread(&entries[i], sizeof(DataEntry), 1, binFile);
    }

    fclose(binFile);
}

// -------------------------------
// ----- OPERATIONS FUNCTION -----
// -------------------------------
void searchByDateRange(const char *bin_path, time_t start_date, time_t end_date) {
    FILE *binFile = fopen(bin_path, "rb");
    if (!binFile) {
        perror("Error opening binary file");
        return;
    }

    FileHeader header;
    fread(&header, sizeof(FileHeader), 1, binFile);

//    printf("Searching for records between %s", ctime(&start_date));
//    printf("and %s\n", ctime(&end_date));

    int found_count = 0;
    DataEntry entry;

    for (int i = 0; i < header.record_count; i++) {
        fread(&entry, sizeof(DataEntry), 1, binFile);

        if (entry.dt >= start_date && entry.dt <= end_date) {
            found_count++;
            printf("Record #%d - Date: %s, Temp: %.1f°C\n",
                   i+1, entry.dt_iso, entry.temp);
        }
    }

    printf("\nTotal records found: %d\n", found_count);
    fclose(binFile);
}

int verifyFileIntegrity(const char *bin_path) {
    FILE *binFile = fopen(bin_path, "rb");
    if (!binFile) {
        perror("Error opening binary file");
        return 0;
    }

    FileHeader header;
    size_t readResult = fread(&header, sizeof(FileHeader), 1, binFile);
    if (readResult != 1) {
        printf("Error reading header\n");
        fclose(binFile);
        return 0;
    }

    // ----- checking the magic number -----
    if (strncmp(header.magic, MAGIC, 4) != 0) {
        printf("Invalid file format - magic number mismatch\n");
        fclose(binFile);
        return 0;
    }

    // ----- verify record count -----
    fseek(binFile, 0, SEEK_END);
    long fileSize = ftell(binFile);
    fseek(binFile, 0, SEEK_END);
    long expectedSize = sizeof(FileHeader) + header.record_count * sizeof(DataEntry);

    if (fileSize > expectedSize) {
        printf("File size mismatch. Expected: %ld, Actual: %ld\n", expectedSize, fileSize);
        fclose(binFile);
        return 0;
    }

    printf("File integrity verified. Format: %s, Version: %.1f, Records: %d\n",
           header.magic, header.version, header.record_count);

    fclose(binFile);
    return 1;
}

int main() {

    int current_choice = -1;

    char csv_path[128], bin_path[128];
    while (current_choice != 0) {
        printf("\n[MENU]\n");
        printf("1. Convert CSV to Binary\n");
        printf("2. Read Binary File\n");
        printf("3. Search by Date Range\n");
        printf("4. Verify file integrity\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &current_choice);

        if (current_choice == 1) {
            // ----- converting CSV to binary -----
            printf("Enter CSV file path: ");
            scanf("%s", csv_path);

            printf("Enter Binary file path: ");
            scanf("%s", bin_path);

            convertCsv2Binary(csv_path, bin_path);
        } else if (current_choice == 2) {
            // ----- reading binary file -----
            printf("Enter Binary file path: ");
            scanf("%s", bin_path);

            readFromBinary(bin_path);
        } else if (current_choice == 3) {
            // ----- searching by date range -----
            printf("Enter Binary file path: ");
            scanf("%s", bin_path);

            char start_date_str[20], end_date_str[20];
            printf("Enter start date (YYYY-MM-DD HH:MM:SS): ");
            scanf(" %19[^\n]", start_date_str);
            printf("Enter end date (YYYY-MM-DD HH:MM:SS): ");
            scanf(" %19[^\n]", end_date_str);

            time_t start_date = (time_t)parseDatetime(start_date_str);
            time_t end_date = (time_t)parseDatetime(end_date_str);

            searchByDateRange(bin_path, start_date, end_date);

        } else if (current_choice == 4) {
            // ----- verifying file integrity -----
            printf("Enter Binary file path: ");
            scanf("%s", bin_path);

            verifyFileIntegrity(bin_path);
        } else if (current_choice == 0) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
    }

    return 0;
}