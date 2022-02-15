#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define INIT_LENGTH 2
#define swapcharptr(a, i, j) {\
    char *tmp = a[j];\
    a[j] = a[i];\
    a[i] = tmp;\
}\

void reverse_linebuf(char **linebuf, int size) {
    size--;
    int j = 0;
    while (size > j) {
        swapcharptr(linebuf, size, j);
        size--;
        j++;
    }
}
void reverse(FILE *ifp, FILE *ofp) {
    char *line = NULL;
    int linebufsize = INIT_LENGTH;
    char **linebuf = malloc(INIT_LENGTH * sizeof(char *));
    size_t len = 0;
    ssize_t nread;
    int i = 0;

    while ((nread = getline(&line, &len, ifp)) >= 0) {
        linebuf[i++] = strdup(line);
        if (i == linebufsize) {
            linebufsize *= 2;
            linebuf  = realloc(linebuf, linebufsize * sizeof(char *));
        }
    }

    reverse_linebuf(linebuf, i);
    for (int ii=0; ii < i; ii++){
        fputs(linebuf[ii], ofp);
    }
}

int main(int argc, char *argv[]) {
    FILE *ifp = stdin;
    FILE *ofp = stdout;

    if (argc > 3) {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc >= 2) {
        if ((ifp = fopen(argv[1], "r")) == NULL ) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    if (argc >= 3) {
        if ((ofp = fopen(argv[2], "w")) == NULL ) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            exit(1);
        }

        struct stat sb1, sb2;
        lstat(argv[1], &sb1);
        lstat(argv[2], &sb2);
        if (sb1.st_ino == sb2.st_ino) {
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }

    }


    reverse(ifp, ofp);
    exit(0);
}
