#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "assembler.h"
#include "parser.h"
#include "preprocess.h"
#include "hashtable.h"
#include "first_pass.h"
#include "second_pass.h"

static void runTests(void) {
    if (!testParser()) {
        fprintf(stderr, "Failure: parser.c\n");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stdout, "Success: parser.c\n");
    }

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "No files to compile. Use: \"asm <file>.\".");
        exit(EXIT_FAILURE);
    }

    // run unittests
    runTests();

    preprocessFile(argv[1], "pre.asm");
    printf("Successfully preprocessed file\n");

    struct hashTable symbolTable;
    initializeHashTable(&symbolTable);

    FILE* preprocessed = fopen("pre.asm", "r");
    if (preprocessed == NULL) {
        fprintf(stderr, "Could not open pre.asm for reading.\n");
        exit(EXIT_FAILURE);
    }

    firstPass(preprocessed, &symbolTable);
    printf("First pass successful.\n");

    fclose(preprocessed);

    preprocessed = fopen("pre.asm", "r");
    FILE* output = fopen("output.bin", "wb");
    if (output == NULL) {
        fprintf(stderr, "Error: could not open output.bin for writing.\n");
        exit(EXIT_FAILURE);
    }

    secondPass(preprocessed, output, &symbolTable);
    printf("Second pass successful.\n");
    
    printf("Machine code written to 'output.bin'\n");

    fclose(preprocessed);
    fclose(output);

    freeHashTable(&symbolTable);

    return EXIT_SUCCESS;
}
