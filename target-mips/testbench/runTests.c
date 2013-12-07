#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <glib.h>
#include <assert.h>

#include "../cache.h"
#include "../mips-cache-opts.h"

#define N_LINES 100

cache_item_t *icache;
cache_item_t *dcache;
cache_item_t *l2cache;


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

static void dump_cache(const char *fname, cache_item_t *cache, int n_lines) {
    int i;
    FILE *fd = fopen(fname, "w");

    if (fd) {
        for (i = 0; i < n_lines; ++i) {
            fprintf(fd, "%d, %x, rfield: %d, lock: %d, valid: %d\n", 
                i, cache[i].tag, (int)cache[i].r_field, (char)cache[i].lock, (char)cache[i].valid);
        }
        fclose(fd);
    }
    else printf("Error opening cache dump file...\n");
}

int main(int argc, char const *argv[])
{
    int idx, tag, result, line;
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
    idx = 10; tag = 0xbeef8eef;
    result = (*interface_lru.lookup)(icache, idx, tag,
            mips_cache_opts.i_way_mask, mips_cache_opts.i_ways, &line);
    printf("result: %d, line: %d\n", result, line);

    idx = 10; tag = 0xabcdef12;
    result = (*interface_lru.lookup)(icache, idx, tag,
            mips_cache_opts.i_way_mask, mips_cache_opts.i_ways, &line);
    printf("result: %d, line: %d\n", result, line);


    (*interface_lru.lookup)(icache, idx, tag, mips_cache_opts.i_way_mask, 
                            mips_cache_opts.i_ways, &line);

    // Dump the raw cache data
    dump_cache("icache-data.log", icache, mips_cache_opts.i_no_of_lines);
    dump_cache("dcache-data.log", dcache, mips_cache_opts.d_no_of_lines);
    dump_cache("l2cache-data.log", l2cache, mips_cache_opts.l2_no_of_lines);
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