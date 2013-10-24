/* GDP  */
/* File includes functions calculating offset and index widths */
/* Allowed cache types and sizes;  */
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

#include <string.h>
#include <stdio.h>
#include "mips-cache-opts.h"

/* which_cache; */
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
        fprintf(stderr,"Unrecognised cache in proc_mips_cache_opt - which_cache\n");
        return 1;
    }

    /* Process option argument */

    if(!strcmp(arg,"1x512_dm")){*offset_width=2;*index_width=9;*type = 'd';}
    else if(!strcmp(arg,"1x512_2w")){*offset_width=2;*index_width=9;*type = '2';}
    else if(!strcmp(arg,"1x512_4w")){*offset_width=2;*index_width=9;*type = '4';}
    else if(!strcmp(arg,"2x256_dm")){*offset_width=3;*index_width=8;*type='d';}
    else if(!strcmp(arg,"2x256_2w")){*offset_width=3;*index_width=8;*type='2';}
    else if(!strcmp(arg,"2x256_4w")){*offset_width=3;*index_width=8;*type='4';}
    else if(!strcmp(arg,"4x128_dm")){*offset_width=4;*index_width=7;*type='d';}
    else if(!strcmp(arg,"4x128_2w")){*offset_width=4;*index_width=7;*type='2';}
    else if(!strcmp(arg,"4x128_4w")){*offset_width=4;*index_width=7;*type='4';}
    else if(!strcmp(arg,"8x64_dm")){*offset_width=5;*index_width=6;*type='d';}
    else if(!strcmp(arg,"8x64_2w")){*offset_width=5;*index_width=6;*type='2';}
    else if(!strcmp(arg,"8x64_4w")){*offset_width=5;*index_width=6;*type='4';}
    else if(!strcmp(arg,"1x1024_dm")){*offset_width=2;*index_width=10;*type='d';}
    else if(!strcmp(arg,"1x1024_2w")){*offset_width=2;*index_width=10;*type='2';}
    else if(!strcmp(arg,"1x1024_4w")){*offset_width=2;*index_width=10;*type='4';}
    else if(!strcmp(arg,"2x512_dm")){*offset_width=3;*index_width=9;*type='d';}
    else if(!strcmp(arg,"2x512_2w")){*offset_width=3;*index_width=9;*type='2';}
    else if(!strcmp(arg,"2x512_4w")){*offset_width=3;*index_width=9;*type='4';}
    else if(!strcmp(arg,"4x256_dm")){*offset_width=4;*index_width=8;*type='d';}
    else if(!strcmp(arg,"4x256_2w")){*offset_width=4;*index_width=8;*type='2';}
    else if(!strcmp(arg,"4x256_4w")){*offset_width=4;*index_width=8;*type='4';}
    else if(!strcmp(arg,"8x128_dm")){*offset_width=5;*index_width=7;*type='d';}
    else if(!strcmp(arg,"8x128_2w")){*offset_width=5;*index_width=7;*type='2';}
    else if(!strcmp(arg,"8x128_4w")){*offset_width=5;*index_width=7;*type='4';}
    else if(!strcmp(arg,"1x2048_dm")){*offset_width=2;*index_width=11;*type='d';}
    else if(!strcmp(arg,"1x2048_2w")){*offset_width=2;*index_width=11;*type='2';}
    else if(!strcmp(arg,"1x2048_4w")){*offset_width=2;*index_width=11;*type='4';}
    else if(!strcmp(arg,"2x1024_dm")){*offset_width=3;*index_width=10;*type='d';}
    else if(!strcmp(arg,"2x1024_2w")){*offset_width=3;*index_width=10;*type='2';}
    else if(!strcmp(arg,"2x1024_4w")){*offset_width=3;*index_width=10;*type='4';}
    else if(!strcmp(arg,"4x512_dm")){*offset_width=4;*index_width=9;*type='d';}
    else if(!strcmp(arg,"4x512_2w")){*offset_width=4;*index_width=9;*type='2';}
    else if(!strcmp(arg,"4x512_4w")){*offset_width=4;*index_width=9;*type='4';}
    else if(!strcmp(arg,"8x256_dm")){*offset_width=5;*index_width=8;*type='d';}
    else if(!strcmp(arg,"8x256_2w")){*offset_width=5;*index_width=8;*type='2';}
    else if(!strcmp(arg,"8x256_4w")){*offset_width=5;*index_width=8;*type='4';}     
    else if(!strcmp(arg,"1x4096_dm")){*offset_width=2;*index_width=12;*type='d';} 
    else if(!strcmp(arg,"1x4096_2w")){*offset_width=2;*index_width=12;*type='2';} 
    else if(!strcmp(arg,"1x4096_4w")){*offset_width=2;*index_width=12;*type='4';} 
    else if(!strcmp(arg,"2x2048_dm")){*offset_width=3;*index_width=11;*type='d';} 
    else if(!strcmp(arg,"2x2048_2w")){*offset_width=3;*index_width=11;*type='2';} 
    else if(!strcmp(arg,"2x2048_4w")){*offset_width=3;*index_width=11;*type='4';} 
    else if(!strcmp(arg,"4x1024_dm")){*offset_width=4;*index_width=10;*type='d';} 
    else if(!strcmp(arg,"4x1024_2w")){*offset_width=4;*index_width=10;*type='2';} 
    else if(!strcmp(arg,"4x1024_4w")){*offset_width=4;*index_width=10;*type='4';} 
    else if(!strcmp(arg,"8x512_dm")){*offset_width=5;*index_width=9;*type='d';} 
    else if(!strcmp(arg,"8x512_2w")){*offset_width=5;*index_width=9;*type='2';} 
    else if(!strcmp(arg,"8x512_4w")){*offset_width=5;*index_width=9;*type='4';} 
    else if(!strcmp(arg,"1x8192_dm")){*offset_width=2;*index_width=13;*type='d';} 
    else if(!strcmp(arg,"1x8192_2w")){*offset_width=2;*index_width=13;*type='2';} 
    else if(!strcmp(arg,"1x8192_4w")){*offset_width=2;*index_width=13;*type='4';} 
    else if(!strcmp(arg,"2x4096_dm")){*offset_width=3;*index_width=12;*type='d';} 
    else if(!strcmp(arg,"2x4096_2w")){*offset_width=3;*index_width=12;*type='2';} 
    else if(!strcmp(arg,"2x4096_4w")){*offset_width=3;*index_width=12;*type='4';} 
    else if(!strcmp(arg,"4x2048_dm")){*offset_width=4;*index_width=11;*type='d';} 
    else if(!strcmp(arg,"4x2048_2w")){*offset_width=4;*index_width=11;*type='2';} 
    else if(!strcmp(arg,"4x2048_4w")){*offset_width=4;*index_width=11;*type='4';} 
    else if(!strcmp(arg,"8x1024_dm")){*offset_width=5;*index_width=10;*type='d';} 
    else if(!strcmp(arg,"8x1024_2w")){*offset_width=5;*index_width=10;*type='2';} 
    else if(!strcmp(arg,"8x1024_4w")){*offset_width=5;*index_width=10;*type='4';} 
    else
    {
         fprintf(stderr,"*** Error: Invalid cache type or size: '%s'\n",arg);
        return 1;       
    }

    *no_of_lines = 1 << *index_width;
    *index_mask = *index_width - 1;
    
    return 0; 
}


