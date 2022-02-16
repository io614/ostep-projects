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

int main(void) {
	char buf[MAXLINE] = {0};
	pid_t pid;
	int status;
        char *argv[MAXARGS] = {0};
        pathv[0] = "/bin";
        char *fullpathptr;

        printf("%s", prompt);
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */

		if ((pid = fork()) < 0) {
			printf("fork error");
                        exit(1);
		} else if (pid == 0) {		/* child */
                        argv[0] = strdup(buf);
                        argv[1] = 0;
                        if ((fullpathptr = getfullpath(buf)) == 0) {
                            printf("couldn't execute: %s\n", buf);
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
