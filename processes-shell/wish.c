#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLINE 1000
#define MAXPATHS 1000
#define MAXARGS 100
#define WHITESPACE " \t"

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

int tokenize_args(char buf[], char *argv[], char delim[]) {

    int i = 0;
    if ((argv[i] = strtok(buf, delim)))
        while ((argv[++i] = strtok(NULL, delim)))
            ;
            
    return i;
}

void free_ptr_array(void *arr[]){
    void *ptr;
    for(int i = 0; (ptr = arr[i]); i++){
        free(ptr);
    }
}

void err() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

int main(int argc, char *argv[]) {
	char buf[MAXLINE] = {0};
        char *ssbufptr;
	pid_t sspid, pid;
	int status;
        pathv[0] = strdup("/bin");
        char *fullpathptr;
        int wargc;
        FILE *ifp = stdin;
        char *prompt = "wish> ";
        char *pipev[MAXARGS];
        char *redirectv[MAXARGS];
        int redirectfd;
        char *parav[MAXARGS];
        int sspids[MAXARGS];
        int nsubshells;

        if (argc > 2) {
            err();
            exit(1);
        }
        if (argc == 2) {
            prompt = "";
            if((ifp = fopen(argv[1], "r")) == 0) {
                err();
                exit(1);
            }
        }

        int stdoutdupfd = dup(STDOUT_FILENO);
        int stderrdupfd = dup(STDERR_FILENO);
	while (dup2(stdoutdupfd, STDOUT_FILENO), // restore original stdout fd
               dup2(stderrdupfd, STDERR_FILENO), // restore original stderr fd
               printf("%s", prompt),
               fgets(buf, MAXLINE, ifp) != NULL) {

		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */

                if ((nsubshells = tokenize_args(buf, parav, "&")) == 0) {
                    continue;
                }
                
                int ssi = 1;
                sspid = 0;
                for (;ssi < nsubshells; ssi++) {
                    if ((sspid = fork()) == 0) break;
                    sspids[ssi] = sspid;
                }

                if (ssi == nsubshells) ssi = 0;

                ssbufptr = parav[ssi];

                if (strstr(ssbufptr, ">")) {
                    if ((tokenize_args(ssbufptr, pipev , ">") == 2) &&
                        (tokenize_args(pipev[1], redirectv, WHITESPACE) == 1)) {
                        redirectfd = open(redirectv[0], O_RDWR|O_CREAT|O_TRUNC, 0777);
                        dup2(redirectfd, STDOUT_FILENO);
                        dup2(redirectfd, STDERR_FILENO);
                    }

                    else {
                        err();
                        continue;
                    }
                }

                wargc = tokenize_args(ssbufptr, wargv, WHITESPACE);

                if (wargc == 0) continue;

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
                    continue;
                }

                //path
                if (!strcmp(wargv[0], "path")) {
                    for (int i = 1; i < wargc; i++) {
                        if (pathv[i-1]) free(pathv[i-1]);
                        pathv[i-1] = strdup(wargv[i]);
                    }

                    pathv[wargc-1] = NULL;
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

                /* ss parent */
                if (ssi == 0) {
                    for (int i = 1; i < nsubshells; i++) {
                        waitpid(sspids[i], &status, 0);
                    }
                }
                else exit(0);
	}
	exit(0);
}
