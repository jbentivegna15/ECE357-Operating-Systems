#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *term;
    int c;
    int count = 0;
    char *word = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&word, &len, stdin)) != -1) {
        printf("%s", word);
        count++;
        if (count == 23) {
            printf("---Press RETURN for more---");
            term = fopen("/dev/tty", "w+");
            c = getc(term);
            if (c == 113 || c == 81) exit(0);
            count = 0;
        }
    }

    return 0;
}
