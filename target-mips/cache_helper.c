/*
 *  MIPS emulation helpers for qemu - Cache profiling helpers.
 *
 *  Copyright (c) 2004-2005 Jocelyn Mayer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include "cpu.h"
#include "helper.h"

/* Macros for displaying uint64t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


/*****************************************************************************/
/* GDP */

// I-cache utility functions:

/**
 * invalidate: mark a cache line as invalid and remove lock
 */
static inline void
invalidate (cache_item_t *cache, uint32_t index)
{
    cache[index].valid = 0;
    cache[index].lock = 0;
}

/**
 * fill_line: fill a specified cache line (mark it as valid etc)
 * @return       0: success, 1: error
 */
static inline uint8_t
fill_line (cache_item_t *cache, uint32_t index, uint32_t tag)
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
 * hit_invalidate: mark a line as invalid IF it is currently valid
 */
static inline void
hit_invalidate (cache_item_t *cache, uint32_t index, uint32_t tag)
{
    unsigned int current_tag = cache[index].tag;

    if((current_tag==tag) && (cache[index].valid==1)) {
        invalidate(cache, index);
    }
}

/**
 * fetch_lock: fill a cache line AND mark it as locked
 */
static inline void
fetch_lock (cache_item_t *cache, uint32_t index, uint32_t tag)
{
    fill_line(cache, index, tag);
    cache[index].lock = 1;
}

/**
 * lookup_cache: Lookup data in a cache (L1/L2)
 * @param  *cache     Pointer to the cache memory
 * @param  index      Cache line to use
 * @param  tag        Tag of required memory
 * @param  way_width  0: direct-mapped, 1: 2-way set-associative, 2: 4-way s.-a.
 * @return            0: hit, 1: miss
 */
inline uint8_t 
lookup_cache_dm (cache_item_t *cache, uint32_t index,
              uint32_t tag, unsigned int *mask)
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
        return 1;
    }
}

inline uint8_t 
lookup_cache_2w (cache_item_t *cache, uint32_t index,
              uint32_t tag, unsigned int *mask)
{
    if((cache[index].tag == tag && cache[index].valid == 1) ||
      (cache[index | mask[0]].tag == tag && cache[index | mask[0]].valid == 1))
        return 0;
    else {
        if(cache[index].lock == 0) {   
            cache[index].tag = tag;
            cache[index].valid = 1;
        }
        else {
            printf("Line Locked!!\n");
        }
        return 1;
    }
}

inline uint8_t 
lookup_cache_4w (cache_item_t *cache, uint32_t index,
              uint32_t tag, unsigned int *mask)
{
    if((cache[index].tag == tag && cache[index].valid == 1) ||
      (cache[index | mask[0]].tag == tag && cache[index | mask[0]].valid == 1) ||
      (cache[index | mask[1]].tag == tag && cache[index | mask[1]].valid == 1) ||
      (cache[index | mask[2]].tag == tag && cache[index | mask[2]].valid == 1))
        return 0;
    else {
        if(cache[index].lock == 0) {   
            cache[index].tag = tag;
            cache[index].valid = 1;
        }
        else {
            printf("Line Locked!!\n");
        }
        return 1;
    }
}

/*****************************************************************************/
// QEMU Helpers

#define DECODE_INDEX(addr,mask,offset) (((addr) >> (offset)) & (mask))
#define DECODE_TAG(addr,idx_width) ((addr) >> (idx_width))

#define DECODE_INDEX_L1D(addr) \
    DECODE_INDEX(addr, mips_cache_opts.d_index_mask >> mips_cache_opts.d_way_width, \
    mips_cache_opts.d_offset_width)
#define DECODE_INDEX_L1I(addr) \
    DECODE_INDEX(addr, mips_cache_opts.i_index_mask >> mips_cache_opts.i_way_width, \
    mips_cache_opts.i_offset_width)
#define DECODE_INDEX_L2(addr) \
    DECODE_INDEX(addr, mips_cache_opts.l2_index_mask >> mips_cache_opts.l2_way_width, \
    mips_cache_opts.l2_offset_width)
#define DECODE_TAG_L1D(addr) \
    DECODE_TAG(addr, mips_cache_opts.d_index_width+mips_cache_opts.d_offset_width)
#define DECODE_TAG_L1I(addr) \
    DECODE_TAG(addr, mips_cache_opts.i_index_width+mips_cache_opts.i_offset_width)
#define DECODE_TAG_L2(addr) \
    DECODE_TAG(addr, mips_cache_opts.l2_index_width+mips_cache_opts.l2_offset_width)

