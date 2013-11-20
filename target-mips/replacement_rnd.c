// Functions for the random replacement strategy

#include <stdlib.h>

#include "qemu-common.h"
#include "cache.h"

static inline uint8_t random_number(uint8_t n_indexes)
{
    return rand() / (RAND_MAX / n_indexes + 1);
}


// Main cache lookup function
static int lookup_rnd(cache_item_t *cache, uint32_t index, uint32_t tag,
                          unsigned int *mask, uint8_t n_indexes)
{
    uint32_t i, unlocked_cnt=0;
    uint8_t rnd_no, unlocked_lines[n_indexes];

    // cache lines array
    cache_item_t *lines[n_indexes];
    for (i=0; i<n_indexes; i++){
/*        fprintf(stderr,"Tag: %u, index %u contains this tag: %u\n", tag, index | mask[i],*/
/*            cache[index | mask[i]].tag);*/
        lines[i] = &cache[index | mask[i]];
    }

    // Loop over all cache line indexes
    for (i=0; i<n_indexes; i++) {
        if (lines[i]->valid == 1) {  /* Line valid... */
            if (lines[i]->tag == tag) {  /* ... and correct tag */
/*                fprintf(stderr,"Tag found: %u %u\n",tag,i);*/
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
/*            fprintf(stderr,"Invalid: %u %u\n",tag,i);*/
            lines[i]->tag = tag;
            lines[i]->valid = 1;
            return 1;
        }
    }
    rnd_no = random_number(n_indexes);
    if (unlocked_cnt == n_indexes) {  /* All lines valid and all unlocked */
/*        fprintf(stderr,"ReplacedA: %u %u\n",tag,rnd_no);*/
        lines[rnd_no]->tag = tag;
        lines[rnd_no]->valid = 1;
    }
    else if(unlocked_cnt == 0){
        printf("All cache lines locked for index %x\n", index);
    }
    else if(unlocked_cnt == 1){ /* Only one line unlocked */
/*        fprintf(stderr,"Replaced1: %u %u\n",tag,rnd_no);*/
        lines[unlocked_lines[0]]->tag = tag;
        lines[unlocked_lines[0]]->valid = 1;
    }
    else {  /* All lines are valid and some are unlocked */
        do {
            for(i = 0; i < unlocked_cnt; i++) {
                if(rnd_no == unlocked_lines[i]) {
                    lines[rnd_no]->tag = tag;
                    lines[rnd_no]->valid = 1;
                    unlocked_cnt = 0;
/*                    fprintf(stderr,"Replaced2: %u %u\n",tag,rnd_no);*/
                    break;
                }
            }
            if(unlocked_cnt == 0)
                break;
            rnd_no = random_number(n_indexes);
/*            fprintf(stderr,"New no needed\n");*/
        } while(1);
    }
    return 1;
}


/*****************************************************************************/
// Cache operations for Rand

static void
hit_invalidate_rnd (cache_item_t *cache, uint32_t index, uint32_t tag,
                    unsigned int *mask, uint8_t n_indexes)
{

}

static void
fetch_lock_rnd (cache_item_t *cache, uint32_t index, uint32_t tag,
                unsigned int *mask, uint8_t n_indexes)
{
    
}


/*****************************************************************************/
// Define the interface

cache_interface_t interface_rnd = {
    .lookup         = lookup_rnd,
    .invalidate     = simple_invalidate,
    .hit_invalidate = hit_invalidate_rnd,
    .fill_line      = lookup_rnd,
    .fetch_lock     = fetch_lock_rnd
};
