#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include "common_threads.h"
#define NTHREADS 4
#define PAGESIZE 4096
// #define PAGESIZE 1000
#define MPAGE_BUFFER_SIZE 100000
#define min(a, b) a <= b ? a : b
#define max(a, b) a >= b ? a : b
#define SIZEOF_IC (sizeof (int) + sizeof (char))

typedef struct Mmap_file {
    unsigned long starting_index;
    unsigned long filesize;
    unsigned long npages;
    char *filebuf;
} mmap_file_t;

typedef struct Mpage {
    unsigned long index;
    int pagesize;
    char *filebufptr; // points to start of page in mmapped file buffer
} mpage_t;

# define CPAGESIZE_DIV_FACTOR 16 
# define CPAGESIZE_INIT (PAGESIZE/CPAGESIZE_DIV_FACTOR)

typedef struct Compressed_page {
    int *o_count;
    char *o_chars;
    unsigned long o_size; // number of runs (size of array)
} compressed_page_t;