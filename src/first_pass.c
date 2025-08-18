#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "first_pass.h"
#include "preprocess.h"
#include "hashtable.h"

#define MAX_LINE_LEN 256

// Helper function to check if a line is a label
static int isLabel(const char* line) {
    size_t len = strlen(line);
    return (len > 0 && line[len - 1] == ':');
}

// Implementation of the first pass
void firstPass(FILE* preprocessed, struct hashTable* symbolTable) {
    char line[MAX_LINE_LEN];
    uint16_t PC = 0; // Initialize the program counter

    while (fgets(line, MAX_LINE_LEN, preprocessed)) {
        preprocessString(line); // Ensure the line is clean

        // Skip empty lines
        if (line[0] == '\0') {
            continue;
        }

        // Check if the line is a label
        if (isLabel(line)) {
            line[strlen(line) - 1] = '\0'; // Remove the colon at the end of the label

            // Check if the label already exists in the symbol table
            if (search(symbolTable, line) != -1) {
                fprintf(stderr, "Error: Duplicate label \"%s\" found.\n", line);
                exit(EXIT_FAILURE);
            }

            // Insert the label into the symbol table with the current PC value
            insert(symbolTable, line, PC);
        } else {
            // Only increment the program counter for instructions (not directives)
            if (line[0] != '.') {
                PC += 4; // Increment PC by the size of the instruction
            }
        }
    }
}
