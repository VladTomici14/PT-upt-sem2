#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Hash table configuration
#define HASH_TABLE_SIZE 10007  // Prime number for better distribution
#define MAX_M 5
#define MAX_N 20

// Hash table entry structure
typedef struct HashEntry {
    int m;
    int n;
    uint64_t result;
    struct HashEntry* next;
} HashEntry;

// Hash table structure
typedef struct {
    HashEntry* table[HASH_TABLE_SIZE];
    int total_insertions;
    int total_lookups;
    int cache_hits;
} HashTable;

// Statistics structure
typedef struct {
    int function_calls;
    int cache_hits;
    int cache_misses;
    double computation_time;
} AckermannStats;

// Function prototypes
uint64_t ackermann_basic(int m, int n, AckermannStats* stats);
uint64_t ackermann_memoized(int m, int n, HashTable* ht, AckermannStats* stats);
HashTable* create_hash_table(void);
void destroy_hash_table(HashTable* ht);
uint32_t hash_function(int m, int n);
uint64_t hash_lookup(HashTable* ht, int m, int n);
void hash_insert(HashTable* ht, int m, int n, uint64_t result);
void print_stats(AckermannStats* stats, const char* method);
void benchmark_comparison(void);
void demonstrate_growth(void);
double get_time_diff(clock_t start, clock_t end);

// Simple hash function combining m and n
uint32_t hash_function(int m, int n) {
    // Use a simple polynomial hash
    return ((uint32_t)m * 31 + (uint32_t)n) % HASH_TABLE_SIZE;
}

// Create and initialize hash table
HashTable* create_hash_table(void) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        fprintf(stderr, "Memory allocation failed for hash table\n");
        exit(1);
    }
    
    // Initialize all entries to NULL
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->table[i] = NULL;
    }
    
    ht->total_insertions = 0;
    ht->total_lookups = 0;
    ht->cache_hits = 0;
    
    return ht;
}

