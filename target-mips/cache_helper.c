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
#include "cache.h"
#include "mips-cache-opts.h"

/* Macros for displaying uint64t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>


#ifdef UNCACHED_REGION
static inline uint8_t check_cached_region(target_ulong addr)
{
    if(addr < UNCACHED_START || addr > UNCACHED_END)
        return 0;
    return 1;
}
#endif

#define I_LOG 2000
#define D_LOG 2000
#define L2_LOG 2000
/*static int i_cnt = 0, d_cnt = 0, l2_cnt = 0;*/

#if !defined(CONFIG_USER_ONLY)
//static uint8_t asid, asid_prev;
#endif

/*****************************************************************************/
// Main Helpers

/**
 * Main I-Cache helper function:
 */
void helper_icache(CPUMIPSState *env, target_ulong pc_addr, unsigned int opcode)
{
#ifdef UNCACHED_REGION
    if(check_cached_region(pc_addr)) {
        return;
    }
#endif

#ifndef CONFIG_USER_ONLY
    hwaddr phys_address = (hwaddr)get_phys_addr_cache(env, pc_addr);
#else
    hwaddr phys_address = (hwaddr)pc_addr;
#endif

    uint32_t idx_l1 = DECODE_INDEX_i(pc_addr);
    uint32_t tag_l1 = DECODE_TAG_i(phys_address);
    uint32_t idx_l2, tag_l2;
    int miss_l2;

/*    // Dump data periodically*/
/*    if (i_cnt++ >= I_LOG) {*/
/*        i_cnt = 0;*/
/*        log_icache(0);*/
/*    }*/

    int miss_l1 = (*env->cache->icache_api->lookup)(env->cache->icache,
                        idx_l1, tag_l1, mips_cache_opts.i_way_mask, 
                        mips_cache_opts.i_ways);

    // Check for hit / miss
    if (miss_l1 != -1) {
        mips_cache_opts.i_hit_cnt[idx_l1]++;
    }
    else {
        mips_cache_opts.i_miss_cnt[idx_l1]++;

        if(mips_cache_opts.use_l2) {
/*            if (l2_cnt++ >= L2_LOG) {*/
/*                l2_cnt = 0;*/
/*                log_l2cache(0);*/
/*            }*/
            idx_l2 = DECODE_INDEX_l2(phys_address);
            tag_l2 = DECODE_TAG_l2(phys_address);
            miss_l2 = (*env->cache->l2cache_api->lookup)(env->cache->l2cache,
                        idx_l2, tag_l2, mips_cache_opts.l2_way_mask, 
                        mips_cache_opts.l2_ways);

            if (miss_l2 != -1)
                mips_cache_opts.l2_hit_cnt[idx_l2]++;
            else
                mips_cache_opts.l2_miss_cnt[idx_l2]++;
        }
    }
}


/**
 * This is called by the two d-cache helpers
 * if is_load is 0 then it is a store instruction
 */
