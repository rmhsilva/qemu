/* GDP */
/* File includes functions calculating offset and index widths */
/* See allowed cache types and sizes at the end of this file */

#include <string.h>
#include <stdio.h>
#include "qemu-common.h"
#include "mips-cache-opts.h"
#include "cpu.h"

/* Macros for displaying uint64t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/* which_cache: */
/*     d - d-cache */
/*     i - i-cache */
/*     u - unified L2 cache */

/* Cache replacement algorithms:
 * 0 - no replacement algorithm -> direct mapped (8'b00000000)
 * 2 - least recently used (8'b00000010)
 * 4 - least frequently used (8'b00000100)
 * 6 - random (8'b00000110)
 */

/*  GDP cache options struct MIPS  */
struct MipsCacheOpts mips_cache_opts = {"","","",0};

unsigned char proc_mips_cache_opt(char which_cache, const char *arg)
{
#ifndef TARGET_MIPS  
    fprintf(stderr,
    "*** Error: MIPS cache options are only supported in MIPS target\n");
    return 1;
#else
    unsigned char *replacement;
    unsigned char *way_width;
    unsigned int **way_mask;
    unsigned int *no_of_lines;
    /* offset width includes bits used to address bytes in a word */
    unsigned int *offset_width;
    unsigned int *index_width;
    unsigned int *index_mask;
    unsigned int tmp0, tmp1, tmp3;
    char tmp2[5], tmp4[5];
    int opt_len;


    /* D-cache */
    if(which_cache == 'd')
    {
        pstrcpy(mips_cache_opts.d_opt,15,arg);
        mips_cache_opts.use_d = 1; 
        replacement = &mips_cache_opts.d_replacement;
        way_width = &mips_cache_opts.d_way_width;
        way_mask = &mips_cache_opts.d_way_mask;
        no_of_lines = &mips_cache_opts.d_no_of_lines;
        offset_width = &mips_cache_opts.d_offset_width;
        index_width = &mips_cache_opts.d_index_width;
        index_mask = &mips_cache_opts.d_index_mask;
    }
    /* I-cache */
    else if(which_cache == 'i')
    {
        pstrcpy(mips_cache_opts.i_opt,15,arg);
        mips_cache_opts.use_i = 1;
        replacement = &mips_cache_opts.i_replacement;
        way_width = &mips_cache_opts.i_way_width;
        way_mask = &mips_cache_opts.i_way_mask;
        no_of_lines = &mips_cache_opts.i_no_of_lines;
        offset_width = &mips_cache_opts.i_offset_width;
        index_width = &mips_cache_opts.i_index_width;
        index_mask = &mips_cache_opts.i_index_mask;
    }
    /* L2-cache */
    else if(which_cache == 'u')
    {
        pstrcpy(mips_cache_opts.l2_opt,15,arg);
        mips_cache_opts.use_l2 = 1; 
        replacement = &mips_cache_opts.l2_replacement;
        way_width = &mips_cache_opts.l2_way_width;
        way_mask = &mips_cache_opts.l2_way_mask;
        no_of_lines = &mips_cache_opts.l2_no_of_lines;
        offset_width = &mips_cache_opts.l2_offset_width;
        index_width = &mips_cache_opts.l2_index_width;
        index_mask = &mips_cache_opts.l2_index_mask;
    }
    else
    {
        fprintf(stderr,
            "*** Error: Unrecognised cache in proc_mips_cache_opt - which_cache\n");
        return 1;
    }

    /* Process option arguments */
    opt_len = sscanf(arg,"%ux%u_%2s_%3s", &tmp0, &tmp1, tmp2, tmp4);
    if( opt_len != 4 && opt_len != 3)
    {
        fprintf(stderr,
            "*** Error: Cannot process arguments of %c-cache\n",which_cache);
        return 1;        
    }

    
    /* Check no of words in each cache line and number of lines */
    if(which_cache != 'u') /* d-cache and i-cache */
    {
        if(tmp0 != 1 && tmp0 != 2 && tmp0 != 4 && tmp0 != 8)
        {
            fprintf(stderr,
                "*** Error: Wrong number of words in a line of %c-cache: %u\n",
                which_cache, tmp0);
            return 1;       
        }
        /* 64 to 8192 lines (increased by power of two) */
        if(tmp1 != 1 << 6 && tmp1 != 1 << 7 && tmp1 != 1 << 8 && tmp1 != 1 << 9 &&
             tmp1 != 1 << 10 && tmp1 != 1 << 11 && tmp1 != 1 << 12 && tmp1 != 1 << 13)
        {
            fprintf(stderr,
                "*** Error: Wrong number of lines in %c-cache: %u\n",
                which_cache, tmp1);
            return 1;     
        }        
    }
    else /* L2-cache */
    {
        if(tmp0 != 4 && tmp0 != 8 && tmp0 != 16 && tmp0 != 32)
        {
            fprintf(stderr,
                "*** Error: Wrong number of words in a line of L2 cache: %u\n",
                tmp0);
            return 1;       
        }
        /* 2048 to 262144 lines (increased by power of two) */
        if(tmp1 != 1 << 11 && tmp1 != 1 << 12 && tmp1 != 1 << 13 && tmp1 != 1 << 14 &&
             tmp1 != 1 << 15 && tmp1 != 1 << 16 && tmp1 != 1 << 17 && tmp1 != 1 << 18)
        {
            fprintf(stderr,
                "*** Error: Wrong number of lines in L2 cache: %u\n",
                tmp1);
            return 1;     
        }     
    }

    /* Check cache size */
    tmp3 = tmp0 * 32 * tmp1;
    if(which_cache != 'u') /* d-cache and i-cache */
    {
        /* 2kB - 32kB */
        if(tmp3 != 1 << 14 && tmp3 != 1 << 15 && tmp3 != 1 << 16 && 
            tmp3 != 1 << 17 && tmp3 != 1 << 18)
        {
            fprintf(stderr,
                "*** Error: Unsupported size of %c-cache: %u\n",
                which_cache, tmp3);
            return 1;       
        }
    }
    else /* L2-cache */
    {
        /* 256kB - 4MB */
        if(tmp3 != 1 << 21 && tmp3 != 1 << 22 && tmp3 != 1 << 23 && 
            tmp3 != 1 << 24 && tmp3 != 1 << 25)
        {
            fprintf(stderr,
                "*** Error: Unsupported size of L2 cache: %u\n",
                tmp3);
            return 1;       
        }    
    }
    
    /* Extra 2 bits to account for byte address */
    *offset_width = gdp_log2(tmp0) + 2;
    
    *index_width = gdp_log2(tmp1);
    *no_of_lines = tmp1;
    *index_mask = tmp1 - 1;

    /* Check if line size of L2 is greater than line size of L1 */
    if((which_cache != 'u' && mips_cache_opts.use_l2
        && *offset_width > mips_cache_opts.l2_offset_width) ||
        (which_cache == 'u' &&
          ((mips_cache_opts.use_i &&
            *offset_width < mips_cache_opts.i_offset_width) ||
          (mips_cache_opts.use_d &&
            *offset_width < mips_cache_opts.d_offset_width))
        )
    )
    {
      fprintf(stderr,"*** Error: Number of entries in a L2 cache line must be" 
          " equal or greater than \nno. of entries in a L1 cache line\n");
      return 1;      
    }

    /* Check cache type */
    if(!strcmp(tmp2,"dm")) {
        *way_width = 0;
    }
    else if(!strcmp(tmp2,"2w")) {
        *way_width = 1;
        *way_mask = (unsigned int *)g_malloc(sizeof(unsigned int));
        (*way_mask)[0] = 1 << (*index_width - 1);
    }
    else if(!strcmp(tmp2,"4w")) {
        *way_width = 2;
        *way_mask = (unsigned int *)g_malloc(3*sizeof(unsigned int));
        (*way_mask)[0] = 1 << (*index_width - 2);
        (*way_mask)[1] = 1 << (*index_width - 1);
        (*way_mask)[2] = (*way_mask)[0] | (*way_mask)[1];          
    }
    else
    {
        fprintf(stderr,
            "*** Error: Unrecognised type of %c-cache\n",which_cache);
        return 1;       
    }

    /* Check replacement algorithm */
    if(opt_len == 3) { /* No replacement passed */
        if(!*way_width == 0) {/* not direct-mapped */
            fprintf(stderr,
                "*** Error: Specify replacement algorithm of %c-cache\n",which_cache);
            return 1; 
        }
        else
          *replacement = 0;
    }
    else if(!strcmp(tmp4,"lru")) {
        *replacement = 2;
    }
    else if(!strcmp(tmp4,"lfu")) {
        *replacement = 4;
    }
    else if(!strcmp(tmp4,"rnd")) {
        *replacement = 6;
    }
    else
    {
        fprintf(stderr,
            "*** Error: Unrecognised replacement algorithm of %c-cache\n",which_cache);
        return 1;       
    }


    // Another check for number of lines (required for MIPS format: 64 x 2^S)
    // as S = i_index_width - 6 - mips_cache_opts.i_way_width
    // TODO: ?Change command line options to specify number of lines per way
    // rather than total number of lines?
    if(*index_width < 6 + *way_width)
    {
        fprintf(stderr,
            "*** Error: Number of lines %c-cache too small for chosen cache type\n"
            ,which_cache);
        return 1;   
    }    

    // Allocate memory for hit/miss counters
    if (which_cache == 'd') {
        mips_cache_opts.d_ld_hit_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
        mips_cache_opts.d_ld_miss_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
        mips_cache_opts.d_st_hit_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
        mips_cache_opts.d_st_miss_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
    }
    else if (which_cache == 'i') {
        mips_cache_opts.i_hit_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
        mips_cache_opts.i_miss_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
    }
    else if (which_cache == 'u') {
        mips_cache_opts.l2_hit_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
        mips_cache_opts.l2_miss_cnt = (uint64_t *)calloc(*no_of_lines, sizeof(uint64_t));
    }


    return 0; 
#endif
}


