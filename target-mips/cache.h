/**
 * Virtual Caches for QEMU
 *   GDP17, 2013, University of Southampton
 */

#ifndef GDP_CACHE
#define GDP_CACHE

#ifdef TARGET_MIPS

#ifndef UNCACHED_REGION
#define UNCACHED_REGION
#define UNCACHED_START  0xA0000000
#define UNCACHED_END    0xBFFFFFFF
#endif

#endif

#include <stdlib.h>
#include <stdint.h>


/**
 * Main Cache data structure
 */
typedef struct cache_item_t cache_item_t;
struct cache_item_t {
    uint_fast32_t tag         : 32;
    uint_fast32_t r_field     : 30;
    uint_fast8_t valid        : 1;
    uint_fast8_t lock         : 1;
};


/*****************************************************************************/
/**
 * For different replacement algorithms, there may be many differences
 * between cache functions, eg lookup, cache operations (prefetch, invalidate)
 * etc. So we create this interface that each replacement strategy must use.
 * Each strategy will have its own implementation of 'lookup_cache', for
 * example. It will use the cache_interface_t to point to each of its own
 * functions.
 *
 * For the cache operations (invalidate, etc) there are 'default' functions
 * which are declared in this file, which the other replacement strategies
 * may use instead of creating their own.
 */
typedef struct cache_interface_t cache_interface_t;
struct cache_interface_t {
    /**
     * lookup: The main cache lookup function. 
     * @param *cache     Pointer to the cache to operate on
     * @param  index     Index of memory access
     * @param  tag       Tag of memory accessed
     * @param *mask      Index mask (for set-associative caches)
     * @param  n_indexes Number of indexes (sets)
     * @param *idx_used  In S.A. caches, this is set to the line that was used
     * @return           +ve: hit, -1: miss
     */
    int (*lookup)(cache_item_t *cache, uint32_t index, uint32_t tag,
                  unsigned int *mask, uint8_t n_indexes, int *idx_used);
    /**
     * invalidate: mark a cache line as invalid and remove lock
     */
    void (*invalidate)(cache_item_t *cache, uint32_t index, uint32_t tag);
    /**
     * hit_invalidate: mark a line as invalid IF it is currently valid
     */
    void (*hit_invalidate) (cache_item_t *cache, uint32_t index, uint32_t tag,
                            unsigned int *mask, uint8_t n_indexes);
    /**
     * fill_line: fill a specified cache line (mark it as valid etc)
     * @return       0: success, 1: error
     */
    int (*fill_line) (cache_item_t *cache, uint32_t index, uint32_t tag,
                      unsigned int *mask, uint8_t n_indexes, int *idx_used);
    /**
     * fetch_lock: fill a cache line AND mark it as locked
     */
    void (*fetch_lock) (cache_item_t *cache, uint32_t index, uint32_t tag,
                        unsigned int *mask, uint8_t n_indexes);
};


/*****************************************************************************/
// CPU Cache context struct

typedef struct CPUCacheContext CPUCacheContext;
struct CPUCacheContext {
    cache_item_t *icache;
    cache_item_t *dcache;
    cache_item_t *l2cache;
    struct MipsCacheOpts *opts;
    cache_interface_t *icache_api;
    cache_interface_t *dcache_api;
    cache_interface_t *l2cache_api;
};


/*****************************************************************************/
// Helpers for decoding tags and indexes
#define DECODE_INDEX(addr,mask,offset) (((addr) >> (offset)) & (mask))
#define DECODE_TAG(addr,idx_width) ((addr) >> (idx_width))

#define DECODE_INDEX_d(addr) \
    DECODE_INDEX(addr, mips_cache_opts.d_index_mask >> mips_cache_opts.d_way_width, \
    mips_cache_opts.d_offset_width)
#define DECODE_INDEX_i(addr) \
    DECODE_INDEX(addr, mips_cache_opts.i_index_mask >> mips_cache_opts.i_way_width, \
    mips_cache_opts.i_offset_width)
#define DECODE_INDEX_l2(addr) \
    DECODE_INDEX(addr, mips_cache_opts.l2_index_mask >> mips_cache_opts.l2_way_width, \
    mips_cache_opts.l2_offset_width)
#define DECODE_TAG_d(addr) \
    DECODE_TAG(addr, mips_cache_opts.d_index_width+mips_cache_opts.d_offset_width)
#define DECODE_TAG_i(addr) \
    DECODE_TAG(addr, mips_cache_opts.i_index_width+mips_cache_opts.i_offset_width)
#define DECODE_TAG_l2(addr) \
    DECODE_TAG(addr, mips_cache_opts.l2_index_width+mips_cache_opts.l2_offset_width)

#define DECODE_INDEX_WAY_d(addr) \
    DECODE_INDEX(addr, mips_cache_opts.d_index_mask, mips_cache_opts.d_offset_width)
#define DECODE_INDEX_WAY_i(addr) \
    DECODE_INDEX(addr, mips_cache_opts.i_index_mask, mips_cache_opts.i_offset_width)
#define DECODE_INDEX_WAY_l2(addr) \
    DECODE_INDEX(addr, mips_cache_opts.l2_index_mask, mips_cache_opts.l2_offset_width)


/*****************************************************************************/
/**
 * More complex replacement strategies get their own file, but direct mapped
 * gets declared here. The interface is also declared here.
 */

// 'Default' (simple) cache operation functions
void simple_invalidate(cache_item_t *cache, uint32_t index, uint32_t tag);
void simple_hit_invalidate(cache_item_t *cache, uint32_t index, uint32_t tag, unsigned int *mask, uint8_t n_indexes);
int simple_fill_line(cache_item_t *cache, uint32_t index, uint32_t tag, unsigned int *mask, uint8_t n_indexes, int *idx_used);
void simple_fetch_lock(cache_item_t *cache, uint32_t index, uint32_t tag, unsigned int *mask, uint8_t n_indexes);

// Direct Mapped cache lookup
int lookup_cache_dm(cache_item_t *cache, uint32_t index, uint32_t tag,
                    unsigned int *mask, uint8_t n_indexes, int *idx_used);

// Interface to the Direct Mapped functionality
extern cache_interface_t interface_dm;


// Associative caches share some functionality
void assoc_hit_invalidate(cache_item_t *cache, uint32_t index, uint32_t tag, unsigned int *mask, uint8_t n_indexes);

/*****************************************************************************/
// Declare interfaces to the other replacement strategies we have
// These are implemented in other files

extern cache_interface_t interface_lru;
extern cache_interface_t interface_lfu;
extern cache_interface_t interface_rnd;


#endif // ifdef GDP_CACHE
