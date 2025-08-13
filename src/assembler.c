#include <stdio.h>
#include "assembler.h"
#include "preprocess.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No files to compile. Use: \"asm <file>.\".");
        exit(EXIT_FAILURE);
    }

    preprocessFile(argv[1], "pre.asm");

    printf("Successfully preprocessed file");

    int PC = 0;

    // read pre.asm line by line
    // for each line
    

    exit(EXIT_SUCCESS);
}
