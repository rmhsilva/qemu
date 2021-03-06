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

#include <inttypes.h>
#include "gnuplot_i.h"
#include <stdint.h>

#ifdef GDP17_TESTBENCH
#include <glib.h>
void pstrcpy(char *dest, int dest_buf_size, const char *src);
char *pstrcat(char *buf, int buf_size, const char *s);
#else
#include "qemu-common.h"
#endif

struct MipsCacheOpts {
    char d_opt[20];
    char i_opt[20];
    char l2_opt[20];

    unsigned char transparent_cache;
    unsigned char onchip_l2;

    unsigned int *d_way_mask;
    unsigned int *i_way_mask;
    unsigned int *l2_way_mask;
    
    unsigned char use_d;
    unsigned char d_replacement;
    unsigned char d_way_width;
    unsigned int d_ways;
    unsigned int d_no_of_lines;
    unsigned int d_lines_per_way;
    unsigned int d_offset_width;
    unsigned int d_index_width;
    unsigned int d_index_mask;

    unsigned char use_i;
    unsigned char i_replacement;
    unsigned char i_way_width;
    unsigned int i_ways;
    unsigned int i_no_of_lines;
    unsigned int i_lines_per_way;
    unsigned int i_offset_width;
    unsigned int i_index_width;
    unsigned int i_index_mask;

    unsigned char use_l2;
    unsigned char l2_replacement;
    unsigned char l2_way_width;
    unsigned int l2_ways;
    unsigned int l2_no_of_lines;
    unsigned int l2_lines_per_way;
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

    uint64_t tlb_error_cnt;

    gnuplot_ctrl *gp_dcache_ldhit;
    gnuplot_ctrl *gp_dcache_ldmiss;
    gnuplot_ctrl *gp_dcache_sthit;
    gnuplot_ctrl *gp_dcache_stmiss;
    gnuplot_ctrl *gp_icache_hit;
    gnuplot_ctrl *gp_icache_miss;  
    unsigned int gnuplot_max_y;    
};


extern struct MipsCacheOpts mips_cache_opts;

unsigned char proc_mips_cache_opt(char which_cache, const char *arg);


void set_transparent_cache(void);
void enable_onchip_l2(void);
unsigned char check_hw_cache_constraints(void);

void log_icache(char verbose);
void log_dcache(char verbose);
void log_l2cache(char verbose);

void log_cache_data(void);

unsigned int gdp_log2(unsigned int n);

void print_cache_info(char which_cache);

void gnuplot_setup(void);
void gnuplot_create(char which_graph);

#endif