void helper_icache(CPUMIPSState *env, target_ulong pc_addr, unsigned int opcode)
{
    uint32_t idx_l1 = DECODE_INDEX_L1I(pc_addr);
    uint32_t tag_l1 = DECODE_TAG_L1I(pc_addr);
    uint32_t idx_l2, tag_l2;
    uint8_t miss_l2;
    
    uint8_t miss_l1 = lookup_cache_dm(env->cache->icache, idx_l1,
                        tag_l1, mips_cache_opts.i_way_mask);

    if (!miss_l1) {
        mips_cache_opts.i_hit_cnt[idx_l1]++;
    }
    else {
        mips_cache_opts.i_miss_cnt[idx_l1]++;

        if(mips_cache_opts.use_l2) {
            idx_l2 = DECODE_INDEX_L2(pc_addr);
            tag_l2 = DECODE_TAG_L2(pc_addr);
            miss_l2 = lookup_cache_dm(env->cache->l2cache, idx_l2,
                        tag_l2, mips_cache_opts.l2_way_mask);
            if (!miss_l2)
                mips_cache_opts.l2_hit_cnt[idx_l2]++;
            else
                mips_cache_opts.l2_miss_cnt[idx_l2]++;
        }
    }
}


/**
 * This is called by the two d-cache helpers:
 */
static inline void 
helper_dcache (CPUMIPSState *env, target_ulong addr, int is_load) 
{
    /* if is_load is 0 then it is a store instruction */
#ifndef CONFIG_USER_ONLY 
    hwaddr phys_address = (hwaddr)get_phys_addr_cache(env, addr);
#else
    hwaddr phys_address = (hwaddr)addr;
#endif

    uint32_t idx_l1 = DECODE_INDEX_L1D(phys_address);
    uint32_t tag_l1 = DECODE_TAG_L1D(phys_address);
    uint32_t idx_l2, tag_l2;
    uint8_t miss_l2;

    uint8_t miss_l1 = lookup_cache_dm(env->cache->dcache, idx_l1,
                        tag_l1, mips_cache_opts.d_way_mask);
    
    // TODO: dirty bit etc for write-back?
    
    if (!miss_l1) {
        if (is_load)
            mips_cache_opts.d_ld_hit_cnt[idx_l1]++;
        else {
            mips_cache_opts.d_st_hit_cnt[idx_l1]++;
        }
    }
    else {
        if (is_load)
            mips_cache_opts.d_ld_miss_cnt[idx_l1]++;
        else
            mips_cache_opts.d_st_miss_cnt[idx_l1]++;
        if(mips_cache_opts.use_l2) {
            idx_l2 = DECODE_INDEX_L2(phys_address);
            tag_l2 = DECODE_TAG_L2(phys_address);
            miss_l2 = lookup_cache_dm(env->cache->l2cache, idx_l2,
                        tag_l2, mips_cache_opts.l2_way_mask);
            if (!miss_l2)
                mips_cache_opts.l2_hit_cnt[idx_l2]++;
            else
                mips_cache_opts.l2_miss_cnt[idx_l2]++;
        }
    }
}

void helper_dcache_ld(CPUMIPSState *env, target_ulong addr) {
    helper_dcache(env, addr, 1);
}
void helper_dcache_st(CPUMIPSState *env, target_ulong addr) {
    helper_dcache(env, addr, 0);
}


// XXX Must add d-cache helpers like the i-cache helpers below

void helper_cache_invalidate_i(CPUMIPSState *env, unsigned int addr)
{
    invalidate(env->cache->icache, DECODE_INDEX_L1I(addr));
}

void helper_cache_load_tag_i(CPUMIPSState *env, unsigned int addr)
{
    // TODO - what about TagHi?
    env->CP0_TagLo = env->cache->icache[DECODE_INDEX_L1I(addr)].tag;
}

void helper_cache_store_tag_i(CPUMIPSState *env, unsigned int addr)
{
    env->cache->icache[DECODE_INDEX_L1I(addr)].tag = env->CP0_TagLo;
}

void helper_cache_hit_invalidate_i(CPUMIPSState *env, unsigned int addr)
{
    hit_invalidate(env->cache->icache, DECODE_INDEX_L1I(addr), DECODE_TAG_L1I(addr));
}

void helper_cache_fill_i(CPUMIPSState *env, unsigned int addr)
{
    fill_line(env->cache->icache, DECODE_INDEX_L1I(addr), DECODE_TAG_L1I(addr));
}

void helper_cache_fetch_lock_i(CPUMIPSState *env, unsigned int addr)
{
    fetch_lock(env->cache->icache, DECODE_INDEX_L1I(addr), DECODE_TAG_L1I(addr));
}
