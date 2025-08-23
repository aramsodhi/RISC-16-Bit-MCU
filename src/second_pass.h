#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include <stdio.h>
#include "hashtable.h"

void secondPass(FILE* preprocessed, FILE* output, struct hashTable* symbolTable);

enum {
    OP_ADD = 0x00,
    OP_SUB = 0x01,
    OP_AND = 0x02,
    OP_OR = 0x03,
    OP_XOR = 0x04,
    OP_SLL = 0x05,
    OP_SRL = 0x06,
    OP_SRA = 0x07,
    OP_LW = 0x10,
    OP_SW = 0x11,
    OP_LA = 0x12,
    OP_J = 0x16,
    OP_BNZ = 0x17,
    OP_BLT = 0x18,
    OP_BGT = 0x19,
    OP_HLT = 0x3F
}; // change opcodes when designing datapath schematic

typedef enum { RR, RI, MEM, BR, J, HLT } instr_kind_t;

typedef struct {
    char mneumonic[10];
    uint8_t opcode;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t imm;
    instr_kind_t kind;
} instr_t;

typedef struct {
    uint16_t dest_addr;
    uint16_t soruce_addr;
} fixup_t;

#endif
