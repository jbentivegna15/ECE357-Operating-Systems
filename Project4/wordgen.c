#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

int main(int argc, char *argv[]) {
    int num, length;
    char randomletter;
    char word[10];
    srand(time(NULL));

    if (argc > 1) {
        num = atoi(argv[1]);
    } else {
        num = INT_MAX;
    }

    while (num > 0) {
        length = (rand() % (4)) + 3; 
        for (int i = 0; i < length; i++) {
            randomletter = 'A' + (rand() % 26);
            word[i] = randomletter;
        }
        word[length] = '\0';
        printf("%s\n", word);
        num--;
    }
    return 0;
}
