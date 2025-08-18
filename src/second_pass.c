#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "second_pass.h"
#include "hashtable.h"
#include "assembler.h"

#define MAX_LINE_LEN 256


typedef enum { RR, RI, BR, J, HLT } type_t;

typedef struct {
    const char* name;
    uint8_t op6;
    type_t f_rr; // rr or hlt if no rr
    type_t f_ri; // ri or j/br if it's a jump or branch
    uint8_t imm_policy; // 0 for no immediate, 1 for ri, 2 for j/br
} opdesc_t;

static const opdesc_t OPTABLE[] = {
    {"ADD", 0x00, RR, RI, 0},
    {"SUB", 0x01, RR, RI, 0},
    {"AND", 0x02, RR, RI, 0},
    {"OR",  0x03, RR, RI, 0},
    {"XOR", 0x04, RR, RI, 0},
    {"SLL", 0x05, RR, RI, 0},
    {"SRL", 0x06, RR, RI, 0},
    {"SRA", 0x07, RR, RI, 0},
    {"J",   0x18, HLT, J, 1},
    {"JNZ", 0x19, HLT, BR, 2},
    {"JLT", 0x1A, HLT, BR, 2},
    {"JGT", 0x1B, HLT, BR, 2},
    {"HLT", 0x3F, HLT, HLT, 0}
};



void secondPass(FILE* preprocessed, FILE* output, struct hashTable* symbolTable) {
    char line[MAX_LINE_LEN];

    while (fgets(line, MAX_LINE_LEN, preprocessed)) {
        // skip empty lines
        if (line[0] == '\0') {
            continue;
        }

        // parse the line (instruction/directive/label)
        // encode instructions
        // handle directives
    }
}
