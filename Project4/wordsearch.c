#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

int count;

void sighand(int signum) {
    fprintf(stderr, "Matched %d words\n", count);
    exit(0);
}

void makeUp(char * word) {
    for (int i = 0; i < strlen(word); i++) {
        word[i] = toupper(word[i]);
    }
}
    
int main(int argc, char *argv[]) {
    char *buff[500000];
    FILE *stream;
    char *word = NULL;
    size_t len = 0;
    size_t len2 = 0;
    ssize_t nread;
    ssize_t nread2;
    int ind;

    signal(SIGPIPE, sighand);

    if (argc < 2) {
        fprintf(stderr, "Not enough arguments, terminating...");
        return 1;
    } else {
        if ((stream = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "Error opening file %s: %s\n", argv[1], strerror(errno)); 
            return 1;
        }    
    }

    ind = 0; 
    while ((nread = getline(&buff[ind], &len, stream)) != -1) {
        makeUp(buff[ind]);
        ind++;
    }

    fclose(stream);

    count = 0;
    while ((nread2 = getline(&word, &len2, stdin)) != -1) {
        makeUp(word);
        for (int i = 0; i < ind+1; i++) {
            if (strcmp(word, buff[i]) == 0) {
                printf("%s", word);
                count++;
                break;
            }
        }
    }

    for (int i = 0; i < ind+1; i++) {
        free(buff[i]);
    }

    printf("Matched %d words\n", count);

    return 0;
}
    
