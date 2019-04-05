    /* Name: Kiara Mutschler
 * CS login: mutschler
 * Section(s): Lecture 2
 *
 * csim.c - A cache simulator that can replay traces from Valgrind
 *     and output statistics such as number of hits, misses, and
 *     evictions.  The replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss plus a possible eviction.
 *  2. Instruction loads (I) are ignored.
 *  3. Data modify (M) is treated as a load followed by a store to the same
 *  address. Hence, an M operation can result in two cache hits, or a miss and a
 *  hit plus a possible eviction.
 *
 * The function print_summary() is given to print output.
 * Please use this function to print the number of hits, misses and evictions.
 * This is crucial for the driver to evaluate your work.
 */

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/****************************************************************************/
/***** DO NOT MODIFY THESE VARIABLE NAMES ***********************************/

/* Globals set by command line args */
int s = 0; /* set index bits */
int E = 0; /* associativity */
int b = 0; /* block offset bits */
int verbosity = 0; /* print trace if set */
char* trace_file = NULL;

/* Derived from command line args */
int B; /* block size (bytes) B = 2^b */
int S; /* number of sets S = 2^s In C, you can use the left shift operator */
  
/* Counters used to record cache statistics */
int hit_cnt = 0;
int miss_cnt = 0;
int evict_cnt = 0;
/*****************************************************************************/


/* Type: Memory address
 * Use this type whenever dealing with addresses or address masks
 */
typedef unsigned long long int mem_addr_t;

/* Type: Cache line
 * TODO
 *
 * NOTE:
 * You might (not necessarily though) want to add an extra field to this struct
 * depending on your implementation
 *
 * For example, to use a linked list based LRU,
 * you might want to have a field "struct cache_line * next" in the struct
 */
typedef struct cache_line {
    char valid;
    mem_addr_t tag;
    int counter;
} cache_line_t;

typedef cache_line_t* cache_set_t;
typedef cache_set_t* cache_t;

/* The cache we are simulating */
cache_t cache;

/* TODO - COMPLETE THIS FUNCTION
 * init_cache -
 * Allocate data structures to hold info regrading the sets and cache lines
 * use struct "cache_line_t" here
 * Initialize valid and tag field with 0s.
 * use S (= 2^s) and E while allocating the data structures here
 */

      void init_cache() {
  B = (1<<b);
  S = (1<<s);

  cache = malloc(S*sizeof(cache_set_t));
  if (cache == NULL) {
        printf("failure trying to allocate cache");
        exit(1);
  }
  for(int i = 0; i < S; i++){
        cache[i] = malloc(E*sizeof(cache_line_t));
        if(cache[i] == NULL){
          printf("failure trying to allocate sets");
          exit(1);
        }
        for(int j = 0; j < E; j++){
          (cache[i] + j)->valid = '0';
          (cache[i] + j)->tag = 0;
          (cache[i] + j)->next = NULL;
          //TODO: HOW DO I set the "next" of the prev line to this new one?
          //case 1: it is the beginning of a new set and a prev exists
          //case 2:
          //add list to it?
        }
  }
}


/* TODO - COMPLETE THIS FUNCTION
 * free_cache - free each piece of memory you allocated using malloc
 * inside init_cache() function
 */
void free_cache() {
   for(int i = 0; i < cache; i++){
        free(cache[i]);
   }
   free(cache);
   cache = NULL;
}

/* TODO - COMPLETE THIS FUNCTION
 * access_data - Access data at memory address addr.
 *   If it is already in cache, increase hit_cnit
 *   If it is not in cache, bring it in cache, increase miss count.
 *   Also increase evict_cnt if a line is evicted.
 *   you will manipulate data structures allocated in init_cache() here
 */
void access_data(mem_addr_t addr) {
       //find it t's in the cache
        //Case 1: it's a hit
        //hit_cnt++;
        //
        //TODO: do I need to recompute the s and b bits for this addr or is that made for us??
        mem_addr_t set = (addr/b)%s;
        mem_addr_t tag = (addr/b)/s;
        mem_addr_t lowestLine;
        //search for addr in the cache
        //
        for(int i = 0; i < E; i++){
             if(((*(cache+set) + i)->tag) == tag && (*(cache + set) + i)->valid == 1){
                //as if it's a 2D array and we get inside the cache's particular set's tag
                //then check if the valid bit of it is 1 (aka it's valid)
                //TODO: increment counter for that tag?
                hit_cnt++;
                return;
             }
        }




        //Case 2: it's a miss
        //miss_cnt++;
        //Case A; there's space left
        //look through and try to find if there are any valid bits that are
        //0 which means we have more space!
        for(int i = 0; i < E; i++){
            if(((*(cache+set) + i)->valid) == 0){
                (*(cache+set) + i)->tag = tag;
                (*(cache+set) + i)->valid = 1;
                miss_cnt++;
                return;
            }
        }

        //Case 2B: there is no space left and we have to evict
        //aka find line with the lowest counter

        for(int i = 0; i < E; i++){
        }
        //evict_cnt++;
//look for in cache, increase hit_cnt for that data
// if not in cache
//using LRU replacement policy
}

/* TODO - FILL IN THE MISSING CODE
 * replay_trace - replays the given trace file against the cache
 * reads the input trace file line by line
 * extracts the type of each memory access : L/S/M
 * YOU MUST TRANSLATE one "L" as a load i.e. 1 memory access
 * YOU MUST TRANSLATE one "S" as a store i.e. 1 memory access
 * YOU MUST TRANSLATE one "M" as a load followed by a store i.e. 2 memory accesses
 */
void replay_trace(char* trace_fn) {
    char buf[1000];
    mem_addr_t addr = 0;
    unsigned int len = 0;
    FILE* trace_fp = fopen(trace_fn, "r");

    if (!trace_fp) {
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }

    while (fgets(buf, 1000, trace_fp) != NULL) {
        if (buf[1] == 'S' || buf[1] == 'L' || buf[1] == 'M') {
            sscanf(buf+3, "%llx,%u", &addr, &len);

            if (verbosity)
                printf("%c %llx,%u ", buf[1], addr, len);

            // TODO - MISSING CODE
            // now you have:
            // 1. address accessed in variable - addr
            // 2. type of acccess(S/L/M)  in variable - buf[1]
            // call access_data function here depending on type of access
            //
            access_data(addr); //depending on type of access
            //TODO: idk what else I'm supposed to do lol

            if (verbosity)
                printf("\n");
        }
    }

    fclose(trace_fp);
}

/*
 * print_usage - Print usage info
 */
void print_usage(char* argv[]) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * print_summary - Summarize the cache simulation statistics. Student cache simulators
 *                must call this function in order to be properly autograded.
 */
void print_summary(int hits, int misses, int evictions) {
    printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
    fclose(output_fp);
}

/*
 * main - Main routine
 */
int main(int argc, char* argv[]) {
    char c;

    // Parse the command line arguments: -h, -v, -s, -E, -b, -t
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (c) {
            case 'b':
                b = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'h':
                print_usage(argv);
                exit(0);
            case 's':
                s = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
                break;
            case 'v':
                verbosity = 1;
                break;
            default:
                print_usage(argv);
                exit(1);
        }
    }

    /* Make sure that all required command line args were specified */
    if (s == 0 || E == 0 || b == 0 || trace_file == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        print_usage(argv);
        exit(1);
    }

    /* Initialize cache */
    init_cache();

    replay_trace(trace_file);

    /* Free allocated memory */
    free_cache();

    /* Output the hit and miss statistics for the autograder */
    print_summary(hit_cnt, miss_cnt, evict_cnt);
    return 0;
}
