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

typedef struct logdata_t logdata_t;
struct logdata_t {
    uint32_t address;
    uint8_t not_used;
    uint8_t is_load;
    uint8_t opcode;
    uint8_t is_hit;
};

/*****************************************************************************/
/* Misc Helpers */

static void log_data(CPUMIPSState *env,
    uint32_t addr, uint8_t is_hit, uint8_t opcode, uint32_t is_load)
{
    logdata_t log_data = {
        .address = addr,
        .opcode = opcode,
        .is_hit = is_hit,
        .is_load = is_load,
    };
    
    fwrite(&log_data, sizeof(logdata_t), 1, env->cache->logfile);
    // printf("[%x]: %d, Opcode: %x, Load: %d\n", addr, is_hit, opcode, is_load);
}


/*****************************************************************************/
/* GDP */

#define DECODE_INDEX(addr) (((addr) & (MIPS_CACHE_INDEX_MASK)) >> MIPS_CACHE_INDEX_SHIFT)
#define DECODE_TAG(addr) ((addr) >> MIPS_CACHE_TAG_SHIFT)
#define DECODE_OFFSET(addr) ((addr & (MIPS_CACHE_OFFSET_MASK)) >> MIPS_CACHE_OFFSET_SHIFT)

// #ifndef CONFIG_USER_ONLY
// // D-cache test:
// void helper_dcache(CPUMIPSState *env, target_ulong addr)
// {
//     hwaddr phys_address;
//     phys_address = get_phys_addr_cache(env, addr);
    
//     fprintf(stderr,"VA: [%x] PA: %" PRIx64 "\n", addr,phys_address);

// #if TARGET_LONG_BITS == 32
//     //printf("LD PA: %" PRId64 "\n", phys_address);
// #else
//    // printf("LD PA: %" PRId64 "\n", phys_address);
// #endif

// }
// #endif


// I-cache utility functions:

static inline void invalidate(CPUMIPSState *env, unsigned int address)
{
    unsigned int ind = DECODE_INDEX(address);
    env->cache->icache[ind].valid = 0;
    env->cache->icache[ind].lock = 0;
}


static inline void fill_line(CPUMIPSState *env, unsigned int address)
{
    unsigned int index_val = DECODE_INDEX(address);

    if (env->cache->icache[index_val].lock == 0) {
        env->cache->icache[index_val].tag = DECODE_TAG(address);
        env->cache->icache[index_val].valid = 1;
    }
    else {
        printf("Line Locked: %x\n", index_val);
    }
}


/**
 * access_cache: An interface to either a data cache or instruction cache.
 * @param  *cache  Pointer to the cache memory
 * @param  address Address to access
 * @return         0: miss, 1: hit
 */
static inline uint8_t access_cache(cache_item_t *cache, uint32_t address)
{
    uint32_t tag_val = DECODE_TAG(address);
    uint32_t index_val = DECODE_INDEX(address);
    uint32_t taglo = cache[index_val].tag;
    
    if ((taglo==tag_val) && (cache[index_val].valid==1)) {
        return 1;
    }
    else {
        if(cache[index_val].lock == 0) {   
            cache[index_val].tag = tag_val;
            cache[index_val].valid = 1;
        }
        else {
            printf("Line Locked!!\n");
        }
        return 0;
    }
}


static inline void hit_invalidate(CPUMIPSState *env, unsigned int address)
{
    unsigned int tag_val = DECODE_TAG(address);
    unsigned int index_val = DECODE_INDEX(address);
    unsigned int taglo = env->cache->icache[index_val].tag;

    if((taglo==tag_val) && (env->cache->icache[index_val].valid==1)) {
        invalidate(env, address);
    }
}


static inline void fetch_lock(CPUMIPSState *env, unsigned int address)
{
    fill_line(env, address);
    env->cache->icache[DECODE_INDEX(address)].lock = 1;
}


/*****************************************************************************/
// QEMU Helpers

void helper_icache(CPUMIPSState *env, target_ulong pc, unsigned int opcode)
{    
    uint8_t hit = access_cache(env->cache->icache, (uint32_t)pc);

    // uint32_t addr, uint8_t is_hit, uint8_t opcode, uint32_t is_load
    log_data(env, (uint32_t)pc, hit, ((opcode >> 26) & 0x3F), 0);
}

#ifndef CONFIG_USER_ONLY
void helper_dcache(CPUMIPSState *env, target_ulong addr, int is_load) 
{
    // if is_load is 0 then it is a store instruction
    
    hwaddr phys_address = (uint32_t)get_phys_addr_cache(env, addr);
    uint8_t hit = access_cache(env->cache->dcache, phys_address);
    
    // TODO: dirty bit etc

    log_data(env, phys_address, hit, 0xFF, is_load);
}
#endif

void helper_cache_invalidate(CPUMIPSState *env, unsigned int addr)
{
    invalidate(env, addr);
}

void helper_cache_load_tag(CPUMIPSState *env, unsigned int addr)
{
    // TODO - what about TagHi?
    env->CP0_TagLo = env->cache->icache[DECODE_INDEX(addr)].tag;
}

void helper_cache_store_tag(CPUMIPSState *env, unsigned int addr)
{
    env->cache->icache[DECODE_INDEX(addr)].tag = env->CP0_TagLo;
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
