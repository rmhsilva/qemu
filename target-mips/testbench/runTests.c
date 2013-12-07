/**
 * Runs some tests!
 *  1) Initialise the caches here.
 *  2) Run some tests here, OR
 *  3) Make the tests in another file, and declare them in runTests.h
 *
 * GDP 17, 2013
 */
#include "runTests.h"

cache_item_t *icache;
cache_item_t *dcache;
cache_item_t *l2cache;


// Initialises the cache memory
static void init_caches()
{
    //pass seed to random number generator
    srand(time(NULL));
    if (mips_cache_opts.use_i)
        icache = (cache_item_t *)g_malloc0(sizeof(cache_item_t)*mips_cache_opts.i_no_of_lines);
    if (mips_cache_opts.use_d)
        dcache = (cache_item_t *)g_malloc0(sizeof(cache_item_t)*mips_cache_opts.d_no_of_lines);
    if (mips_cache_opts.use_l2)
        l2cache = (cache_item_t *)g_malloc0(sizeof(cache_item_t)*mips_cache_opts.l2_no_of_lines);
}

// Dumps the cache memory into a file
void dump_cache(const char *fname, cache_item_t *cache, int n_lines)
{
    int i;
    FILE *fd = fopen(fname, "w");

    if (fd) {
        printf("Logging cache data to %s\n", fname);
        for (i = 0; i < n_lines; ++i) {
            fprintf(fd, "%d, %x, rfield: %d, lock: %d, valid: %d\n", 
                i, cache[i].tag, (int)cache[i].r_field, (char)cache[i].lock, (char)cache[i].valid);
        }
        fclose(fd);
    }
    else printf("Error opening cache dump file...\n");
}

// Prints a single line from a cache
void print_line(cache_item_t *cache, int line)
{
    printf("line: %8x | tag: %x, r_field: %d, valid: %d, lock: %d\n", 
        line, cache[line].tag, cache[line].r_field, cache[line].valid, cache[line].lock);
}


/**
 * Do all testing stuff in here
 */
int main(int argc, char const *argv[])
{
    printf("Welcome to the cache module testing environment...\n");

    // Create some caches...
    proc_mips_cache_opt('i', "4x256_2w_lru");
    // proc_mips_cache_opt('d', "4x256_2w_lru");
    // proc_mips_cache_opt('u', "4x16384_4w_rnd");
    init_caches();
    print_cache_info('i');
    // print_cache_info('d');
    // print_cache_info('u');

    // Do stuff with them!
    do_lru_tests();

    // Dump the raw cache data
    dump_cache("icache-data.log", icache, mips_cache_opts.i_no_of_lines);
    // dump_cache("dcache-data.log", dcache, mips_cache_opts.d_no_of_lines);
    // dump_cache("l2cache-data.log", l2cache, mips_cache_opts.l2_no_of_lines);
    return 0;
}



/*****************************************************************************/
// 'Polyfills' for pstring.h needed by mips-cache-opts.c

void pstrcpy(char *dest, int dest_buf_size, const char *src)
{
    strncpy(dest, src, dest_buf_size);
}

char *pstrcat(char *buf, int buf_size, const char *s)
{
    return strncat(buf, s, buf_size);
}