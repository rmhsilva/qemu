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


/*****************************************************************************/
/* GDP */

#define DECODE_INDEX(addr) (((addr) & (MIPS_CACHE_INDEX_MASK)) >> MIPS_CACHE_INDEX_SHIFT)
#define DECODE_TAG(addr) ((addr) >> MIPS_CACHE_TAG_SHIFT)


// I-cache utility functions:

static inline void invalidate(CPUMIPSState *env, unsigned int address)
{
    unsigned int ind = DECODE_INDEX(address);
    env->cache->data[ind].valid = 0;
    env->cache->data[ind].lock = 0;
}


// static void fill_line(CPUMIPSState *env, unsigned int address)
// {
//     unsigned int index_val = DECODE_INDEX(address);

//     if(env->cache->data[index_val].lock == 0) {   
//         env->cache->data[index_val].tag = DECODE_TAG(address);
//         env->cache->data[index_val].valid = 1;
//     }
//     else {
//         printf("Line Locked: %x\n", index_val);
//     }
// }


static void hit_miss(CPUMIPSState *env, unsigned int address)
{
    unsigned int tag_val = DECODE_TAG(address);
    unsigned int index_val = DECODE_INDEX(address);
    unsigned int taglo = env->cache->data[index_val].tag;
    
    if((taglo==tag_val) && (env->cache->data[index_val].valid==1)) {
        printf("Hit! \n");
    }
    else {
        printf("Miss! \n");

        if(env->cache->data[index_val].lock == 0) {   
            env->cache->data[index_val].tag = tag_val;
            env->cache->data[index_val].valid = 1;
        }
        else {
            printf("Line Locked\n");
        }
    }
}


// static void fetch_lock(CPUMIPSState *env, unsigned int address)
// {
//     env->cache->data[DECODE_INDEX(address)].lock = 1;
// }


// QEMU Helpers

void helper_tester(CPUMIPSState *env, target_ulong pc)
{
    printf("[%x] ", pc);

    // if (env->cache->data[0].tag != DECODE_TAG(pc)) {
    //     env->cache->data[0].tag = DECODE_TAG(pc);
    //     printf("miss\n");
    // }
    // else {
    //     printf("hit\n");
    // }
    
    hit_miss(env, (unsigned int)pc);
}
