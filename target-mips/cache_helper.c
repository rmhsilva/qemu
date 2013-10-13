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

// static inline void invalidate(unsigned int address)
// {
//     unsigned int ind = DECODE_INDEX(address);
//     cache[ind].valid = 0;
//     cache[ind].lock = 0;
// }

// static inline void load_tag(unsigned int address)
// {
//     taglo = cache[DECODE_INDEX(address)].tag;
// }

// static inline void store_tag(unsigned int address)
// {
//     cache[DECODE_INDEX(address)].tag = taglo;   
// }


// Helpers

void helper_tester(CPUMIPSState *env, target_ulong pc)
{
    printf("[%x] current tag: %x. Require tag: %x. => ", 
        pc, env->cache->data[0].tag, DECODE_TAG(pc));

    if (env->cache->data[0].tag != DECODE_TAG(pc)) {
        env->cache->data[0].tag = DECODE_TAG(pc);
        printf("miss\n");
    }
    else {
        printf("hit\n");
    }
}
