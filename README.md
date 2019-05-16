# Heap

Implemented a library that interacts with the operating system to perform heap management.

in order to run 

1). Type in make 

2). $ env LD_PRELOAD=lib/libmalloc-ff.so tests/test1

    To run the other heap management schemes replace libmalloc-ff.so with the appropriate library:
    
    Best-Fit: libmalloc-bf.so
    
    First-Fit: libmalloc-ff.so
    
    Next-Fit: libmalloc-nf.so
    
    Worst-Fit: libmalloc-wf.so 
    
 This programs 
 
 Splits and coalescing with memory blocks if needed.
 
 Uses four heap management strategies: Next Fit, Worst Fit, Best Fit, First Fit
 
 keeps track of 
 
• Number of times the user calls malloc successfully

• Number of times the user calls free successfully

• Number of times we reuse an existing block

• Number of times we request a new block

• Number of times we split a block

• Number of times we coalesce blocks

• Number blocks in free list

• Total amount of memory requested

• Maximum size of the heap 

calloc and realloc is also implemented in the code

use by

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size); 
