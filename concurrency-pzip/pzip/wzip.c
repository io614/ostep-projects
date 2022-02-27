#include "defs.h"

mmap_file_t *mfiles;
mpage_t mpages[MPAGE_BUFFER_SIZE];
compressed_page_t *cpages;
char *fwrite_buf;

int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
char done_mapping = 0;
pthread_t tid[NTHREADS];

void put(mpage_t value) {
    mpages[fill_ptr] = value; 
    fill_ptr = (fill_ptr + 1) % MPAGE_BUFFER_SIZE;
    count++;
}

mpage_t get() {
    mpage_t value = mpages[use_ptr]; 
    use_ptr = (use_ptr + 1) % MPAGE_BUFFER_SIZE;
    count--;
    return value;
}

pthread_cond_t not_empty, not_full;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long map_mfile(mmap_file_t mfile) {
    mpage_t mpage;
    unsigned long i;
    for (i=0; i*PAGESIZE < mfile.filesize; i++) {
        mpage.index = mfile.starting_index + i;
        mpage.filebufptr = mfile.filebuf + i*PAGESIZE;
        mpage.pagesize= min(PAGESIZE, (mfile.filesize - i*PAGESIZE));
        Pthread_mutex_lock(&lock);
        while (count == MPAGE_BUFFER_SIZE) {
            Pthread_cond_wait(&not_full, &lock);
        }
        put(mpage);
        pthread_cond_signal(&not_empty);
        Pthread_mutex_unlock(&lock);
    }
    return i;
}

void map_mfiles(mmap_file_t mfile_arr[], int nfiles) {
    for (size_t i = 0; i < nfiles; i++) {
        assert(mfile_arr[i].npages == map_mfile(mfile_arr[i]));
    }
}

void fwrite_ic(char *fbuf_ptr, int i, char c) {
    int *i_ptr = (int *) fbuf_ptr;
    char *c_ptr = fbuf_ptr + sizeof (int);
    *i_ptr = i;
    *c_ptr = c;
}
void write_cpage(compressed_page_t cpage, char* fwrite_buf) {
    char *fbuf_ptr = fwrite_buf;
    for (size_t i = 0; i < cpage.o_size; i++) {
        fwrite_ic(fbuf_ptr, cpage.o_count[i], cpage.o_chars[i]);
        fbuf_ptr += SIZEOF_IC;
    }
    free(cpage.o_count);
    free(cpage.o_chars);
    
}
unsigned long write_cpages(compressed_page_t cpages[], int npages, char* fwrite_buf, unsigned long osize_total) {
    compressed_page_t *cptr_a = 0, *cptr_b = 0;
    for (size_t i = 0; i < npages-1; i++) {
        cptr_a = cpages + i;
        cptr_b = cpages + i + 1;
        #define cpage_a_last_letter (cptr_a->o_chars)[cptr_a->o_size-1]
        #define cpage_b_first_letter (cptr_b->o_chars)[0]
        if (cpage_a_last_letter == cpage_b_first_letter) {
            (cptr_a->o_size)--;
            (cptr_b->o_count[0])+=(cptr_a->o_count[cptr_a->o_size]);
            osize_total--;
        }
        write_cpage(*cptr_a, fwrite_buf);
        fwrite_buf += (cptr_a->o_size) * SIZEOF_IC;
    }
    write_cpage(cpages[npages - 1], fwrite_buf);
    return osize_total;
}

int RLE(char i_filebuf[], int i_size, int **o_count_ptr, char **o_chars_ptr) {
    int cpage_size = max(CPAGESIZE_INIT, 1);
    int *o_count;
    char *o_chars;
    assert((o_chars = malloc(cpage_size*sizeof(char))));
    assert((o_count = malloc(cpage_size*sizeof(int))));
    char counting = 0;
    char current = 0;
    int counter = 0;
    int o_ptr = 0;
    # define writecount \
            if (counting) {\
                o_count[o_ptr] = counter;\
                o_chars[o_ptr] = counting;\
                o_ptr++;\
            }\

    for (int i = 0; i < i_size; i++) {
        current = i_filebuf[i];
        if (current == counting) {
            counter++;
        }
        else {
            writecount;
            if (o_ptr >= cpage_size) {
                cpage_size = min(2*cpage_size, PAGESIZE);
                assert((o_chars = realloc(o_chars, cpage_size*sizeof(char))));
                assert((o_count = realloc(o_count, cpage_size*sizeof(int))));
            }
            
            counting = current;
            counter = 1;
        }
    }
    writecount;
    *o_chars_ptr = o_chars;
    *o_count_ptr = o_count;
    return o_ptr;
}

compressed_page_t mpage_to_cpage(mpage_t mpage) {
    compressed_page_t cpage;
    cpage.o_size = RLE(mpage.filebufptr, mpage.pagesize, &cpage.o_count, &cpage.o_chars);
    return cpage;
}

void *consumer(void *arg) {
    unsigned long o_size_total = 0;
    mpage_t mpage;
    compressed_page_t cpage;
    while (1) {
        Pthread_mutex_lock(&lock);
        while (count==0) {
            if (done_mapping) {
                Pthread_mutex_unlock(&lock);
                return (void *) o_size_total;
            }
            Pthread_cond_wait(&not_empty, &lock);
        }
        mpage = get();
        if (!done_mapping) Pthread_cond_signal(&not_full);
        Pthread_mutex_unlock(&lock);
        cpage = mpage_to_cpage(mpage);
        cpages[mpage.index] = cpage;
        o_size_total += cpage.o_size;
    }
}

int main(int argc, char *argv[]) {
    int fd, nfiles, nconsumers;
    nconsumers = NTHREADS - 1;
    struct stat sb;

    if (argc <= 1) {
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    nfiles = (argc - 1);
    mfiles = malloc(nfiles * sizeof (mmap_file_t));

    int npages, size;
    void *fileptr;
    unsigned long page_counter = 0;
    mmap_file_t *mmap_file_ptr;
    for (int i = 1; i < argc; i++) {
        if ((fd = open(argv[i], O_RDONLY)) == -1) {
            exit(1);
        }
        else {
            fstat(fd, &sb);
            size = sb.st_size;
            mmap_file_ptr = &mfiles[i-1];
            mmap_file_ptr->starting_index = page_counter;
            mmap_file_ptr->filesize = size;
            npages = ( size / PAGESIZE) + !!( size % PAGESIZE);
            mmap_file_ptr->npages = npages;
            mmap_file_ptr->filebuf = (char *) (fileptr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0));
            assert( ((long) fileptr) >= 0);
            page_counter += npages;
            close(fd);
        }
    }

    cpages = malloc( page_counter * sizeof (compressed_page_t));


    for (int i = 0; i < nconsumers; i++) {
        Pthread_create(&tid[i], NULL, consumer, NULL);
    }
    
    // start producer thread (main)
    map_mfiles(mfiles, nfiles);
    Pthread_mutex_lock(&lock);
    done_mapping = 1;
    pthread_cond_broadcast(&not_empty);
    Pthread_mutex_unlock(&lock);

    unsigned long osize = 0; // number of runs
    unsigned long osize_total = 0; 
    // join threads
    for (int i = 0; i < nconsumers; i++) {
        Pthread_join(tid[i], (void **) &osize);
        osize_total += osize;
    }

    char* fwrite_buf = malloc(osize_total*SIZEOF_IC);
    unsigned long osize_merged = write_cpages(cpages, page_counter, fwrite_buf, osize_total);
    fwrite(fwrite_buf, SIZEOF_IC, osize_merged, stdout);

    free(mfiles);
    free(cpages);
    free(fwrite_buf);
    exit(0);
}
