// Functions for the LRU replacement strategy

#include "qemu-common.h"
#include "cache.h"


/**
 * lru_update: Perform LRU update tasks. Called by replace_lru only.
 * @param **cache_lines  Array of cache item pointers to work on
 * @param item_to_update Index of the item that was hit
 * @param n_indexes      Number of items to work on
 */
static void
lru_update(cache_item_t **cache_lines, uint32_t item_to_update, uint8_t n_indexes)
{
    int i;
    uint32_t old_rfield = cache_lines[item_to_update]->r_field;
    if(old_rfield == n_indexes - 1)
        return;
    for(i = 0; i < n_indexes; i = i + 1)
    {
        if(cache_lines[i]->r_field > old_rfield)
            cache_lines[i]->r_field--;
    }
    cache_lines[item_to_update]->r_field = n_indexes - 1;
}


// Main cache lookup function
static uint8_t lookup_lru(cache_item_t *cache, uint32_t index, uint32_t tag,
                          unsigned int *mask, uint8_t n_indexes)
{
    uint32_t i, unlocked_cnt=0, min_r_field=(n_indexes-1), lru_idx=0;

    // cache lines array
    cache_item_t *lines[n_indexes];
    for (i=0; i<n_indexes; i++)
        lines[i] = &cache[index | mask[i]];

    // Loop over all cache line indexes
    for (i=0; i<n_indexes; i++) {
        if (lines[i]->valid == 1) {  /* Line valid... */
            if (lines[i]->tag == tag) {  /* ... and correct tag */
                lru_update(lines, i, n_indexes);
                return 0;
            }
            else if (!lines[i]->lock) {  /* ... and wrong tag and unlocked */
                unlocked_cnt++;
                if (lines[i]->r_field < min_r_field) { // Check if less used
                    min_r_field = lines[i]->r_field;
                    lru_idx = i;
                }
            }
        }
        else {  /* line invalid... */
            if (lines[i]->lock) {    /* ...and locked */
                fprintf(stderr, "*** Error: Line %x in way 0 invalid and locked!\n",
                    index | mask[i]);
                exit(1);
            }  /* ...and unlocked */
            lines[i]->tag = tag;
            lines[i]->valid = 1;
            lru_update(lines, i, n_indexes);
            return 1;
        }
    }

    if (unlocked_cnt == 0) {  /* All lines valid and locked */
        printf("All cache lines locked for index %x\n", index);
    }
    else {  /* All lines are valid and some are unlocked */
        lines[lru_idx]->tag = tag;
        lines[lru_idx]->valid = 1;
        lru_update(lines, lru_idx, n_indexes);
    }
    return 1;
}


/*****************************************************************************/
// Cache operations for LRU

static void invalidate_lru(cache_item_t *cache, uint32_t index, uint32_t tag)
{
    cache[index].valid = 0;
    cache[index].lock = 0;
    
    // TODO - rfield = 0
}

static void hit_invalidate_lru(cache_item_t *cache, uint32_t index, uint32_t tag)
{

}

static uint8_t fill_line_lru(cache_item_t *cache, uint32_t index, uint32_t tag)
{
    return 0;
}

static void fetch_lock_lru(cache_item_t *cache, uint32_t index, uint32_t tag)
{
    
}


/*****************************************************************************/
// Define the interface

cache_interface_t interface_lru = {
    .lookup         = lookup_lru,
    .invalidate     = invalidate_lru,
    .hit_invalidate = hit_invalidate_lru,
    .fill_line      = fill_line_lru,
    .fetch_lock     = fetch_lock_lru
};