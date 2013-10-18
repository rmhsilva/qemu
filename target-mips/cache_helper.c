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
// #include "qemu/host-utils.h"  // Probably don't need this

#include "helper.h"
/* Macros for displaying uint64t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/*****************************************************************************/
/* GDP */

#define DECODE_INDEX(addr) (((addr) & (MIPS_CACHE_INDEX_MASK)) >> MIPS_CACHE_INDEX_SHIFT)
#define DECODE_TAG(addr) ((addr) >> MIPS_CACHE_TAG_SHIFT)
#define DECODE_OFFSET(addr) ((addr & (MIPS_CACHE_OFFSET_MASK)) >> MIPS_CACHE_OFFSET_SHIFT)

#ifndef CONFIG_USER_ONLY
// D-cache test:
void helper_dcache(CPUMIPSState *env, target_ulong addr)
{
    hwaddr phys_address;
    phys_address = get_phys_addr_cache(env, addr);
    
   fprintf(stderr,"VA: [%x] PA: %" PRIx64 "\n", addr,phys_address);

#if TARGET_LONG_BITS == 32
    //printf("LD PA: %" PRId64 "\n", phys_address);
#else
   // printf("LD PA: %" PRId64 "\n", phys_address);
#endif

}
#endif
// I-cache utility functions:

static inline void invalidate(CPUMIPSState *env, unsigned int address)
{
    unsigned int ind = DECODE_INDEX(address);
    env->cache->data[ind].valid = 0;
    env->cache->data[ind].lock = 0;
}


static void fill_line(CPUMIPSState *env, unsigned int address)
{
    unsigned int index_val = DECODE_INDEX(address);

    if (env->cache->data[index_val].lock == 0) {
        env->cache->data[index_val].tag = DECODE_TAG(address);
        env->cache->data[index_val].valid = 1;
    }
    else {
        printf("Line Locked: %x\n", index_val);
    }
}


inline static void hit_miss(CPUMIPSState *env, unsigned int address)
{
    unsigned int tag_val = DECODE_TAG(address);
    unsigned int index_val = DECODE_INDEX(address);
    unsigned int taglo = env->cache->data[index_val].tag;
    
    if((taglo==tag_val) && (env->cache->data[index_val].valid==1)) {
        printf("Hit:  tag[%x], idx[%x], offset[%x]! \n",
            tag_val, index_val, DECODE_OFFSET(address));
    }
    else {
        printf("Miss: tag[%x], idx[%x], offset[%x]! \n",
            tag_val, index_val, DECODE_OFFSET(address));

        if(env->cache->data[index_val].lock == 0) {   
            env->cache->data[index_val].tag = tag_val;
            env->cache->data[index_val].valid = 1;
        }
        else {
            printf("Line Locked\n");
        }
    }
}


inline static void hit_invalidate(CPUMIPSState *env, unsigned int address)
{
    unsigned int tag_val = DECODE_TAG(address);
    unsigned int index_val = DECODE_INDEX(address);
    unsigned int taglo = env->cache->data[index_val].tag;

    if((taglo==tag_val) && (env->cache->data[index_val].valid==1)) {
        invalidate(env, address);
    }
}


static void fetch_lock(CPUMIPSState *env, unsigned int address)
{
    fill_line(env, address);
    env->cache->data[DECODE_INDEX(address)].lock = 1;
}


/*****************************************************************************/
// QEMU Helpers

void helper_icache(CPUMIPSState *env, target_ulong pc)
{
    printf("[%x] ", pc);
    
    hit_miss(env, (unsigned int)pc);
}

void helper_cache_invalidate(CPUMIPSState *env, unsigned int addr)
{
    invalidate(env, addr);
}

void helper_cache_load_tag(CPUMIPSState *env, unsigned int addr)
{
    // TODO - what about TagHi?
    env->CP0_TagLo = env->cache->data[DECODE_INDEX(addr)].tag;
}

void helper_cache_store_tag(CPUMIPSState *env, unsigned int addr)
{
    env->cache->data[DECODE_INDEX(addr)].tag = env->CP0_TagLo;
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
