#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"
#define NTHREADS 4
#define NWORKERS (NTHREADS - 1)
#define I_SIZE (sizeof (int))
#define C_SIZE (sizeof (char))
#define IC_SIZE (I_SIZE + C_SIZE)
#define BUFSIZE_INIT_FACTOR 2
#define REALLOC_FACTOR 2
#define min(a, b) a <= b ? a : b
#define max(a, b) a >= b ? a : b

typedef struct Thread_args {
    char *startptr;
    unsigned long n_ic;
} thread_args_t;

pthread_t tid[NWORKERS];
thread_args_t thread_args[NWORKERS];

void *worker(void *args) {
    thread_args_t *thread_args = (thread_args_t *) args;
    unsigned long n_ic = thread_args->n_ic;
    if (n_ic == 0) return (void*) 0;
    int *count_arr = malloc(n_ic * I_SIZE);
    char *c_arr = malloc(n_ic * C_SIZE);
    int count;
    char c;
    int *ic_pos;
    size_t bufsize = 1;

    for (size_t i = 0; i < n_ic; i++) {
        ic_pos = (int*) ((thread_args->startptr) + IC_SIZE*i);
        count =  *ic_pos;
        c = (char) *(ic_pos+1);
        count_arr[i] = count;
        c_arr[i] = c;
        bufsize += count;
    }

    char* buf = malloc(bufsize);

    char *bufhead = buf;
    for (size_t i = 0; i < n_ic; i++) {
        count = count_arr[i];
        c = c_arr[i];
        for (size_t j = 0; j < count; j++) {
            *(bufhead++) = c;
        }
    }
    assert((bufhead == (buf + bufsize - 1)));
    *bufhead = '\0';
    return buf;
}


void decompress(int fd) {
    struct stat sb;
    fstat(fd, &sb);
    char *fileptr;
    assert(sb.st_size % IC_SIZE == 0);
    thread_args_t *tap;
    unsigned long ic_total = sb.st_size / IC_SIZE;
    unsigned long ic_per_worker = ic_total / NWORKERS;
    unsigned long ic_leftover = ic_total % NWORKERS;

    fileptr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    assert(((long) fileptr) >= 0);

    for (int i = 0; i < NWORKERS; i++) {
        tap = &thread_args[i];
        tap->startptr = fileptr + IC_SIZE * i * ic_per_worker;
        tap->n_ic = (i == (NWORKERS - 1)) ? ic_per_worker + ic_leftover : ic_per_worker ;
        Pthread_create(&tid[i], NULL, worker, (void *) tap);
    }

    for (int i = 0; i < NWORKERS; i++) {
        char *buf;
        Pthread_join(tid[i], (void **) &buf);
        if (buf != 0) {
            printf("%s", buf);
            free(buf);
        }
    }
    
}

int main(int argc, char *argv[]) {
    int fd;

    if (argc <= 1) {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    else {
        while (--argc > 0) {
            if ((fd = open(*++argv, O_RDONLY)) == -1) {
                printf("wunzip: cannot open file\n");
                exit(1);
            }
            else {
                decompress(fd);
                close(fd);
            }
            
        }
    }
    exit(0);
}
