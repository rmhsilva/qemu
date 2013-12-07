// Functions for the LFU replacement strategy
#include <stdio.h>
#include "cache.h"

// Main cache lookup function
static int lookup_lfu(cache_item_t *cache, uint32_t index, uint32_t tag,
                          unsigned int *mask, uint8_t n_indexes, int *idx_used)
{
    uint32_t i, unlocked_cnt=0, min_r_field=UINT32_MAX, lfu_idx=0;
    static unsigned int use_count = 0;

    if (use_count++ > 10000) {
        // TODO!!!!! Clear counters and stuff....
    }

    // cache lines array
    cache_item_t *lines[n_indexes];
    for (i=0; i<n_indexes; i++){
        lines[i] = &cache[index | mask[i]];
    }

    // Loop over all cache line indexes
    for (i=0; i<n_indexes; i++) {
        if (lines[i]->valid == 1) {  /* Line valid... */
            if (lines[i]->tag == tag) {  /* ... and correct tag */
                lines[i]->r_field++;
                *idx_used = i;
                return 0;
            }
            else if (!lines[i]->lock) {  /* ... and wrong tag and unlocked */
                unlocked_cnt++;
                if (lines[i]->r_field < min_r_field) { // Check if less used
                    min_r_field = lines[i]->r_field;
                    lfu_idx = i;
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
            lines[i]->r_field = 0;
            *idx_used = i;
            return -1;
        }
    }

    if (unlocked_cnt == 0) {  /* All lines valid and locked */
        printf("All cache lines locked for index %x\n", index);
    }
    else {  /* All lines are valid and some are unlocked */
        lines[lfu_idx]->tag = tag;
        lines[lfu_idx]->valid = 1;
        lines[lfu_idx]->r_field = 0;
        *idx_used = lfu_idx;
    }
    return -1;
}


/*****************************************************************************/
// Cache operations for LFU

static void
fetch_lock_lfu (cache_item_t *cache, uint32_t index, uint32_t tag,
                unsigned int *mask, uint8_t n_indexes)
{
    int idx_used;
    lookup_lfu(cache, index, tag, mask, n_indexes, &idx_used);

    if (idx_used)
        cache[idx_used].lock = 1;
}


/*****************************************************************************/
// Define the interface

cache_interface_t interface_lfu = {
    .lookup         = lookup_lfu,
    .invalidate     = simple_invalidate,
    .hit_invalidate = assoc_hit_invalidate,
    .fill_line      = lookup_lfu,
    .fetch_lock     = fetch_lock_lfu
};
