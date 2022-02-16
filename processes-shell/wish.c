#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXLINE 1000
#define MAXPATHS 1000
#define MAXARGS 100

char *pathv[MAXPATHS] = {0};
char fullpath[MAXLINE];
char *wargv[MAXARGS] = {0};

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
    if ((wargv[i] = strtok(buf, delim)))
        while ((wargv[++i] = strtok(NULL, delim)))
            ;
            
    return i;
}

void err() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

int main(int argc, char *argv[]) {
	char buf[MAXLINE] = {0};
	pid_t pid;
	int status;
        pathv[0] = "/bin";
        char *fullpathptr;
        int wargc;
        FILE *ifp = stdin;
        char *prompt = "wish> ";

        if (argc > 1) {
            prompt = "";
            ifp = fopen(argv[1], "r");
        }

        printf("%s", prompt);
	while (fgets(buf, MAXLINE, ifp) != NULL) {
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */
                wargc = tokenize_args(buf);

                // inbuilt shell commands
                // exit
                if (!strcmp(wargv[0], "exit")) {
                    if (wargc != 1) {
                        err();
                    }
                    exit(0);
                }

                // cd
                if (!strcmp(wargv[0], "cd")) {
                    if (wargc != 2 || chdir(wargv[1]) < 0) {
                        err();
                    }
                    printf("%s", prompt);
                    continue;
                }

                //path
                if (!strcmp(wargv[0], "path")) {
                    for (int i = 1; i < wargc; i++) {
                        pathv[i-1] = strdup(wargv[i]);
                    }

                    pathv[wargc-1] = NULL;
                    printf("%s", prompt);
                    continue;
                }

                // fork new process
		if ((pid = fork()) < 0) {
			printf("fork error");
                        exit(1);
		} else if (pid == 0) {		/* child */
                        if ((fullpathptr = getfullpath(wargv[0])) == 0) {
                            err();
                            exit(1);
                        }

			execv(fullpathptr, wargv);
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
