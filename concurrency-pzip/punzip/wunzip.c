#include <stdio.h>
#include <stdlib.h>

void decompress(FILE *ifp, FILE *ofp) {
    char c;
    int count;
    while (fread(&count, sizeof count, 1, ifp)) {
        c = fgetc(ifp);
        for(int i = 0; i < count; i++) {
            fputc(c, ofp);
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;

    if (argc <= 1) {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }

    else {
        while (--argc > 0) {
            if ((fp = fopen(*++argv, "r")) == NULL) {
                printf("wunzip: cannot open file\n");
                exit(1);
            }
            else {
                decompress(fp, stdout);
                fclose(fp);
            }
            
        }
    }
    exit(0);
}