// Destroy hash table and free memory
void destroy_hash_table(HashTable* ht) {
    if (!ht) return;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry* entry = ht->table[i];
        while (entry) {
            HashEntry* temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
    free(ht);
}

// Look up value in hash table
uint64_t hash_lookup(HashTable* ht, int m, int n) {
    ht->total_lookups++;
    uint32_t index = hash_function(m, n);
    HashEntry* entry = ht->table[index];
    
    while (entry) {
        if (entry->m == m && entry->n == n) {
            ht->cache_hits++;
            return entry->result;
        }
        entry = entry->next;
    }
    
    return 0; // Not found (0 is not a valid Ackermann result for our inputs)
}

// Insert value into hash table
void hash_insert(HashTable* ht, int m, int n, uint64_t result) {
    uint32_t index = hash_function(m, n);
    
    // Check if already exists
    HashEntry* entry = ht->table[index];
    while (entry) {
        if (entry->m == m && entry->n == n) {
            entry->result = result; // Update existing
            return;
        }
        entry = entry->next;
    }
    
    // Create new entry
    HashEntry* new_entry = (HashEntry*)malloc(sizeof(HashEntry));
    if (!new_entry) {
        fprintf(stderr, "Memory allocation failed for hash entry\n");
        exit(1);
    }
    
    new_entry->m = m;
    new_entry->n = n;
    new_entry->result = result;
    new_entry->next = ht->table[index];
    ht->table[index] = new_entry;
    ht->total_insertions++;
}

// Basic Ackermann function (no memoization)
uint64_t ackermann_basic(int m, int n, AckermannStats* stats) {
    if (stats) stats->function_calls++;
    
    if (m == 0) {
        return n + 1;
    } else if (n == 0) {
        return ackermann_basic(m - 1, 1, stats);
    } else {
        return ackermann_basic(m - 1, ackermann_basic(m, n - 1, stats), stats);
    }
}

// Memoized Ackermann function
uint64_t ackermann_memoized(int m, int n, HashTable* ht, AckermannStats* stats) {
    if (stats) stats->function_calls++;
    
    // Check cache first
    uint64_t cached_result = hash_lookup(ht, m, n);
    if (cached_result > 0) {
        if (stats) stats->cache_hits++;
        return cached_result;
    }
    
    if (stats) stats->cache_misses++;
    
    uint64_t result;
    if (m == 0) {
        result = n + 1;
    } else if (n == 0) {
        result = ackermann_memoized(m - 1, 1, ht, stats);
    } else {
        result = ackermann_memoized(m - 1, 
                    ackermann_memoized(m, n - 1, ht, stats), ht, stats);
    }
    
    // Cache the result
    hash_insert(ht, m, n, result);
    return result;
}

// Calculate time difference in seconds
double get_time_diff(clock_t start, clock_t end) {
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

// Print statistics
void print_stats(AckermannStats* stats, const char* method) {
    printf("  %s:\n", method);
    printf("    Function calls: %d\n", stats->function_calls);
    printf("    Cache hits: %d\n", stats->cache_hits);
    printf("    Cache misses: %d\n", stats->cache_misses);
    printf("    Computation time: %.6f seconds\n", stats->computation_time);
    if (stats->function_calls > 0) {
        double hit_ratio = (double)stats->cache_hits / stats->function_calls * 100.0;
        printf("    Cache hit ratio: %.2f%%\n", hit_ratio);
    }
}

// Benchmark comparison between basic and memoized versions
void benchmark_comparison(void) {
    printf("=== Ackermann Function Benchmark ===\n\n");
    
    // Test cases (be careful with larger values!)
    int test_cases[][2] = {{1, 1}, {2, 2}, {3, 2}, {3, 3}, {3, 4}};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; i++) {
        int m = test_cases[i][0];
        int n = test_cases[i][1];
        
        printf("Computing A(%d, %d):\n", m, n);
        
        // Test memoized version
        HashTable* ht = create_hash_table();
        AckermannStats memo_stats = {0, 0, 0, 0.0};
        
        clock_t start = clock();
        uint64_t result_memo = ackermann_memoized(m, n, ht, &memo_stats);
        clock_t end = clock();
        memo_stats.computation_time = get_time_diff(start, end);
        
        printf("  Result: %llu\n", (unsigned long long)result_memo);
        print_stats(&memo_stats, "Memoized version");
        
        // Test basic version only for small inputs
        if (m <= 3 && n <= 3) {
            AckermannStats basic_stats = {0, 0, 0, 0.0};
            
            start = clock();
            uint64_t result_basic = ackermann_basic(m, n, &basic_stats);
            end = clock();
            basic_stats.computation_time = get_time_diff(start, end);
            
            print_stats(&basic_stats, "Basic version");
            printf("  Results match: %s\n", 
                   (result_memo == result_basic) ? "Yes" : "No");
            
            if (basic_stats.computation_time > 0 && memo_stats.computation_time > 0) {
                double speedup = basic_stats.computation_time / memo_stats.computation_time;
                printf("  Speedup: %.2fx\n", speedup);
            }
        }
        
        printf("  Hash table efficiency:\n");
        printf("    Total insertions: %d\n", ht->total_insertions);
        printf("    Total lookups: %d\n", ht->total_lookups);
        printf("    Hash table hits: %d\n", ht->cache_hits);
        
        destroy_hash_table(ht);
        printf("\n");
    }
}

// Demonstrate the rapid growth of Ackermann function
void demonstrate_growth(void) {
    printf("=== Ackermann Function Growth Demonstration ===\n\n");
    
    HashTable* ht = create_hash_table();
    
    printf("A(m, n) values:\n");
    printf("m\\n");
    for (int n = 0; n < 6; n++) {
        printf("%8d", n);
    }
    printf("\n");
    
    for (int m = 0; m < 5; m++) {
        printf("%d  ", m);
        for (int n = 0; n < 6; n++) {
            if ((m == 4 && n > 1) || m > 4) {
                printf("  TOO BIG");
            } else {
                uint64_t result = ackermann_memoized(m, n, ht, NULL);
                if (result > 999999) {
                    printf("  TOO BIG");
                } else {
                    printf("%8llu", (unsigned long long)result);
                }
            }
        }
        printf("\n");
    }
    
    destroy_hash_table(ht);
}

// Main demonstration function
int main(void) {
    printf("Ackermann Function with Dynamic Programming in C\n");
    printf("================================================\n\n");
    
    // Show some basic computations
    printf("=== Basic Computations ===\n");
    HashTable* ht = create_hash_table();
    
    int examples[][2] = {{0, 5}, {1, 3}, {2, 4}, {3, 3}};
    int num_examples = sizeof(examples) / sizeof(examples[0]);
    
    for (int i = 0; i < num_examples; i++) {
        int m = examples[i][0];
        int n = examples[i][1];
        uint64_t result = ackermann_memoized(m, n, ht, NULL);
        printf("A(%d, %d) = %llu\n", m, n, (unsigned long long)result);
    }
    
    destroy_hash_table(ht);
    printf("\n");
    
    // Show memoization benefits
    printf("=== Memoization Benefits ===\n");
    ht = create_hash_table();
    AckermannStats stats = {0, 0, 0, 0.0};
    
    clock_t start = clock();
    uint64_t result = ackermann_memoized(3, 3, ht, &stats);
    clock_t end = clock();
    stats.computation_time = get_time_diff(start, end);
    
    printf("A(3, 3) = %llu\n", (unsigned long long)result);
    printf("Total function calls: %d\n", stats.function_calls);
    printf("Cache hits: %d\n", stats.cache_hits);
    printf("Cache misses: %d\n", stats.cache_misses);
    printf("Hash table entries: %d\n", ht->total_insertions);
    printf("Computation time: %.6f seconds\n", stats.computation_time);
    
    if (stats.function_calls > 0) {
        double hit_ratio = (double)stats.cache_hits / stats.function_calls * 100.0;
        printf("Cache hit ratio: %.2f%%\n", hit_ratio);
    }
    
    destroy_hash_table(ht);
    printf("\n");
    
    // Demonstrate growth
    demonstrate_growth();
    printf("\n");
    
    // Run benchmarks
    benchmark_comparison();
    
    // Important notes
    printf("=== Important Notes ===\n");
    printf("• The Ackermann function grows extremely rapidly\n");
    printf("• A(4, 2) = 2^65536 - 3 (a number with ~20,000 digits!)\n");
    printf("• Even with memoization, large inputs will cause stack overflow\n");
    printf("• This implementation uses uint64_t, limiting maximum values\n");
    printf("• Hash table uses chaining for collision resolution\n");
    printf("• Memory is properly managed with create/destroy functions\n");
    
    return 0;
}