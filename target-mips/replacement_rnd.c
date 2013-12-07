// Functions for the random replacement strategy
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"

static inline uint8_t random_number(uint8_t n_indexes)
{
    return rand() / (RAND_MAX / n_indexes + 1);
}


// Main cache lookup function
static int lookup_rnd(cache_item_t *cache, uint32_t index, uint32_t tag,
                          unsigned int *mask, uint8_t n_indexes, int *idx_used)
{
    uint32_t i, unlocked_cnt=0;
    uint8_t rnd_no, unlocked_lines[n_indexes];

    // cache lines array
    cache_item_t *lines[n_indexes];
    for (i=0; i<n_indexes; i++) {
        lines[i] = &cache[index | mask[i]];
    }

    // Loop over all cache line indexes
    for (i=0; i<n_indexes; i++) {
        if (lines[i]->valid == 1) {  /* Line valid... */
            if (lines[i]->tag == tag) {  /* ... and correct tag */
                *idx_used = i;
                return 0;
            }
            else if (!lines[i]->lock) {  /* ... and wrong tag and unlocked */
                unlocked_lines[unlocked_cnt++] = i;
            }
        }
        else {  /* line invalid... */
            if (lines[i]->lock) {    /* ...and locked */
                fprintf(stderr, "*** Error: Line %x in way %u invalid and locked!\n",
                    index | mask[i], i);
                exit(1);
            }  /* ...and unlocked */
            lines[i]->tag = tag;
            lines[i]->valid = 1;
            *idx_used = i;
            return -1;
        }
    }
    rnd_no = random_number(n_indexes);
    if (unlocked_cnt == n_indexes) {  /* All lines valid and all unlocked */
        lines[rnd_no]->tag = tag;
        lines[rnd_no]->valid = 1;
        *idx_used = rnd_no;
    }
    else if(unlocked_cnt == 0){
        printf("All cache lines locked for index %x\n", index);
    }
    else if(unlocked_cnt == 1){ /* Only one line unlocked */
        lines[unlocked_lines[0]]->tag = tag;
        lines[unlocked_lines[0]]->valid = 1;
        *idx_used = unlocked_lines[0];
    }
    else {  /* All lines are valid and some are unlocked */
        do {
            for(i = 0; i < unlocked_cnt; i++) {
                if(rnd_no == unlocked_lines[i]) {
                    lines[rnd_no]->tag = tag;
                    lines[rnd_no]->valid = 1;
                    unlocked_cnt = 0;
                    break;
                }
            }
            if(unlocked_cnt == 0)
                break;
            rnd_no = random_number(n_indexes);
        } while(1);
        *idx_used = rnd_no;
    }
    return -1;
}


/*****************************************************************************/
// Cache operations for Rand

static void
fetch_lock_rnd (cache_item_t *cache, uint32_t index, uint32_t tag,
                unsigned int *mask, uint8_t n_indexes)
{
    int idx_used;
    lookup_rnd(cache, index, tag, mask, n_indexes, &idx_used);

    if (idx_used)
        cache[idx_used].lock = 1;
}


/*****************************************************************************/
// Define the interface

cache_interface_t interface_rnd = {
    .lookup         = lookup_rnd,
    .invalidate     = simple_invalidate,
    .hit_invalidate = assoc_hit_invalidate,
    .fill_line      = lookup_rnd,
    .fetch_lock     = fetch_lock_rnd
};
