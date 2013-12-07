#include <stdio.h>
#include "cache.h"

/**
 * Direct mapped cache lookup function
 * Defined here because it's simple enough to not need its own file, and it's
 * the 'default' cache structure.
 */
int lookup_cache_dm(cache_item_t *cache, uint32_t index, uint32_t tag,
                    unsigned int *mask, uint8_t n_indexes, int *idx_used)
{
    if(cache[index].tag == tag && cache[index].valid == 1)
        return 0;
    else {
        if(cache[index].lock == 0) {   
            cache[index].tag = tag;
            cache[index].valid = 1;
        }
        else {
            printf("Line Locked!!\n");
        }
        return -1;
    }
}


/*****************************************************************************/
// 'Default' (simple) cache operation functions

/**
 * invalidate: mark a cache line as invalid and remove lock
 */
void simple_invalidate (cache_item_t *cache, uint32_t index, uint32_t tag)
{
    cache[index].valid = 0;
    cache[index].lock = 0;
    cache[index].r_field = 0;
}

/**
 * hit_invalidate: mark a line as invalid IF it is currently valid
 */
void simple_hit_invalidate (cache_item_t *cache, uint32_t index, uint32_t tag,
                            unsigned int *mask, uint8_t n_indexes)
{
    unsigned int current_tag = cache[index].tag;

    if((current_tag==tag) && (cache[index].valid==1)) {
        cache[index].valid = 0;
        cache[index].lock = 0;
    }
}

/**
 * fill_line: fill a specified cache line (mark it as valid etc)
 * @return       0: success, 1: error
 */
int simple_fill_line (cache_item_t *cache, uint32_t index, uint32_t tag,
                      unsigned int *mask, uint8_t n_indexes, int *idx_used)
{
    if (cache[index].lock == 0) {
        cache[index].tag = tag;
        cache[index].valid = 1;
        return 0;
    }
    else {
        printf("Line Locked: %x\n", index);
        return 1;
    }
}

/**
 * fetch_lock: fill a cache line AND mark it as locked
 */
void simple_fetch_lock (cache_item_t *cache, uint32_t index, uint32_t tag,
                        unsigned int *mask, uint8_t n_indexes)
{
    if (cache[index].lock == 0) {
        cache[index].tag = tag;
        cache[index].valid = 1;
        cache[index].lock = 1;
    }
    else {
        printf("Line Locked: %x\n", index);
    }
}


/*****************************************************************************/
/* Associative caches share some functionality too */

/**
 * Look in all possible Ways for tag.
 * If found, invalidate that line.
 */
void assoc_hit_invalidate (cache_item_t *cache, uint32_t index, uint32_t tag,
                           unsigned int *mask, uint8_t n_indexes)
{
    uint32_t i;

    // cache lines array
    cache_item_t *lines[n_indexes];
    for (i=0; i<n_indexes; i++)
        lines[i] = &cache[index | mask[i]];

    // Loop over all cache line indexes
    for (i=0; i<n_indexes; i++) {
        if (lines[i]->valid == 1) {  /* Line valid... */
            if (lines[i]->tag == tag) {  /* ... and correct tag */
                lines[i]->valid = 0;
                lines[i]->lock = 0;
                lines[i]->r_field = 0;
            }
        }
    }
}


/*****************************************************************************/
// Define the interface

cache_interface_t interface_dm = {
    .lookup         = lookup_cache_dm,
    .invalidate     = simple_invalidate,
    .hit_invalidate = simple_hit_invalidate,
    .fill_line      = simple_fill_line,
    .fetch_lock     = simple_fetch_lock
};
