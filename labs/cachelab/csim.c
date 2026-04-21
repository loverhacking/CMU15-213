#include "cachelab.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 256

/*
 * One cache line stores three pieces of state:
 * 1. valid: whether this line already contains meaningful data
 * 2. tag:   the high-order address bits used to identify the block
 * 3. last_used: a logical timestamp for LRU replacement
 */
typedef struct {
    int valid;
    unsigned long long tag;
    unsigned long long last_used;
} cache_line_t;

/* A set is just an array of cache lines in an E-way associative cache. */
typedef struct {
    cache_line_t *lines;
} cache_set_t;

/*
 * The cache object keeps both the allocated storage and the configuration
 * derived from command-line arguments. "time" is incremented on every access
 * so we can implement LRU without needing a separate queue structure.
 */
typedef struct {
    cache_set_t *sets;
    size_t set_count;
    size_t set_bits;
    size_t lines_per_set;
    size_t block_bits;
    unsigned long long time;
} cache_t;

typedef enum {
    ACCESS_HIT,
    ACCESS_MISS,
    ACCESS_MISS_EVICTION
} access_result_t;

typedef struct {
    int hits;
    int misses;
    int evictions;
} sim_stats_t;

static void printUsage(const char *program_name);
static size_t parseSizeOption(const char *flag, const char *value);
static void parseArgs(int argc, char **argv, int *is_display, size_t *set_bits,
                      size_t *assoc, size_t *block_bits, const char **filename);
static cache_t initCache(size_t set_bits, size_t assoc, size_t block_bits);
static void freeCache(cache_t *cache);
static access_result_t accessCache(cache_t *cache, unsigned long long address);
static void updateStats(sim_stats_t *stats, access_result_t result);
static void replayTrace(FILE *file, cache_t *cache, int is_display,
                        sim_stats_t *stats);
static void printAccessResult(access_result_t result);

/*
 * Main flow:
 * 1. Parse the cache parameters and trace path.
 * 2. Build an in-memory cache with S sets and E lines per set.
 * 3. Replay each trace record in order.
 * 4. Update hit/miss/eviction counters based on the simulated accesses.
 * 5. Print the final summary in the exact format expected by Cache Lab.
 */
int main(int argc, char **argv)
{
    size_t set_bits = 0;
    size_t assoc = 0;
    size_t block_bits = 0;
    const char *filename = NULL;
    int is_display = 0;
    FILE *file;
    cache_t cache;
    sim_stats_t stats = {0, 0, 0};

    parseArgs(argc, argv, &is_display, &set_bits, &assoc, &block_bits, &filename);
    cache = initCache(set_bits, assoc, block_bits);

    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open trace file: %s\n", filename);
        freeCache(&cache);
        return EXIT_FAILURE;
    }

    replayTrace(file, &cache, is_display, &stats);

    fclose(file);
    freeCache(&cache);
    printSummary(stats.hits, stats.misses, stats.evictions);
    return 0;
}

/* Print a small help message matching the standard Cache Lab interface. */
static void printUsage(const char *program_name)
{
    printf("Usage: %s [-v] -s <num> -E <num> -b <num> -t <file>\n", program_name);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
}

/*
 * Parse a numeric command-line option and reject malformed values early.
 * Using strtoul lets us detect cases like empty strings or trailing junk.
 */
static size_t parseSizeOption(const char *flag, const char *value)
{
    char *endptr = NULL;
    unsigned long parsed = strtoul(value, &endptr, 10);

    if (value[0] == '\0' || *endptr != '\0') {
        fprintf(stderr, "Invalid value for -%s: %s\n", flag, value);
        exit(EXIT_FAILURE);
    }

    return (size_t)parsed;
}

/*
 * Read all command-line options needed by the simulator.
 * The lab requires:
 * - s: number of set index bits
 * - E: number of lines per set
 * - b: number of block offset bits
 * - t: trace filename
 * The optional -v flag enables per-access output.
 */