void log_cache_data(void)
{
#ifdef TARGET_MIPS
    int i;
    if (mips_cache_opts.use_i) {
        char fname[60] = "log-icache-";
        pstrcat(fname, 60, mips_cache_opts.i_opt);
        FILE *fd = fopen(fname, "w");

        if (fd) {
            printf("Logging icache data (%s)\n", fname);
            
            for (i=0; i<mips_cache_opts.i_no_of_lines; i++) {
                fprintf(fd, "%d,%"PRIu64",%"PRIu64"\n", i,
                    mips_cache_opts.i_hit_cnt[i], mips_cache_opts.i_miss_cnt[i]);
            }

            fclose(fd);
        }
        else {
            printf("Failed to open file %s\n", fname);
        }

        free(mips_cache_opts.i_hit_cnt);
        free(mips_cache_opts.i_miss_cnt);
        mips_cache_opts.i_hit_cnt = NULL;
        mips_cache_opts.i_miss_cnt = NULL;

    }

    if (mips_cache_opts.use_d) {
        char fname[60] = "log-dcache-";
        pstrcat(fname, 60, mips_cache_opts.d_opt);
        FILE *fd = fopen(fname, "w");

        if (fd) {
            printf("Logging dcache data (%s)\n", fname);

            for (i=0; i<mips_cache_opts.d_no_of_lines; i++) {
                fprintf(fd, "%d,%"PRIu64",%"PRIu64",%"PRIu64",%"PRIu64"\n", i,
                    mips_cache_opts.d_st_hit_cnt[i], mips_cache_opts.d_st_miss_cnt[i],
                    mips_cache_opts.d_ld_hit_cnt[i], mips_cache_opts.d_ld_miss_cnt[i]);
            }

            fclose(fd);
        }
        else {
            printf("Failed to open file %s\n", fname);
        }

        free(mips_cache_opts.d_ld_hit_cnt);
        free(mips_cache_opts.d_st_hit_cnt);
        free(mips_cache_opts.d_ld_miss_cnt);
        free(mips_cache_opts.d_st_miss_cnt);

        mips_cache_opts.d_ld_hit_cnt = NULL;
        mips_cache_opts.d_st_hit_cnt = NULL;
        mips_cache_opts.d_ld_miss_cnt = NULL;
        mips_cache_opts.d_st_miss_cnt = NULL;

    }

    if (mips_cache_opts.use_l2) {
        char fname[60] = "log-l2cache-";
        pstrcat(fname, 60, mips_cache_opts.l2_opt);
        FILE *fd = fopen(fname, "w");

        if (fd) {
            printf("Logging L2 cache data (%s)\n", fname);
            
            for (i=0; i<mips_cache_opts.l2_no_of_lines; i++) {
                fprintf(fd, "%d,%"PRIu64",%"PRIu64"\n", i,
                    mips_cache_opts.l2_hit_cnt[i], mips_cache_opts.l2_miss_cnt[i]);
            }

            fclose(fd);
        }
        else {
            printf("Failed to open file %s\n", fname);
        }

        free(mips_cache_opts.l2_hit_cnt);
        free(mips_cache_opts.l2_miss_cnt);
        mips_cache_opts.l2_hit_cnt = NULL;
        mips_cache_opts.l2_miss_cnt = NULL;

    }
#endif
}

