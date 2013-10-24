/* GDP */
/* File holding sizes of caches in MIPS processor */
/* Sizes are set up in vl.c using command line arguments */
/* Pointer to a file is used to dump cache performance info */

/* Cache types:
 * '2' - two way associative 
 * '4' - four way associative
 * 'd' - direct-mapped */

/* offset_width includes LSBs which address bytes in a word */
/* index_mask is a mask after shifting address by offset_width to the right */

#ifndef MIPS_CACHE_OPTS
#define MIPS_CACHE_OPTS

struct MipsCacheOpts {
    char d_opt[10];
    char i_opt[10];
    char l2_opt[10];

    unsigned char use_d;    
    unsigned char d_type;
    unsigned int d_no_of_lines;
    unsigned int d_offset_width;
    unsigned int d_index_width;
    unsigned int d_index_mask;

    unsigned char use_i;
    unsigned char i_type;
    unsigned int i_no_of_lines;
    unsigned int i_offset_width;
    unsigned int i_index_width;
    unsigned int i_index_mask;    

    unsigned char use_l2;    
    unsigned int l2_no_of_lines;
    unsigned char l2_type;
    unsigned int l2_offset_width;
    unsigned int l2_index_width;
    unsigned int l2_index_mask; 

    FILE *perf_dump;  
};

extern struct MipsCacheOpts mips_cache_opts;

unsigned char proc_mips_cache_opt(char which_cache, const char *arg);

#endif


