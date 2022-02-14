#include <stdio.h>
#include <stdlib.h>
#define writecount(ofp)\
    if (counting) {\
        fwrite(&counter, sizeof counter, 1, ofp);\
        fputc(counting, ofp);\
    }\

char counting = 0;
char current = 0;
int counter = 0;

void compress(FILE *ifp, FILE *ofp) {
    while ((current = fgetc(ifp)) != EOF) {
        if (current == counting)
            counter++;
        else {
            writecount(ofp);
            counting = current;
            counter = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc <= 1) {
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    else {
        while (--argc > 0) {
            if ((fp = fopen(*++argv, "r")) == NULL) {
                printf("wzip: cannot open file\n");
                exit(1);
            }
            else {
                compress(fp, stdout);
                fclose(fp);
            }
            
        }
    }
    writecount(stdout);
    exit(0);
}
