#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXLINE 1000
#define MAXPATHS 1000
#define MAXARGS 100

char *pathv[MAXPATHS] = {0};
char prompt[] = "wish> ";
char fullpath[MAXLINE];
char *argv[MAXARGS] = {0};

char *getfullpath(char filename[]) {
    char *path;
    int i = 0;
    while((path = pathv[i++])) {
        strcpy(fullpath, path);
        strcat(fullpath, "/");
        strcat(fullpath, filename);
        if (access(fullpath, X_OK) >= 0) {
            return fullpath;
        }
    }
    return (char *) 0;
}

int tokenize_args(char buf[]) {

    int i = 0;
    char delim[] = " \t";
    if ((argv[i] = strtok(buf, delim)))
        while ((argv[++i] = strtok(NULL, delim)))
            ;
            
    return i;
}

int main(void) {
	char buf[MAXLINE] = {0};
	pid_t pid;
	int status;
        pathv[0] = "/bin";
        char *fullpathptr;

        printf("%s", prompt);
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */
                tokenize_args(buf);

                // fork new process
		if ((pid = fork()) < 0) {
			printf("fork error");
                        exit(1);
		} else if (pid == 0) {		/* child */
                        if ((fullpathptr = getfullpath(argv[0])) == 0) {
                            printf("couldn't execute: %s\n", argv[0]);
                            exit(1);
                        }

			execv(fullpathptr, argv);
		}

		/* parent */
                if ((pid = waitpid(pid, &status, 0)) < 0) {
			printf("waitpid error\n");
                        exit(1);
                }
                printf("%s", prompt);
	}
	exit(0);
}
