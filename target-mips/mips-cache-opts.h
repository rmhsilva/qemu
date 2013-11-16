/* GDP */
/* File holding sizes of caches in MIPS processor */
/* Sizes are set up in vl.c using command line arguments */
/* Pointer to a file is used to dump cache performance info */

/* Cache replacement algorithms:
 * 0 - no replacement algorithm -> direct mapped (8'b00000000)
 * 2 - least recently used (8'b00000010)
 * 4 - least frequently used (8'b00000100)
 * 6 - random (8'b00000110)
 */

/* offset_width includes LSBs which address bytes in a word */
/* index_mask is a mask after shifting address by offset_width to the right */


#ifndef MIPS_CACHE_OPTS
#define MIPS_CACHE_OPTS

#include "qemu-common.h"

struct MipsCacheOpts {
    char d_opt[16];
    char i_opt[16];
    char l2_opt[16];

    unsigned char hw_cache_config;

    unsigned int *d_way_mask;
    unsigned int *i_way_mask;
    unsigned int *l2_way_mask;
    
    unsigned char use_d;
    unsigned char d_replacement;
    unsigned char d_way_width;
    unsigned int d_no_of_lines;
    unsigned int d_offset_width;
    unsigned int d_index_width;
    unsigned int d_index_mask;

    unsigned char use_i;
    unsigned char i_replacement;
    unsigned char i_way_width;
    unsigned int i_no_of_lines;
    unsigned int i_offset_width;
    unsigned int i_index_width;
    unsigned int i_index_mask;

    unsigned char use_l2;
    unsigned char l2_replacement;
    unsigned char l2_way_width;
    unsigned int l2_no_of_lines;
    unsigned int l2_offset_width;
    unsigned int l2_index_width;
    unsigned int l2_index_mask;

    FILE *icache_log;
    FILE *dcache_log;
    FILE *l2cache_log;

    uint64_t *d_ld_hit_cnt;
    uint64_t *d_ld_miss_cnt;
    uint64_t *d_st_hit_cnt;
    uint64_t *d_st_miss_cnt;
    uint64_t *i_hit_cnt;
    uint64_t *i_miss_cnt;
    uint64_t *l2_hit_cnt;
    uint64_t *l2_miss_cnt;
};


extern struct MipsCacheOpts mips_cache_opts;

unsigned char proc_mips_cache_opt(char which_cache, const char *arg);


void set_hw_cache_config(void);
unsigned char check_hw_cache_constraints(void);

void log_icache(char verbose);
void log_dcache(char verbose);
void log_l2cache(char verbose);

void log_cache_data(void);

unsigned int gdp_log2(unsigned int n);

#endif


