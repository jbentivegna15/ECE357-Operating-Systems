#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void signalHandler(int sig) {
    fprintf(stderr, "Signal #%d, \"%s\" received!\n", sig, strsignal(sig));
    exit(sig);
}

int createFile(size_t length) {
    FILE *fp;
    int fd;

    if ((fp = fopen("file.txt", "w+")) == NULL) {
        fprintf(stderr, "Error creating text file: %s\n", strerror(errno));
        exit(255);
    }
    srand(time(NULL));
    for(int i=0; i < length; i++) {
        fprintf(fp, "%d", rand() % 9);
    }
    rewind(fp);
    fd = fileno(fp);

    return fd;
}

int mtest1() {
    char *map;
    size_t length = 4096;
    int fd;
    
    printf("Executing Test #1 (write to r/o mmap):\n");
    fd = createFile(length);
    if ((map = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    printf("map[3]: \'%d\'\n", map[3]);
    printf("writing a \'%d\'\n", map[4]);
    map[3] = map[4];

    if (munmap(map,length) == -1) {
        close(fd);
        fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor: %s\n", strerror(errno));
        exit(255);
    }
    exit(0);
}

int mtest23(int flags) {
    char *map, *testChar;
    size_t length = 4096;
    int fd, offset, ans, buf[1];
    
    if (flags == MAP_SHARED) {
        printf("Executing Test #2 (write to shared mmap):\n");
    } else if (flags == MAP_PRIVATE) {
        printf("Executing Test #3 (write to private mmap):\n");
    } else {
        fprintf(stderr, "Invalid Input to Test function\n");
        exit(255);
    }

    fd = createFile(length);
    if ((map = mmap(NULL, length, PROT_READ|PROT_WRITE, flags, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    offset = 25;
    printf("map[%d]: \'%d\'\n", offset, map[offset]);
    printf("writing a 7\n");
    map[offset] = 7;
    printf("new map[%d]: \'%d\'\n", offset, map[offset]);

    if (lseek(fd, offset, SEEK_SET) == -1) {
        fprintf(stderr, "Error lseek on %d: %s\n", fd, strerror(errno));
        exit(255);
    }
    if (read(fd, buf, 1) == -1) {
        fprintf(stderr, "Error reading from %d: %s\n:", fd, strerror(errno));
        exit(255);
    }
    
    printf("read from file returns: \'%d\'\n", buf[0]);    
    if (buf[0] == 7) {
        ans = 0;
    } else {
        ans = 1;
    }
    printf("Update to the mapped memory with %s %s\n", flags==MAP_SHARED?"MAP_SHARED":"MAP_PRIVATE", ans? "is not visible.":"is visible.");

    if (munmap(map,length) == -1) {
        fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor %s\n", strerror(errno));
        exit(255);
    }

    ans? exit(1):exit(0);
}

int mtest4() {
    size_t length = 6000;
    int fd, offset;
    char *map;
    struct stat sb;
    off_t firstSize, secondSize;

    printf("Executing Test #4 (writing beyond the edge):\n");

    fd = createFile(length);
    if (fstat(fd, &sb) == -1) {
        fprintf(stderr, "Error stating file: %s\n", strerror(errno));
        exit(255);
    }
    firstSize = sb.st_size;
    printf("File size: %lld bytes\n", firstSize);

    if ((map = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
        exit(255);
    }

    offset = length+2;
    map[offset] = 7;

    if (fstat(fd, &sb) == -1) {
        fprintf(stderr,"Error stating file: %s\n", strerror(errno));
        exit(255);
    }
    secondSize = sb.st_size;
    printf("File size now: %lld bytes\n", secondSize);

    if (munmap(map,length) == -1) {
        fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor %s\n", strerror(errno));
        exit(255);
    }

    if (firstSize == secondSize) {
        printf("Size of file does not change\n");
        exit(1);
    } else {
        printf("Size of file does change\n");
        exit(0);
    }
}

int mtest5() {
    size_t length = 6000;
    int fd, offset, ans, buf[1];
    char *map;

    printf("Executing Test #5 (writing into a hole):\n");

    fd = createFile(length);
    if ((map = mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
        exit(255);
    }

    offset = length+1;
    map[offset] = 7;

    if (lseek(fd, (length+16), SEEK_SET) == -1) {
        fprintf(stderr, "Error lseek on %d: %s\n", fd, strerror(errno));
        exit(255);
    }
    buf[0] = 0;
    if (write(fd, buf, 1) == -1) {
        fprintf(stderr, "Error write to %d: %s\n", fd, strerror(errno));
        exit(255);
    }

    if (lseek(fd, length+1, SEEK_SET) == -1) {
        fprintf(stderr, "Error lseek on %d: %s\n", fd, strerror(errno));
        exit(255);
    }
    if (read(fd, buf, 1) == -1) {
        fprintf(stderr, "Error reading from %d: %s\n:", fd, strerror(errno));
        exit(255);
    }

    printf("read from file returns: \'%d\'\n", buf[0]);
    if (buf[0] == 7) {
        ans = 0;
    } else {
        ans = 1;
    }

    if (munmap(map, length) == -1) {
        fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor %s\n", strerror(errno));
        exit(255);
    }

    printf("Writing to a hole in mapped memory is %s\n", ans? "not visible.":"is visible.");

    ans? exit(1):exit(0);
}

int mtest6() {
    size_t length = 2000;
    int fd, offset;
    char *map;

    printf("Executing Test #6 (reading beyond the edge):\n");

    fd = createFile(length);
    if ((map = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) ==MAP_FAILED) {
        fprintf(stderr, "Error mapping file to memory: %s\n", strerror(errno));
        exit(255);
    }

    printf("map[2500]: \'%d\'\n", map[2500]);

    printf("map[5000]: \'%d\'\n", map[5000]);

    if (munmap(map,length) == -1) {
        fprintf(stderr, "Error unmapping file to memory: %s\n", strerror(errno));
        exit(255);
    }
    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file descriptor %s\n", strerror(errno));
        exit(255);
    }

    exit(0);
}
        
int main(int argc, char *argv[]) {

    for(int i=0; i < 32; i++) {
        signal(i, signalHandler);
    }

    if (argc < 2) {
        fprintf(stderr, "Not enough arguments, terminating...");
        return 1;
    } else {
        switch (atoi(argv[1])) {
        case 1:
            mtest1();
            break;
        case 2:
            mtest23(MAP_SHARED);
            break;
        case 3:
            mtest23(MAP_PRIVATE);
            break;
        case 4:
            mtest4();
            break;
        case 5:
            mtest5();
            break;
        case 6:
            mtest6();
            break;
        default:
            fprintf(stderr, "Invalid input. Valid inputs: 1-6");
            return 1;
        }
    }
}