static inline void 
helper_dcache (CPUMIPSState *env, target_ulong addr, int is_load) 
{
#ifdef UNCACHED_REGION
    if(check_cached_region(addr)) {
        return;
    }
#endif

#ifndef CONFIG_USER_ONLY
    hwaddr phys_address = (hwaddr)get_phys_addr_cache(env, addr);
#else
    hwaddr phys_address = (hwaddr)addr;
#endif

    if (phys_address == -1) {
        // printf("Error in Physical Address lookup, %x\n", addr);
        mips_cache_opts.tlb_error_cnt += 1;
        return;
    }

    uint32_t idx_l1 = DECODE_INDEX_d(addr);
    uint32_t tag_l1 = DECODE_TAG_d(phys_address);
    uint32_t idx_l2, tag_l2;
    int miss_l2;

/*    // Dump data periodically*/
/*    if (d_cnt++ >= D_LOG) {*/
/*        d_cnt = 0;*/
/*        log_dcache(0);*/
/*    }*/

    int miss_l1 = (*env->cache->dcache_api->lookup)(env->cache->dcache,
                        idx_l1, tag_l1, mips_cache_opts.d_way_mask, 
                        mips_cache_opts.d_ways);

    if (miss_l1 != -1) {
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
/*            if (l2_cnt++ >= L2_LOG) {*/
/*                l2_cnt = 0;*/
/*                log_l2cache(0);*/
/*            }*/
            idx_l2 = DECODE_INDEX_l2(phys_address);
            tag_l2 = DECODE_TAG_l2(phys_address);
            miss_l2 = (*env->cache->l2cache_api->lookup)(env->cache->l2cache,
                        idx_l2, tag_l2, mips_cache_opts.l2_way_mask, 
                        mips_cache_opts.l2_ways);

            if (miss_l2 != -1)
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


/*****************************************************************************/
// Cache Operation helpers:


#define HELPER_INVALIDATE(type)                                                \
void helper_cache_invalidate_ ## type (CPUMIPSState *env, unsigned int addr)   \
{                                                                              \
    (*env->cache->type##cache_api->invalidate)(env->cache->type##cache,        \
        DECODE_INDEX_WAY_ ## type (addr), DECODE_TAG_ ## type (addr));         \
}
HELPER_INVALIDATE(i)
HELPER_INVALIDATE(d)
HELPER_INVALIDATE(l2)
#undef HELPER_INVALIDATE

// NOTE: TagHi is optional. 
// Also, we're not storing DataHi/Lo at all as we don't store data anywhere.
#define HELPER_LOAD_TAG(type)                                                  \
void helper_cache_load_tag_ ## type (CPUMIPSState *env, unsigned int addr)     \
{                                                                              \
    env->CP0_TagLo = env->cache->type##cache[DECODE_INDEX_WAY_ ## type (addr)].tag;\
}
HELPER_LOAD_TAG(i)
HELPER_LOAD_TAG(d)
HELPER_LOAD_TAG(l2)
#undef HELPER_LOAD_TAG

#define HELPER_STORE_TAG(type)                                                 \
void helper_cache_store_tag_ ## type (CPUMIPSState *env, unsigned int addr)    \
{                                                                              \
    env->cache->type##cache[DECODE_INDEX_WAY_ ## type (addr)].tag = env->CP0_TagLo;\
    env->cache->type##cache[DECODE_INDEX_WAY_ ## type (addr)].lock = 0;        \
}
HELPER_STORE_TAG(i)
HELPER_STORE_TAG(d)
HELPER_STORE_TAG(l2)
#undef HELPER_STORE_TAG

#define HELPER_HIT_INVALIDATE(type)                                            \
void helper_cache_hit_invalidate_ ## type(CPUMIPSState *env, unsigned int addr)\
{                                                                              \
    (*env->cache->type##cache_api->hit_invalidate)(env->cache->type##cache,    \
        DECODE_INDEX_ ## type (addr), DECODE_TAG_ ## type (addr),              \
        mips_cache_opts.type ## _way_mask, mips_cache_opts.type ## _ways); \
}
HELPER_HIT_INVALIDATE(i)
HELPER_HIT_INVALIDATE(d)
HELPER_HIT_INVALIDATE(l2)
#undef HELPER_HIT_INVALIDATE

// D-Cache and L2 cache DO NOT have a Fill instruction.
// This code is for Hit Writeback Invalidate
#define HELPER_FILL_LINE(type)                                                 \
void helper_cache_fill_ ## type(CPUMIPSState *env, unsigned int addr)          \
{                                                                              \
    (*env->cache->type##cache_api->hit_invalidate)(env->cache->type##cache,    \
        DECODE_INDEX_ ## type (addr), DECODE_TAG_ ## type (addr),              \
        mips_cache_opts.type ## _way_mask, mips_cache_opts.type ## _ways); \
}
HELPER_FILL_LINE(d)
HELPER_FILL_LINE(l2)
#undef HELPER_FILL_LINE

void helper_cache_fill_i(CPUMIPSState *env, unsigned int addr) {
    // I-Cache is the only cache with an actual fill instruction
    (*env->cache->icache_api->fill_line)(env->cache->icache,
        DECODE_INDEX_i(addr), DECODE_TAG_i(addr),
        mips_cache_opts.i_way_mask, mips_cache_opts.i_ways);
}

#define HELPER_FETCH_LOCK(type)                                                \
void helper_cache_fetch_lock_ ## type(CPUMIPSState *env, unsigned int addr)    \
{                                                                              \
    (*env->cache->type##cache_api->fetch_lock)(env->cache->type##cache,        \
        DECODE_INDEX_ ## type (addr), DECODE_TAG_ ## type (addr),              \
        mips_cache_opts.type ## _way_mask, mips_cache_opts.type ## _ways); \
}
HELPER_FETCH_LOCK(i)
HELPER_FETCH_LOCK(d)
#undef HELPER_FETCH_LOCK

// No-op for L2.
void helper_cache_fetch_lock_l2(CPUMIPSState *env, unsigned int addr) { }
