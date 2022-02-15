#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXLEN 1000
#define MAXLINES 10000
#define swapcharptr(a, i, j) {\
    char *tmp = a[j];\
    a[j] = a[i];\
    a[i] = tmp;\
}\

void reverse_linebuf(char *linebuf[], int size) {
    size--;
    int j = 0;
    while (size > j) {
        swapcharptr(linebuf, size, j);
        size--;
        j++;
    }
}
void reverse(FILE *ifp, FILE *ofp) {
    char buf[MAXLEN];
    char *linebuf[MAXLINES];
    int i = 0;
    while (fgets(buf, MAXLEN, ifp)) {
        linebuf[i++] = strdup(buf);
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