void set_hw_cache_config(void)
{
    mips_cache_opts.hw_cache_config = 1;
}

/* Returns 0 if n is 0 */
unsigned int gdp_log2(unsigned int n)
{
    unsigned int tmp = 0;
    if(n == 0)
    {
        fprintf(stderr, "***Warning in gdp_log2(), log2 of 0 does not exist!\n");
        return 0;
    }
    n = n >> 1;
    while(n)
    {
        tmp = tmp + 1;
        n = n >> 1;
    }
    return tmp;  
}

/* ------------------------------------------------------------------------- */
/* Allowed cache types and sizes for dcache and icache:  */

/*2kB:*/
/*1x512_tp_rep 2x256_tp_rep 4x128_tp_rep 8x64_tp_rep*/

/*4kB:*/
/*1x1024_tp_rep 2x512_tp_rep 4x256_tp_rep 8x128_tp_rep*/

/*8kB:*/
/*1x2048_tp_rep 2x1024_tp_rep 4x512_tp_rep 8x256_tp_rep*/

/*16kB:*/
/*1x4096_tp_rep 2x2048_tp_rep 4x1024_tp_rep 8x512_tp_rep*/

/*32kB:*/
/*1x8192_tp_rep 2x4096_tp_rep 4x2048_tp_rep 8x1024_tp_rep*/

/* Allowed cache types and sizes for l2cache:  */

/*256kB:*/
/*4x16384_tp_rep 8x8192_tp_rep 16x4096_tp_rep 32x2048_tp_rep*/

/*512kB:*/
/*4x32768_tp_rep 8x16384_tp_rep 16x8192_tp_rep 32x4096_tp_rep*/

/*1MB:*/
/*4x65536_tp_rep 8x32768_tp_rep 16x16384_tp_rep 32x8192_tp_rep*/

/*2MB:*/
/*4x131072_tp_rep 8x65536_tp_rep 16x32768_tp_rep 32x16384_tp_rep*/
/*4MB:*/
/*4x262144_tp_rep 8x131072_tp_rep 16x65536_tp_rep 32x32768_tp_rep*/

