#ifndef ASSEMBLER_H
#include <stdlib.h>

#define ASSEMBLER_H

#define MAX_PROGRAM_SIZE 512

#define OPCODE_ADD  0x00
#define OPCODE_SUB  0x01
#define OPCODE_XOR  0x02
#define OPCODE_AND  0x03
#define OPCODE_OR   0x04
#define OPCODE_SLL  0x05
#define OPCODE_SRL  0x06
#define OPCODE_SRA  0x07
#define OPCODE_JEQ  0x10
#define OPCODE_JNE  0x11
#define OPCODE_JLT  0x12
#define OPCODE_JGT  0x13
#define OPCODE_HLT  0x14

// struct definitions
struct symbolEntry {
    char name[32];
    uint16_t address;
};

struct instruction {
    char* label;
    char* mneumonic;
    char* operand1;
    char* operand2;
    char* operand3;
};

#endif
