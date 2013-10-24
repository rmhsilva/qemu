/* GDP */
/* File holding sizes of caches in MIPS processor */
/* Sizes are set up in vl.c using command line arguments */
/* Pointer to a file is used to dump cache performance info */

struct MipsCacheOpts {
    unsigned char use_d;
    unsigned char use_i;
    unsigned char use_l2;

    unsigned int d_line_size;
    unsigned int d_no_of_lines;
    unsigned char d_type;

    unsigned int i_line_size;
    unsigned int i_no_of_lines;
    unsigned char i_type;

    unsigned int l2_line_size;
    unsigned int l2_no_of_lines;
    unsigned char l2_type;

    FILE *perf_dump;  
};

extern struct MipsCacheOpts mips_cache_opts;

