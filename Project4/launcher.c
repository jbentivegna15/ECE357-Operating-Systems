#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int fds[2], fds2[2];
    pid_t pid, pid2, pid3;
    pid_t opid1, opid2, opid3;
    int i, status;
    struct rusage ru;

    if (pipe(fds) < 0) {
        fprintf(stderr, "Can't create pipe (1): %s\n", strerror(errno));
        return 1;
    }

    if (pipe(fds2) < 0) {
        fprintf(stderr, "Can't creat pipe (2): %s\n", strerror(errno));
        return 1;
    }

    if ((pid = fork()) < 0) {
        fprintf(stderr, "Error forking process (1): %s\n", strerror(errno));
        return 1;
    } else if (pid == 0) {
        if (dup2(fds[1], 1) < 0) {
            fprintf(stderr, "Error duping to pipe: %s\n", strerror(errno));
            return 1;
        }
        close(fds[0]); close(fds[1]); close(fds2[0]); close(fds2[1]);
        if (execlp("./wordgen", "wordgen", argv[1], (char *)NULL) == -1) {
            fprintf(stderr, "Error executing command wordgen.c: %s\n", strerror(errno));
        }
        exit(1);
    } else {
        if ((pid2 = fork()) < 0) {
            fprintf(stderr, "Error forking process (2): %s\n", strerror(errno));
            return 1;
        } else if (pid2 == 0) {
            if (dup2(fds[0], 0) < 0) {
                fprintf(stderr, "Error duping to pipe: %s\n", strerror(errno));
                return 1;
            }
            if (dup2(fds2[1], 1) < 0) {
                fprintf(stderr, "Error duping to pipe: %s\n", strerror(errno));
                return 1;
            }
            close(fds[0]); close(fds[1]); close(fds2[0]); close(fds2[1]);
            if (execlp("./wordsearch", "wordsearch", "dict.txt", (char *)NULL) == -1) {
                fprintf(stderr, "Error executing command wordsearch.c: %s\n", strerror(errno));
            }
            exit(1);
        } else {
            if ((pid3 = fork()) < 0) {
                fprintf(stderr, "Error forking process (3)");
                return 1;
            } else if(pid3 == 0) {
                if (dup2(fds2[0], 0) < 0 ) {
                    fprintf(stderr, "Error duping to pipe: %s\n", strerror(errno));
                    return 1;
                }
                close(fds[0]); close(fds[1]); close(fds2[0]); close(fds2[1]);
                if (execlp("./pager", "pager", (char *)NULL) == -1) {
                    fprintf(stderr, "Error executing command pager.c: %s\n", strerror(errno));
                }
            }
        }
    }

    close(fds[0]); close(fds[1]); close(fds2[0]); close(fds2[1]);

    opid1 = wait3(&status, 0, &ru);
    fprintf(stderr, "Process %d exits with %d\n", opid1, status);
    opid2 = wait3(&status, 0, &ru);
    fprintf(stderr, "Process %d exits with %d\n", opid2, status);
    opid3 = wait3(&status, 0, &ru);
    fprintf(stderr, "Proces %d exits with %d\n", opid3, status);
    
    return 0;
}
