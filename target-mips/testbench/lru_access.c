// LRU tests
#include "runTests.h"

void do_lru_tests()
{
    int idx, tag, result, line;
    
    idx = 10; tag = 0xbeef8eef;
    result = (*interface_lru.lookup)(icache, idx, tag,
            mips_cache_opts.i_way_mask, mips_cache_opts.i_ways, &line);
    print_line(icache, 10);
    print_line(icache, 138);

    result = (*interface_lru.lookup)(icache, idx, tag,
            mips_cache_opts.i_way_mask, mips_cache_opts.i_ways, &line);
    print_line(icache, 10);
    print_line(icache, 138);

    tag = 0x12345678;
    result = (*interface_lru.lookup)(icache, idx, tag,
            mips_cache_opts.i_way_mask, mips_cache_opts.i_ways, &line);
    print_line(icache, 10);
    print_line(icache, 138);

    tag = 0xFFBBDDBB;
    result = (*interface_lru.lookup)(icache, idx, tag,
            mips_cache_opts.i_way_mask, mips_cache_opts.i_ways, &line);
    print_line(icache, 10);
    print_line(icache, 138);
}