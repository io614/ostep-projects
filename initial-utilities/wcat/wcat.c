#include <stdio.h>
#include <stdlib.h>
#define MAXLINE 1000

/* char by char */
void filecopyc(FILE *ifp, FILE *ofp) {
    int c;
    while ((c = getc(ifp)) != EOF) 
        putc(c, ofp);
}

/* line by line */
void filecopys(FILE *ifp, FILE *ofp) {
    char buf[MAXLINE];
    while (fgets(buf, MAXLINE, ifp)) 
        fputs(buf, ofp);
}

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc <= 1) {
        exit(0);
    }

    else {
        while (--argc > 0) {
            if ((fp = fopen(*++argv, "r")) == NULL) {
                printf("wcat: cannot open file\n");
                exit(1);
            }
            else {
                filecopys(fp, stdout);
                fclose(fp);
            }
            
        }
    }
    exit(0);
}
