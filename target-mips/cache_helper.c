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

#define DECODE_INDEX(addr,mask,offset) (((addr) >> (offset)) & (mask))
#define DECODE_TAG(addr,idx_width) ((addr) >> (idx_width))

// I-cache utility functions:

static inline void invalidate(CPUMIPSState *env, unsigned int address)
{
    // unsigned int ind = DECODE_INDEX(address);
    // env->cache->icache[ind].valid = 0;
    // env->cache->icache[ind].lock = 0;
}


static inline void fill_line(CPUMIPSState *env, unsigned int address)
{
    // unsigned int index_val = DECODE_INDEX(address);

    // if (env->cache->icache[index_val].lock == 0) {
    //     env->cache->icache[index_val].tag = DECODE_TAG(address);
    //     env->cache->icache[index_val].valid = 1;
    // }
    // else {
    //     printf("Line Locked: %x\n", index_val);
    // }
}


/**
 * access_cache: An interface to either a data cache or instruction cache.
 * @param  *cache  Pointer to the cache memory
 * @param  index   Cache line to use
 * @param  tag     Tag of required memory
 * @return         0: miss, 1: hit
 */
static inline uint8_t 
access_cache(cache_item_t *cache, uint32_t index, uint32_t tag)
{
    uint32_t current_tag = cache[index].tag;
    
    if ((current_tag==tag) && (cache[index].valid==1)) {
        return 1;
    }
    else {
        if(cache[index].lock == 0) {   
            cache[index].tag = tag;
            cache[index].valid = 1;
        }
        else {
            printf("Line Locked!!\n");
        }
        return 0;
    }
}


static inline void hit_invalidate(CPUMIPSState *env, unsigned int address)
{
    // unsigned int tag_val = DECODE_TAG(address);
    // unsigned int index_val = DECODE_INDEX(address);
    // unsigned int taglo = env->cache->icache[index_val].tag;

    // if((taglo==tag_val) && (env->cache->icache[index_val].valid==1)) {
    //     invalidate(env, address);
    // }
}


static inline void fetch_lock(CPUMIPSState *env, unsigned int address)
{
    // fill_line(env, address);
    // env->cache->icache[DECODE_INDEX(address)].lock = 1;
}


/*****************************************************************************/
// QEMU Helpers

void helper_icache(CPUMIPSState *env, target_ulong pc_addr, unsigned int opcode)
{
    uint32_t idx = DECODE_INDEX(pc_addr, 
        mips_cache_opts.i_index_mask, mips_cache_opts.i_offset_width);
    uint32_t tag = DECODE_TAG(pc_addr, 
        mips_cache_opts.i_index_width + mips_cache_opts.i_offset_width);

    uint8_t hit = access_cache(env->cache->icache, idx, tag);

    if (hit) {
        mips_cache_opts.i_hit_cnt[idx]++;
    }
    else {
        mips_cache_opts.i_miss_cnt[idx]++;
    }
}

#ifndef CONFIG_USER_ONLY
void helper_dcache(CPUMIPSState *env, target_ulong addr, int is_load) 
{
    /* if is_load is 0 then it is a store instruction */
    hwaddr phys_address = (uint32_t)get_phys_addr_cache(env, addr);

    uint32_t idx = DECODE_INDEX(phys_address, 
        mips_cache_opts.d_index_mask, mips_cache_opts.d_offset_width);
    uint32_t tag = DECODE_TAG(phys_address, 
        mips_cache_opts.d_index_width + mips_cache_opts.d_offset_width);

    uint8_t hit = access_cache(env->cache->dcache, idx, tag);
    
    // TODO: dirty bit etc

    if (hit) {
        if (is_load)
            mips_cache_opts.d_ld_hit_cnt[idx]++;
        else
            mips_cache_opts.d_st_hit_cnt[idx]++;
    }
    else {
        if (is_load)
            mips_cache_opts.d_ld_miss_cnt[idx]++;
        else
            mips_cache_opts.d_st_miss_cnt[idx]++;
    }
}
#endif


// XXX Must fix all helpers to work with I and D cache

void helper_cache_invalidate(CPUMIPSState *env, unsigned int addr)
{
    invalidate(env, addr);
}

void helper_cache_load_tag(CPUMIPSState *env, unsigned int addr)
{
    // TODO - what about TagHi?
    // env->CP0_TagLo = env->cache->icache[DECODE_INDEX(addr)].tag;
}

void helper_cache_store_tag(CPUMIPSState *env, unsigned int addr)
{
    // env->cache->icache[DECODE_INDEX(addr)].tag = env->CP0_TagLo;
}

void helper_cache_hit_invalidate(CPUMIPSState *env, unsigned int addr)
{
    hit_invalidate(env, addr);
}

void helper_cache_fill(CPUMIPSState *env, unsigned int addr)
{
    fill_line(env, addr);
}

void helper_cache_fetch_lock(CPUMIPSState *env, unsigned int addr)
{
    fetch_lock(env, addr);
}
