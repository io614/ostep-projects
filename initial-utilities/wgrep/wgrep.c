#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINE 1000

/* line by line */
void filecopymatch(FILE *ifp, FILE *ofp, char *searchterm) {
    char buf[MAXLINE];
    while (fgets(buf, MAXLINE, ifp))  {
        if (strstr(buf, searchterm)) {
            fputs(buf, ofp);
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;
    char *searchterm;

    if (argc <= 1) {
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }

    searchterm = *++argv;
    if (--argc <= 1) {
        fp = stdin;
        filecopymatch(fp, stdout, searchterm);
    }

    else {
        while (--argc > 0) {
            if ((fp = fopen(*++argv, "r")) == NULL) {
                printf("wgrep: cannot open file\n");
                exit(1);
            }
            else {
                filecopymatch(fp, stdout, searchterm);
                fclose(fp);
            }
            
        }
    }
    exit(0);
}
