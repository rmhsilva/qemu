/**
 * Declare all tests in here.
 */
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

extern cache_item_t *icache;
extern cache_item_t *dcache;
extern cache_item_t *l2cache;

// Utility functions
void print_line(cache_item_t *cache, int line);
void dump_cache(const char *fname, cache_item_t *cache, int n_lines);


/*****************************************************************************/
/* Define tests here... */

void do_lru_tests();