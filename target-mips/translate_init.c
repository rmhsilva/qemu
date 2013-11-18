/*
 *  MIPS emulation for qemu: CPU initialisation routines.
 *
 *  Copyright (c) 2004-2005 Jocelyn Mayer
 *  Copyright (c) 2007 Herve Poussineau
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

/* GDP - used to seed random number generator */
#include <stdlib.h>
#include <time.h>

/* CPU / CPU family specific config register values. */

/* Have config1, uncached coherency */
#define MIPS_CONFIG0                                              \
  ((1 << CP0C0_M) | (0x2 << CP0C0_K0))

/* Have config2, no coprocessor2 attached, no MDMX support attached,
   no performance counters, watch registers present,
   no code compression, EJTAG present, no FPU */
#define MIPS_CONFIG1                                              \
((1 << CP0C1_M) |                                                 \
 (0 << CP0C1_C2) | (0 << CP0C1_MD) | (0 << CP0C1_PC) |            \
 (1 << CP0C1_WR) | (0 << CP0C1_CA) | (1 << CP0C1_EP) |            \
 (0 << CP0C1_FP))

/* Have config3, no tertiary/secondary caches implemented */
#define MIPS_CONFIG2                                              \
((1 << CP0C2_M))

/* No config4, no DSP ASE, no large physaddr (PABITS),
   no external interrupt controller, no vectored interrupts,
   no 1kb pages, no SmartMIPS ASE, no trace logic */
#define MIPS_CONFIG3                                              \
((0 << CP0C3_M) | (0 << CP0C3_DSPP) | (0 << CP0C3_LPA) |          \
 (0 << CP0C3_VEIC) | (0 << CP0C3_VInt) | (0 << CP0C3_SP) |        \
 (0 << CP0C3_SM) | (0 << CP0C3_TL))

/* MMU types, the first four entries have the same layout as the
   CP0C0_MT field.  */
enum mips_mmu_types {
    MMU_TYPE_NONE,
    MMU_TYPE_R4000,
    MMU_TYPE_RESERVED,
    MMU_TYPE_FMT,
    MMU_TYPE_R3000,
    MMU_TYPE_R6000,
    MMU_TYPE_R8000
};

struct mips_def_t {
    const char *name;
    int32_t CP0_PRid;
    int32_t CP0_Config0;
    int32_t CP0_Config1;
    int32_t CP0_Config2;
    int32_t CP0_Config3;
    int32_t CP0_Config6;
    int32_t CP0_Config7;
    target_ulong CP0_LLAddr_rw_bitmask;
    int CP0_LLAddr_shift;
    int32_t SYNCI_Step;
    int32_t CCRes;
    int32_t CP0_Status_rw_bitmask;
    int32_t CP0_TCStatus_rw_bitmask;
    int32_t CP0_SRSCtl;
    int32_t CP1_fcr0;
    int32_t SEGBITS;
    int32_t PABITS;
    int32_t CP0_SRSConf0_rw_bitmask;
    int32_t CP0_SRSConf0;
    int32_t CP0_SRSConf1_rw_bitmask;
    int32_t CP0_SRSConf1;
    int32_t CP0_SRSConf2_rw_bitmask;
    int32_t CP0_SRSConf2;
    int32_t CP0_SRSConf3_rw_bitmask;
    int32_t CP0_SRSConf3;
    int32_t CP0_SRSConf4_rw_bitmask;
    int32_t CP0_SRSConf4;
    int insn_flags;
    enum mips_mmu_types mmu_type;
};

