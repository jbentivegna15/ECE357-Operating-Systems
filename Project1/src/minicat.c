#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    int bufferSize, opt, fdin, fdout, noInput, numInputs, firstInd, n, remainder;
    char *output;
    char *inFile;

    bufferSize = 0;
    output = "";
    noInput = 0;
    numInputs = 0;
    remainder = 0;

    //getopt while loop to find flags
    while ((opt = getopt(argc, argv, "b:o:")) != -1) {
        switch (opt) {
            case 'b':
                bufferSize = atoi(optarg);
                break;
            case 'o':
                output = optarg;
                break;
            default: /* '?' */
                exit(EXIT_FAILURE);
        }
    }

    //instantiation of buffer
    if (bufferSize == 0) {
        bufferSize = 8192;
    }
    char buff[bufferSize];

    //setup of output location
    if (output != "") {
        fdout=open(output, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        if (fdout < 0) {
            fprintf(stderr, "Can't open output file %s for writing: %s\n", output, strerror(errno));
            return -1;
        }
    } else {
        fdout=1;
    }
    
    //determine number of inputs
    if (optind < argc) {
        firstInd = optind;
        while (optind < argc) {
            numInputs++;
            optind++;
        }
        optind = firstInd;
    } else {
        noInput = 1;
        numInputs = 1;
    }

    //loop through the inputs 
    for (int i = 0; i < numInputs; i++) {
    
        //setup input location
        if (noInput == 1) {
            inFile = "-";
        } else {
            inFile = argv[optind+i];
        }
    
        //open file for reading
        if ((strcmp(inFile, "-")) == 0) {
            fdin = 0;
            inFile = "stdin";
        } else if ((fdin = open(inFile, O_RDONLY)) < 0) {
            fprintf(stderr, "Error opening input file %s for reading: %s\n", inFile, strerror(errno)); 
            return -1;
        }

        //read and write file multiple times until the end
        while ((n=read(fdin, buff, sizeof(buff))) != 0) {
            if (n < 0) {
                fprintf(stderr, "Error reading input file %s: %s\n", inFile, strerror(errno));
                return -1;
            } else {
                while (remainder < n) {
                    if ((remainder = write(fdout, buff, n)) < 0) {
                        fprintf(stderr, "Error writing to output file %s: %s\n", output, strerror(errno));
                        return -1;
                    }
                    n -= remainder;
                    remainder = 0;
                }
            }
        }

        //close the input file
        if (fdin != 0) {
            if (close(fdin) < 0) {
                fprintf(stderr, "Error closing input file %s: %s\n", inFile, strerror(errno));
                return -1;
            }
        }
    }

    //close the output file
    if (fdout != 1) {
        if (close(fdout) < 0) {
            fprintf(stderr, "Error closing output file %s: %s\n", output, strerror(errno));
            return -1;
        }
    }
    return 0;
}