static void parseArgs(int argc, char **argv, int *is_display, size_t *set_bits,
                      size_t *assoc, size_t *block_bits, const char **filename)
{
    int option;
    int has_s = 0;
    int has_e = 0;
    int has_b = 0;
    int has_t = 0;

    while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (option) {
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'v':
                *is_display = 1;
                break;
            case 's':
                *set_bits = parseSizeOption("s", optarg);
                has_s = 1;
                break;
            case 'E':
                *assoc = parseSizeOption("E", optarg);
                has_e = 1;
                break;
            case 'b':
                *block_bits = parseSizeOption("b", optarg);
                has_b = 1;
                break;
            case 't':
                *filename = optarg;
                has_t = 1;
                break;
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    /* Cache Lab requires all four structural parameters to be provided. */
    if (!has_s || !has_e || !has_b || !has_t) {
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

/*
 * Allocate the cache as:
 *   cache.sets[set_index].lines[line_index]
 * Each line is zero-initialized by calloc, so valid bits start at 0.
 */
static cache_t initCache(size_t set_bits, size_t assoc, size_t block_bits)
{
    size_t set_index;
    cache_t cache;

    cache.set_count = 1ULL << set_bits;
    cache.set_bits = set_bits;
    cache.lines_per_set = assoc;
    cache.block_bits = block_bits;
    cache.time = 0;
    cache.sets = calloc(cache.set_count, sizeof(cache_set_t));
    if (cache.sets == NULL) {
        fprintf(stderr, "Unable to allocate cache sets.\n");
        exit(EXIT_FAILURE);
    }

    for (set_index = 0; set_index < cache.set_count; set_index++) {
        cache.sets[set_index].lines = calloc(assoc, sizeof(cache_line_t));
        if (cache.sets[set_index].lines == NULL) {
            fprintf(stderr, "Unable to allocate cache lines.\n");
            freeCache(&cache);
            exit(EXIT_FAILURE);
        }
    }

    return cache;
}

/* Release every per-set line array first, then release the set array itself. */
static void freeCache(cache_t *cache)
{
    size_t set_index;

    if (cache->sets == NULL) {
        return;
    }

    for (set_index = 0; set_index < cache->set_count; set_index++) {
        free(cache->sets[set_index].lines);
    }
    free(cache->sets);
    cache->sets = NULL;
}

/*
 * Simulate one cache access.
 *
 * Address layout:
 *   [ tag | set index | block offset ]
 *
 * We ignore the block offset because Cache Lab only cares whether the block
 * is present in the cache. The set index chooses which set to inspect, and
 * the tag distinguishes lines within that set.
 */
static access_result_t accessCache(cache_t *cache, unsigned long long address)
{
    size_t line_index;
    size_t replacement_index = 0;
    unsigned long long min_time = 0;
    unsigned long long set_mask = cache->set_count - 1;
    unsigned long long set_index = (address >> cache->block_bits) & set_mask;
    unsigned long long tag = address >> (cache->block_bits + cache->set_bits);
    cache_set_t *set = &cache->sets[set_index];

    cache->time++;

    /*
     * First pass: search the indexed set for an existing valid line whose tag
     * matches the requested address. On a hit we only need to refresh the LRU
     * timestamp and report success.
     */
    for (line_index = 0; line_index < cache->lines_per_set; line_index++) {
        cache_line_t *line = &set->lines[line_index];
        if (line->valid && line->tag == tag) {
            line->last_used = cache->time;
            return ACCESS_HIT;
        }
    }

    /*
     * Second pass: if there is an invalid line, the miss can fill that slot
     * directly with no eviction. Otherwise we keep track of the smallest
     * timestamp, which corresponds to the least recently used line.
     */
    for (line_index = 0; line_index < cache->lines_per_set; line_index++) {
        cache_line_t *line = &set->lines[line_index];
        if (!line->valid) {
            line->valid = 1;
            line->tag = tag;
            line->last_used = cache->time;
            return ACCESS_MISS;
        }

        if (line_index == 0 || line->last_used < min_time) {
            min_time = line->last_used;
            replacement_index = line_index;
        }
    }

    /* No empty line was available, so replace the LRU victim. */
    set->lines[replacement_index].tag = tag;
    set->lines[replacement_index].last_used = cache->time;
    return ACCESS_MISS_EVICTION;
}

/* Convert one access result into the summary counters reported by the lab. */
static void updateStats(sim_stats_t *stats, access_result_t result)
{
    if (result == ACCESS_HIT) {
        stats->hits++;
        return;
    }

    stats->misses++;
    if (result == ACCESS_MISS_EVICTION) {
        stats->evictions++;
    }
}

/*
 * Replay the full trace file and keep main focused on setup/teardown only.
 * This function owns the trace parsing loop and applies the Cache Lab rule
 * that a modify operation behaves like two accesses to the same block.
 */
static void replayTrace(FILE *file, cache_t *cache, int is_display,
                        sim_stats_t *stats)
{
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file) != NULL) {
        char op;
        unsigned long long address;
        int size;
        access_result_t result;

        /*
         * Trace lines look like:
         *   " M 20,1"
         *   " L 10,4"
         * The address is hexadecimal, so we must parse it with %llx.
         */
        if (sscanf(line, " %c %llx,%d", &op, &address, &size) != 3) {
            continue;
        }

        /* "I" means instruction load and should not affect the data cache. */
        if (op == 'I') {
            continue;
        }

        if (is_display) {
            printf("%c %llx,%d", op, address, size);
        }

        /*
         * L and S each correspond to one cache access.
         * M is special: a modify is treated as a load followed by a store,
         * so it always performs two accesses to the same address.
         */
        result = accessCache(cache, address);
        updateStats(stats, result);
        if (is_display) {
            printAccessResult(result);
        }

        if (op == 'M') {
            result = accessCache(cache, address);
            updateStats(stats, result);
            if (is_display) {
                printAccessResult(result);
            }
        }

        if (is_display) {
            printf("\n");
        }
    }
}

/* Verbose mode prints the textual outcome expected by the reference tool. */
static void printAccessResult(access_result_t result)
{
    if (result == ACCESS_HIT) {
        printf(" hit");
    } else if (result == ACCESS_MISS) {
        printf(" miss");
    } else {
        printf(" miss eviction");
    }
}
