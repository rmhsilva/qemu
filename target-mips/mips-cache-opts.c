/* GDP  */
/* File includes functions calculating offset and index widths */
/* See allowed cache types and sizes at the end of this file */

#include <string.h>
#include <stdio.h>
#include "mips-cache-opts.h"
#include "cpu.h"

/* Macros for displaying uint64t */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

/* which_cache: */
/*     d - d-cache */
/*     i - i-cache */
/*     u - unified L2 cache */

/*  GDP cache options struct MIPS  */
struct MipsCacheOpts mips_cache_opts = {"","","",0};

unsigned char proc_mips_cache_opt(char which_cache, const char *arg)
{  
    unsigned char *type;
    unsigned int *no_of_lines;
    /* offset width includes bits used to address bytes in a word */
    unsigned int *offset_width;
    unsigned int *index_width;
    unsigned int *index_mask;
    unsigned int tmp0, tmp1, tmp3;
    char tmp2[10];


    /* D-cache */
    if(which_cache == 'd')
    {
        mips_cache_opts.use_d = 1; 
        type = &mips_cache_opts.d_type;
        no_of_lines = &mips_cache_opts.d_no_of_lines;
        offset_width = &mips_cache_opts.d_offset_width;
        index_width = &mips_cache_opts.d_index_width;
        index_mask = &mips_cache_opts.d_index_mask;
    }
    /* I-cache */
    else if(which_cache == 'i')
    {
        mips_cache_opts.use_i = 1;
        type = &mips_cache_opts.i_type;
        no_of_lines = &mips_cache_opts.i_no_of_lines;
        offset_width = &mips_cache_opts.i_offset_width;
        index_width = &mips_cache_opts.i_index_width;
        index_mask = &mips_cache_opts.i_index_mask;
    }
    /* L2-cache */
    else if(which_cache == 'u')
    {
        mips_cache_opts.use_l2 = 1; 
        type = &mips_cache_opts.l2_type;
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
    if(sscanf(arg,"%ux%u_%9s", &tmp0, &tmp1, tmp2) != 3)
    {
        fprintf(stderr,
            "*** Error: Cannot process arguments of %c-cache\n",which_cache);
        return 1;        
    }

    /* Check cache type */
    if(!strcmp(tmp2,"dm")){*type = 'd';}
    else if(!strcmp(tmp2,"2w")){*type = '2';}
    else if(!strcmp(tmp2,"4w")){*type = '4';}
    else
    {
        fprintf(stderr,
            "*** Error: Unrecognised type of %c-cache\n",which_cache);
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
}


void log_cache_data(void)
{
    int i;
    if (mips_cache_opts.use_i) {
        char fname[60] = "log-icache-";
        pstrcat(fname, 60, mips_cache_opts.i_opt);
        FILE *fd = fopen(fname, "w");

        if (fd) {
            printf("Logging icache data (%s)\n", fname);
            
            for (i=0; i<mips_cache_opts.i_no_of_lines; i++) {
                fprintf(fd, "%x,%"PRIx64",%"PRIx64"\n", i,
                    mips_cache_opts.i_hit_cnt[i], mips_cache_opts.i_miss_cnt[i]);
            }

            fclose(fd);
        }
        else {
            printf("Failed to open file %s\n", fname);
        }

        free(mips_cache_opts.i_hit_cnt);
        free(mips_cache_opts.i_miss_cnt);
    }

    if (mips_cache_opts.use_d) {
        char fname[60] = "log-dcache-";
        pstrcat(fname, 60, mips_cache_opts.d_opt);
        FILE *fd = fopen(fname, "w");

        if (fd) {
            printf("Logging dcache data (%s)\n", fname);

            for (i=0; i<mips_cache_opts.d_no_of_lines; i++) {
                fprintf(fd, "%x,%"PRIx64",%"PRIx64",%"PRIx64",%"PRIx64"\n", i,
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
    }

    // TODO: L2
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
/* 2kB: */
/* 1x512_dm 2x256_dm 4x128_dm 8x64_dm */
/* 1x512_2w 2x256_2w 4x128_2w 8x64_2w */
/* 1x512_4w 2x256_4w 4x128_4w 8x64_4w */

/* 4kB: */
/* 1x1024_dm 2x512_dm 4x256_dm 8x128_dm */
/* 1x1024_2w 2x512_2w 4x256_2w 8x128_2w */
/* 1x1024_4w 2x512_4w 4x256_4w 8x128_4w */

/* 8kB: */
/* 1x2048_dm 2x1024_dm 4x512_dm 8x256_dm */
/* 1x2048_2w 2x1024_2w 4x512_2w 8x256_2w */
/* 1x2048_4w 2x1024_4w 4x512_4w 8x256_4w */

/* 16kB: */
/* 1x4096_dm 2x2048_dm 4x1024_dm 8x512_dm */
/* 1x4096_2w 2x2048_2w 4x1024_2w 8x512_2w */
/* 1x4096_4w 2x2048_4w 4x1024_4w 8x512_4w */

/* 32kB: */
/* 1x8192_dm 2x4096_dm 4x2048_dm 8x1024_dm */
/* 1x8192_2w 2x4096_2w 4x2048_2w 8x1024_2w */
/* 1x8192_4w 2x4096_4w 4x2048_4w 8x1024_4w */

/* Allowed cache types and sizes for l2cache:  */

/* 256kB: */
/* 4x16384_dm 8x8192_dm 16x4096_dm 32x2048_dm */
/* 4x16384_2w 8x8192_2w 16x4096_2w 32x2048_2w */
/* 4x16384_4w 8x8192_4w 16x4096_4w 32x2048_4w */

/* 512kB: */
/* 4x32768_dm 8x16384_dm 16x8192_dm 32x4096_dm */
/* 4x32768_2w 8x16384_2w 16x8192_2w 32x4096_2w */
/* 4x32768_4w 8x16384_4w 16x8192_4w 32x4096_4w */

/* 1MB: */
/* 4x65536_dm 8x32768_dm 16x16384_dm 32x8192_dm */
/* 4x65536_2w 8x32768_2w 16x16384_2w 32x8192_2w */
/* 4x65536_4w 8x32768_4w 16x16384_4w 32x8192_4w */

/* 2MB: */
/* 4x131072_dm 8x65536_dm 16x32768_dm 32x16384_dm */
/* 4x131072_2w 8x65536_2w 16x32768_2w 32x16384_2w */
/* 4x131072_4w 8x65536_4w 16x32768_4w 32x16384_4w */

/* 4MB: */
/* 4x262144_dm 8x131072_dm 16x65536_dm 32x32768_dm */
/* 4x262144_2w 8x131072_2w 16x65536_2w 32x32768_2w */
/* 4x262144_4w 8x131072_4w 16x65536_4w 32x32768_4w */