/*****************************************************************************/
/* MIPS CPU definitions */
/* Removed const to set cache sizes using command line options */
static mips_def_t mips_defs[] =
{
    {
        .name = "4Kc",
        .CP0_PRid = 0x00018000,
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (0 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1278FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "4Km",
        .CP0_PRid = 0x00018300,
        /* Config1 implemented, fixed mapping MMU,
           no virtual icache, uncached coherency. */
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_FMT << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1258FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_FMT,
    },
    {
        .name = "4KEcR1",
        .CP0_PRid = 0x00018400,
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (0 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1278FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "4KEmR1",
        .CP0_PRid = 0x00018500,
        .CP0_Config0 = MIPS_CONFIG0 | (MMU_TYPE_FMT << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1258FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_FMT,
    },
    {
        .name = "4KEc",
        .CP0_PRid = 0x00019000,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                    (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (0 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1278FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "4KEm",
        .CP0_PRid = 0x00019100,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                       (MMU_TYPE_FMT << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x1258FF17,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_FMT,
    },
    {
        .name = "24Kc",
        .CP0_PRid = 0x00019300,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        /* No DSP implemented. */
        .CP0_Status_rw_bitmask = 0x1278FF1F,
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "24Kf",
        .CP0_PRid = 0x00019300,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                    (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        /* No DSP implemented. */
        .CP0_Status_rw_bitmask = 0x3678FF1F,
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_L) | (1 << FCR0_W) |
                    (1 << FCR0_D) | (1 << FCR0_S) | (0x93 << FCR0_PRID),
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "34Kf",
        .CP0_PRid = 0x00019500,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (1 << CP0C3_VInt) | (1 << CP0C3_MT) |
                       (1 << CP0C3_DSPP),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x3778FF1F,
        .CP0_TCStatus_rw_bitmask = (0 << CP0TCSt_TCU3) | (0 << CP0TCSt_TCU2) |
                    (1 << CP0TCSt_TCU1) | (1 << CP0TCSt_TCU0) |
                    (0 << CP0TCSt_TMX) | (1 << CP0TCSt_DT) |
                    (1 << CP0TCSt_DA) | (1 << CP0TCSt_A) |
                    (0x3 << CP0TCSt_TKSU) | (1 << CP0TCSt_IXMT) |
                    (0xff << CP0TCSt_TASID),
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_L) | (1 << FCR0_W) |
                    (1 << FCR0_D) | (1 << FCR0_S) | (0x95 << FCR0_PRID),
        .CP0_SRSCtl = (0xf << CP0SRSCtl_HSS),
        .CP0_SRSConf0_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf0 = (1 << CP0SRSC0_M) | (0x3fe << CP0SRSC0_SRS3) |
                    (0x3fe << CP0SRSC0_SRS2) | (0x3fe << CP0SRSC0_SRS1),
        .CP0_SRSConf1_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf1 = (1 << CP0SRSC1_M) | (0x3fe << CP0SRSC1_SRS6) |
                    (0x3fe << CP0SRSC1_SRS5) | (0x3fe << CP0SRSC1_SRS4),
        .CP0_SRSConf2_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf2 = (1 << CP0SRSC2_M) | (0x3fe << CP0SRSC2_SRS9) |
                    (0x3fe << CP0SRSC2_SRS8) | (0x3fe << CP0SRSC2_SRS7),
        .CP0_SRSConf3_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf3 = (1 << CP0SRSC3_M) | (0x3fe << CP0SRSC3_SRS12) |
                    (0x3fe << CP0SRSC3_SRS11) | (0x3fe << CP0SRSC3_SRS10),
        .CP0_SRSConf4_rw_bitmask = 0x3fffffff,
        .CP0_SRSConf4 = (0x3fe << CP0SRSC4_SRS15) |
                    (0x3fe << CP0SRSC4_SRS14) | (0x3fe << CP0SRSC4_SRS13),
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16 | ASE_DSP | ASE_MT,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "74Kf",
        .CP0_PRid = 0x00019700,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) |
                    (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (15 << CP0C1_MMU) |
                       (0 << CP0C1_IS) | (3 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (0 << CP0C1_DS) | (3 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_CA),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (0 << CP0C3_VInt) | (1 << CP0C3_DSPP),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x3778FF1F,
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_L) | (1 << FCR0_W) |
                    (1 << FCR0_D) | (1 << FCR0_S) | (0x93 << FCR0_PRID),
        .SEGBITS = 32,
        .PABITS = 32,
        .insn_flags = CPU_MIPS32R2 | ASE_MIPS16 | ASE_DSP | ASE_DSPR2,
        .mmu_type = MMU_TYPE_R4000,
    },
#if defined(TARGET_MIPS64)
    {
        .name = "R4000",
        .CP0_PRid = 0x00000400,
        /* No L2 cache, icache size 8k, dcache size 8k, uncached coherency. */
        .CP0_Config0 = (1 << 17) | (0x1 << 9) | (0x1 << 6) | (0x2 << CP0C0_K0),
        /* Note: Config1 is only used internally, the R4000 has only Config0. */
        .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
        .CP0_LLAddr_rw_bitmask = 0xFFFFFFFF,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 16,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x3678FFFF,
        /* The R4000 has a full 64bit FPU but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (0x5 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 36,
        .insn_flags = CPU_MIPS3,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "VR5432",
        .CP0_PRid = 0x00005400,
        /* No L2 cache, icache size 8k, dcache size 8k, uncached coherency. */
        .CP0_Config0 = (1 << 17) | (0x1 << 9) | (0x1 << 6) | (0x2 << CP0C0_K0),
        .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
        .CP0_LLAddr_rw_bitmask = 0xFFFFFFFFL,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 16,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x3678FFFF,
        /* The VR5432 has a full 64bit FPU but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (0x54 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 32,
        .insn_flags = CPU_VR54XX,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "5Kc",
        .CP0_PRid = 0x00018100,
        .CP0_Config0 = MIPS_CONFIG0 | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (31 << CP0C1_MMU) |
                       (1 << CP0C1_IS) | (4 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (1 << CP0C1_DS) | (4 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x32F8FFFF,
        .SEGBITS = 42,
        .PABITS = 36,
        .insn_flags = CPU_MIPS64,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "5Kf",
        .CP0_PRid = 0x00018100,
        .CP0_Config0 = MIPS_CONFIG0 | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (31 << CP0C1_MMU) |
                       (1 << CP0C1_IS) | (4 << CP0C1_IL) | (1 << CP0C1_IA) |
                       (1 << CP0C1_DS) | (4 << CP0C1_DL) | (1 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 4,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x36F8FFFF,
        /* The 5Kf has F64 / L / W but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (1 << FCR0_D) | (1 << FCR0_S) |
                    (0x81 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 42,
        .PABITS = 36,
        .insn_flags = CPU_MIPS64,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "20Kc",
        /* We emulate a later version of the 20Kc, earlier ones had a broken
           WAIT instruction. */
        .CP0_PRid = 0x000182a0,
        .CP0_Config0 = MIPS_CONFIG0 | (0x2 << CP0C0_AT) |
                    (MMU_TYPE_R4000 << CP0C0_MT) | (1 << CP0C0_VI),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (47 << CP0C1_MMU) |
                       (2 << CP0C1_IS) | (4 << CP0C1_IL) | (3 << CP0C1_IA) |
                       (2 << CP0C1_DS) | (4 << CP0C1_DL) | (3 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3,
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 1,
        .CP0_Status_rw_bitmask = 0x36FBFFFF,
        /* The 20Kc has F64 / L / W but doesn't use the fcr0 bits. */
        .CP1_fcr0 = (1 << FCR0_3D) | (1 << FCR0_PS) |
                    (1 << FCR0_D) | (1 << FCR0_S) |
                    (0x82 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 36,
        .insn_flags = CPU_MIPS64 | ASE_MIPS3D,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        /* A generic CPU providing MIPS64 Release 2 features.
           FIXME: Eventually this should be replaced by a real CPU model. */
        .name = "MIPS64R2-generic",
        .CP0_PRid = 0x00010000,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (63 << CP0C1_MMU) |
                       (2 << CP0C1_IS) | (4 << CP0C1_IL) | (3 << CP0C1_IA) |
                       (2 << CP0C1_DS) | (4 << CP0C1_DL) | (3 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (1 << CP0C3_LPA),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x36FBFFFF,
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_3D) | (1 << FCR0_PS) |
                    (1 << FCR0_L) | (1 << FCR0_W) | (1 << FCR0_D) |
                    (1 << FCR0_S) | (0x00 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 42,
        /* The architectural limit is 59, but we have hardcoded 36 bit
           in some places...
        .PABITS = 59, */ /* the architectural limit */
        .PABITS = 36,
        .insn_flags = CPU_MIPS64R2 | ASE_MIPS3D,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
        .name = "Loongson-2E",
        .CP0_PRid = 0x6302,
        /*64KB I-cache and d-cache. 4 way with 32 bit cache line size*/
        .CP0_Config0 = (0x1<<17) | (0x1<<16) | (0x1<<11) | (0x1<<8) | (0x1<<5) |
                       (0x1<<4) | (0x1<<1),
        /* Note: Config1 is only used internally, Loongson-2E has only Config0. */
        .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
        .SYNCI_Step = 16,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x35D0FFFF,
        .CP1_fcr0 = (0x5 << FCR0_PRID) | (0x1 << FCR0_REV),
        .SEGBITS = 40,
        .PABITS = 40,
        .insn_flags = CPU_LOONGSON2E,
        .mmu_type = MMU_TYPE_R4000,
    },
    {
      .name = "Loongson-2F",
      .CP0_PRid = 0x6303,
      /*64KB I-cache and d-cache. 4 way with 32 bit cache line size*/
      .CP0_Config0 = (0x1<<17) | (0x1<<16) | (0x1<<11) | (0x1<<8) | (0x1<<5) |
                     (0x1<<4) | (0x1<<1),
      /* Note: Config1 is only used internally, Loongson-2F has only Config0. */
      .CP0_Config1 = (1 << CP0C1_FP) | (47 << CP0C1_MMU),
      .SYNCI_Step = 16,
      .CCRes = 2,
      .CP0_Status_rw_bitmask = 0xF5D0FF1F,   /*bit5:7 not writable*/
      .CP1_fcr0 = (0x5 << FCR0_PRID) | (0x1 << FCR0_REV),
      .SEGBITS = 40,
      .PABITS = 40,
      .insn_flags = CPU_LOONGSON2F,
      .mmu_type = MMU_TYPE_R4000,
    },
    {
        /* A generic CPU providing MIPS64 ASE DSP 2 features.
           FIXME: Eventually this should be replaced by a real CPU model. */
        .name = "mips64dspr2",
        .CP0_PRid = 0x00010000,
        .CP0_Config0 = MIPS_CONFIG0 | (0x1 << CP0C0_AR) | (0x2 << CP0C0_AT) |
                       (MMU_TYPE_R4000 << CP0C0_MT),
        .CP0_Config1 = MIPS_CONFIG1 | (1 << CP0C1_FP) | (63 << CP0C1_MMU) |
                       (2 << CP0C1_IS) | (4 << CP0C1_IL) | (3 << CP0C1_IA) |
                       (2 << CP0C1_DS) | (4 << CP0C1_DL) | (3 << CP0C1_DA) |
                       (1 << CP0C1_PC) | (1 << CP0C1_WR) | (1 << CP0C1_EP),
        .CP0_Config2 = MIPS_CONFIG2,
        .CP0_Config3 = MIPS_CONFIG3 | (1 << CP0C3_LPA),
        .CP0_LLAddr_rw_bitmask = 0,
        .CP0_LLAddr_shift = 0,
        .SYNCI_Step = 32,
        .CCRes = 2,
        .CP0_Status_rw_bitmask = 0x37FBFFFF,
        .CP1_fcr0 = (1 << FCR0_F64) | (1 << FCR0_3D) | (1 << FCR0_PS) |
                    (1 << FCR0_L) | (1 << FCR0_W) | (1 << FCR0_D) |
                    (1 << FCR0_S) | (0x00 << FCR0_PRID) | (0x0 << FCR0_REV),
        .SEGBITS = 42,
        /* The architectural limit is 59, but we have hardcoded 36 bit
           in some places...
        .PABITS = 59, */ /* the architectural limit */
        .PABITS = 36,
        .insn_flags = CPU_MIPS64R2 | ASE_DSP | ASE_DSPR2,
        .mmu_type = MMU_TYPE_R4000,
    },

#endif
};

static void config_cache(void)
{
    /* Use default config if cache options not passed to program */
    if(!mips_cache_opts.hw_cache_config || (!mips_cache_opts.use_i && !mips_cache_opts.use_d))
      return;

    check_hw_cache_constraints();

    int i;
    uint32_t tmp0, tmp1;
    uint32_t is, il, ia, ds, dl, da, ss, sl, sa, ts, tl, ta;
   
    /* Cache masks L1 fields are 3 bit wide, L2/L3 4-bit wide*/ 
    uint32_t icache_nmask = (7 << CP0C1_IS) | (7 << CP0C1_IL) | (7 << CP0C1_IA);
    uint32_t dcache_nmask = (7 << CP0C1_DS) | (7 << CP0C1_DL) | (7 << CP0C1_DA);
    uint32_t l2cache_nmask = (15 << CP0C2_SS) | (15 << CP0C2_SL) | (15 << CP0C2_SA);
    uint32_t l3cache_nmask = (15 << CP0C2_TS) | (15 << CP0C2_TL) | (15 << CP0C2_TA);

    /* Invert mask */
    icache_nmask = ~icache_nmask;
    dcache_nmask = ~dcache_nmask;
    l2cache_nmask = ~l2cache_nmask;
    l3cache_nmask = ~l3cache_nmask;

    /* Calculate number of cache lines per way, format: 64 x 2^S */
    is = (mips_cache_opts.use_i)? (mips_cache_opts.i_index_width -
        6 - mips_cache_opts.i_way_width) : 0;
    ds = (mips_cache_opts.use_d)? mips_cache_opts.d_index_width -
        6 - mips_cache_opts.d_way_width : 0;
    ss = (mips_cache_opts.use_l2)? mips_cache_opts.l2_index_width -
        6 - mips_cache_opts.l2_way_width : 0;  
    ts = 0;
    
    /* Calculate the size of a cache line (in bytes), format: 2 x 2^L */
    il = (mips_cache_opts.use_i)? (mips_cache_opts.i_offset_width - 1) : 0;
    dl = (mips_cache_opts.use_d)? (mips_cache_opts.d_offset_width - 1) : 0;
    sl = (mips_cache_opts.use_l2)? (mips_cache_opts.l2_offset_width - 1) : 0;
    tl = 0;

    /* Calculate associativity, format: (A+1)-way set-associative */
    ia = (mips_cache_opts.i_way_width)? ((1 << mips_cache_opts.i_way_width) - 1)
        : 0;
    da = (mips_cache_opts.d_way_width)? ((1 << mips_cache_opts.d_way_width) - 1)
        : 0;
    sa = (mips_cache_opts.l2_way_width)? ((1 << mips_cache_opts.l2_way_width) - 1)
        : 0;
    ta = 0;

    for (i = 0; i < ARRAY_SIZE(mips_defs); i++)
    {
        /* Clear default cache configuration */
        tmp0 = mips_defs[i].CP0_Config1 & icache_nmask & dcache_nmask;
        tmp1 = mips_defs[i].CP0_Config2 & l2cache_nmask & l2cache_nmask;

        /* Apply new configuration */
        tmp0 |= (is << CP0C1_IS) | (il << CP0C1_IL) | (ia << CP0C1_IA) |
            (ds << CP0C1_DS) | (dl << CP0C1_DL) | (da << CP0C1_DA);
        tmp1 |= (ss << CP0C2_SS) | (sl << CP0C2_SL) | (sa << CP0C2_SA) |
            (ts << CP0C2_TS) | (tl << CP0C2_TL) | (ta << CP0C2_TA);

        /* Store values in CP0 registers */
        mips_defs[i].CP0_Config1 = tmp0;
        mips_defs[i].CP0_Config2 = tmp1;
        printf("%8x %8x \n", mips_defs[i].CP0_Config2, tmp1);
    }
}

static const mips_def_t *cpu_mips_find_by_name (const char *name)
{
    int i;
    config_cache();
    for (i = 0; i < ARRAY_SIZE(mips_defs); i++) {
        if (strcasecmp(name, mips_defs[i].name) == 0) {
            return &mips_defs[i];
        }
    }
    return NULL;
}

void mips_cpu_list (FILE *f, fprintf_function cpu_fprintf)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(mips_defs); i++) {
        (*cpu_fprintf)(f, "MIPS '%s'\n",
                       mips_defs[i].name);
    }
}

#ifndef CONFIG_USER_ONLY
static void no_mmu_init (CPUMIPSState *env, const mips_def_t *def)
{
    env->tlb->nb_tlb = 1;
    env->tlb->map_address = &no_mmu_map_address;
}

static void fixed_mmu_init (CPUMIPSState *env, const mips_def_t *def)
{
    env->tlb->nb_tlb = 1;
    env->tlb->map_address = &fixed_mmu_map_address;
}

static void r4k_mmu_init (CPUMIPSState *env, const mips_def_t *def)
{
    env->tlb->nb_tlb = 1 + ((def->CP0_Config1 >> CP0C1_MMU) & 63);
    env->tlb->map_address = &r4k_map_address;
    env->tlb->helper_tlbwi = r4k_helper_tlbwi;
    env->tlb->helper_tlbwr = r4k_helper_tlbwr;
    env->tlb->helper_tlbp = r4k_helper_tlbp;
    env->tlb->helper_tlbr = r4k_helper_tlbr;
}

static void mmu_init (CPUMIPSState *env, const mips_def_t *def)
{
    env->tlb = g_malloc0(sizeof(CPUMIPSTLBContext));

    switch (def->mmu_type) {
        case MMU_TYPE_NONE:
            no_mmu_init(env, def);
            break;
        case MMU_TYPE_R4000:
            r4k_mmu_init(env, def);
            break;
        case MMU_TYPE_FMT:
            fixed_mmu_init(env, def);
            break;
        case MMU_TYPE_R3000:
        case MMU_TYPE_R6000:
        case MMU_TYPE_R8000:
        default:
            cpu_abort(env, "MMU type not supported\n");
    }
}
#endif /* CONFIG_USER_ONLY */

static void fpu_init (CPUMIPSState *env, const mips_def_t *def)
{
    int i;

    for (i = 0; i < MIPS_FPU_MAX; i++)
        env->fpus[i].fcr0 = def->CP1_fcr0;

    memcpy(&env->active_fpu, &env->fpus[0], sizeof(env->active_fpu));
}

static void mvp_init (CPUMIPSState *env, const mips_def_t *def)
{
    env->mvp = g_malloc0(sizeof(CPUMIPSMVPContext));

    /* MVPConf1 implemented, TLB sharable, no gating storage support,
       programmable cache partitioning implemented, number of allocatable
       and sharable TLB entries, MVP has allocatable TCs, 2 VPEs
       implemented, 5 TCs implemented. */
    env->mvp->CP0_MVPConf0 = (1 << CP0MVPC0_M) | (1 << CP0MVPC0_TLBS) |
                             (0 << CP0MVPC0_GS) | (1 << CP0MVPC0_PCP) |
// TODO: actually do 2 VPEs.
//                             (1 << CP0MVPC0_TCA) | (0x1 << CP0MVPC0_PVPE) |
//                             (0x04 << CP0MVPC0_PTC);
                             (1 << CP0MVPC0_TCA) | (0x0 << CP0MVPC0_PVPE) |
                             (0x00 << CP0MVPC0_PTC);
#if !defined(CONFIG_USER_ONLY)
    /* Usermode has no TLB support */
    env->mvp->CP0_MVPConf0 |= (env->tlb->nb_tlb << CP0MVPC0_PTLBE);
#endif

    /* Allocatable CP1 have media extensions, allocatable CP1 have FP support,
       no UDI implemented, no CP2 implemented, 1 CP1 implemented. */
    env->mvp->CP0_MVPConf1 = (1 << CP0MVPC1_CIM) | (1 << CP0MVPC1_CIF) |
                             (0x0 << CP0MVPC1_PCX) | (0x0 << CP0MVPC1_PCP2) |
                             (0x1 << CP0MVPC1_PCP1);
}


/**GDP**/
#define SWITCH_FUNCTION(type) switch(mips_cache_opts.type##_replacement) { \
    case 0:                                                                \
        env->cache->type##cache_api = &interface_dm;                       \
    break;                                                                 \
    case 2:                                                                \
        env->cache->type##cache_api = &interface_lru;                      \
    break;                                                                 \
    case 4:                                                                \
        env->cache->type##cache_api = &interface_lfu;                      \
    break;                                                                 \
    case 6:                                                                \
        env->cache->type##cache_api = &interface_rnd;                      \
    break;                                                                 \
}


static void cache_init (CPUMIPSState *env, const mips_def_t *def)
{

    //pass seed to random number generator
    srand(time(NULL));
  
    if(mips_cache_opts.use_d | mips_cache_opts.use_i | mips_cache_opts.use_l2)
      printf("Initialising cache... ");
    
    // allocate the cache context memory...
    env->cache = (CPUCacheContext *)g_malloc0(sizeof(CPUCacheContext));
    env->cache->opts = &mips_cache_opts;

    if (mips_cache_opts.use_i) {
        env->cache->icache =
            (cache_item_t *)g_malloc0(sizeof(cache_item_t)*mips_cache_opts.i_no_of_lines);
        SWITCH_FUNCTION(i);
        // TODO: add replacement algo choice to command lines
    }

    if (mips_cache_opts.use_d) {
        env->cache->dcache =
            (cache_item_t *)g_malloc0(sizeof(cache_item_t)*mips_cache_opts.d_no_of_lines);
        SWITCH_FUNCTION(d);
    }

    if (mips_cache_opts.use_l2) {
        env->cache->l2cache =
            (cache_item_t *)g_malloc0(sizeof(cache_item_t)*mips_cache_opts.l2_no_of_lines);
        SWITCH_FUNCTION(l2);
    }

    if(mips_cache_opts.use_d | mips_cache_opts.use_i | mips_cache_opts.use_l2)
        printf("Done.\n");
}
/**GDP**/
