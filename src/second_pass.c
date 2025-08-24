#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "second_pass.h"
#include "hashtable.h"
#include "assembler.h"
#include "parser.h"
#include "string_ops.h"
#include "pack32.h"

/*
Errors to check for:
Segmentation fault (invalid memory address)


*/


#define MAX_LINE_LEN 256
#define PROGRAM_SIZE 1024

static uint32_t outputBin[PROGRAM_SIZE];
static size_t outputLength = 0;

/*
LI RD IMM   -> ADDI RD R0 IMM
MV RD RS    -> ADD RD RS R0
NEG RD RS   -> SUB RD R0 RS
NOT RD RS   -> XORI RD RS -1
NOP         -> ADD R0 R0 R
*/
static void lowerPseudo(char** mn, char** a1, char** a2, char** a3, char* bufmn, char* bufa1, char* bufa2, char* bufa3, char* bufimm) {
    if (!mn) return;

    // LI RD IMM --> ADDI RD R0 IMM
    if (!strcasecmp(*mn, "LI")) {
        strcpy(bufmn, "ADDI"); *mn = bufmn;
        strcpy(bufa3, *a2); *a3 = bufa3;
        strcpy(bufa2, "R0"); *a2 = bufa2;
        return;
    }

    // MV RD RS1 --> ADDI RD RS1 0x0000
    if (!strcasecmp(*mn, "MV")) {
        strcpy(bufmn, "ADD"); *mn = bufmn;
        strcpy(bufa3, "R0"); *a3=bufa3;
        return;
    }

    // NEG RD RS1 --> SUB RD R0 RS1
    if (!strcasecmp(*mn, "NEG")) {
        strcpy(bufmn, "SUB"); *mn = bufmn;
        strcpy(bufa3, *a2); *a3 = bufa3;
        strcpy(bufa2, "R0"); *a2 = bufa2;
    }

    // NOT RD RS1 --> XORI RD RS1 0xFFFF
    if (!strcasecmp(*mn, "NOT")) {
        strcpy(bufmn, "XOR"); *mn = bufmn;
        strcpy(bufimm, "-1"); *a3 = bufimm;
        return;
    }

    // NOP --> ADDI R0 R0 0x0000
    if (!strcasecmp(*mn, "NOP")) {
        strcpy(bufmn, "ADD"); *mn = bufmn;
        strcpy(bufa1, "R0"); *a1 = bufa1;
        strcpy(bufa2, "R0"); *a2 = bufa2;
        strcpy(bufa3, "R0"); *a3 = bufa3;
        return;
    }
}

static inline void emit32(uint32_t w) {
    if (outputLength >= PROGRAM_SIZE) {
        fprintf(stderr, "Error: output overflow\n");
        exit(EXIT_FAILURE);
    }
    outputBin[outputLength++]=w;
}

// opcode to imm policy lookup
typedef struct {
    uint8_t opcode;
    char* mneuomic;
    instr_kind_t kind;
    int* imm_policy;
} OpcodeInfo;


static int lookupOpcode(const char* mn, uint8_t* op, instr_kind_t* i, int* p) {
    if      (!strcasecmp(mn, "ADD"))    { *op=OP_ADD; *i=RR,  *p=1; return 1; }
    else if (!strcasecmp(mn, "ADDI"))   { *op=OP_ADD; *i=RI,  *p=1; return 1; }
    else if (!strcasecmp(mn, "SUB"))    { *op=OP_SUB; *i=RR;  *p=1; return 1; }
    else if (!strcasecmp(mn, "SUBI"))   { *op=OP_SUB; *i=RI;  *p=1; return 1; }
    else if (!strcasecmp(mn, "AND"))    { *op=OP_AND; *i=RR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "ANDI"))   { *op=OP_AND; *i=RI;  *p=0; return 1; }
    else if (!strcasecmp(mn, "OR"))     { *op=OP_OR;  *i=RR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "ORI"))    { *op=OP_OR;  *i=RI;  *p=0; return 1; }
    else if (!strcasecmp(mn, "XOR"))    { *op=OP_XOR; *i=RR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "XORI"))   { *op=OP_XOR; *i=RI;  *p=0; return 1; }
    else if (!strcasecmp(mn, "SLL"))    { *op=OP_SLL; *i=RR;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SLLI"))   { *op=OP_SLL; *i=RI;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRL"))    { *op=OP_SRL; *i=RR;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRLI"))   { *op=OP_SRL; *i=RI;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRA"))    { *op=OP_SRA; *i=RR;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRAI"))   { *op=OP_SRA; *i=RI;  *p=2; return 1; }
    else if (!strcasecmp(mn, "LW"))     { *op=OP_LW;  *i=MEM; *p=0; return 1; }
    else if (!strcasecmp(mn, "SW"))     { *op=OP_SW;  *i=MEM; *p=0; return 1; }
    else if (!strcasecmp(mn, "LA"))     { *op=OP_LA;  *i=MEM; *p=0; return 1; }
    else if (!strcasecmp(mn, "J"))      { *op=OP_J;   *i=J;   *p=0; return 1; }
    else if (!strcasecmp(mn, "BNZ"))    { *op=OP_BNZ; *i=BR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "BLT"))    { *op=OP_BLT; *i=BR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "BGT"))    { *op=OP_BGT; *i=BR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "HLT"))    { *op=OP_HLT; *i=HLT; *p=0; return 1; }
    return 0;
    // what to do with imm variations?
}


void writeToBin(uint32_t outputBuffer[], FILE* output) {
    uint32_t instr; 
    uint8_t instrbuf[4];
    for (int index = 0; index < outputLength; index++) {
        instr = outputBuffer[index];
        instrbuf[0] = (instr >> 24) & 0xFF;
        instrbuf[1] = (instr >> 16) & 0xFF;
        instrbuf[2] = (instr >> 8) & 0xFF;
        instrbuf[3] = instr & 0xFF;

        //printf("%#x\n", instr);
        fwrite(&instrbuf, sizeof(uint8_t), 4, output);
    }
}

void secondPass(FILE* preprocessed, FILE* output, struct hashTable* symbolTable) {
    char buf[MAX_LINE_LEN];
    enum {SEC_NONE, SEC_TEXT, SEC_DATA} sec = SEC_NONE;

    while (fgets(buf, sizeof(buf), preprocessed)) {
        rtrimInplace(buf);

        // section switches
        if (!strncmp(buf, ".text", 5)) {sec = SEC_TEXT; continue; }
        if (!strncmp(buf, ".data", 5)) {sec = SEC_DATA; continue; }

        if (sec == SEC_DATA) {
            // label: .word 0xFFFF
            char label[64];
            char dotword[16];
            char value[64];
            
            if (sscanf(buf, "%63[^:]: %15s %63s", label, dotword, value) == 3 && !strcmp(dotword, ".word")) {
                int16_t u;
                parseImm(value, &u, 0);

                /*
                if (u == NULL) {
                    fprintf(stderr, "Error: bad .word value %s\n", value);
                    exit(EXIT_FAILURE);
                }
                */

                emit32((uint32_t)u);
                continue;
            }
            fprintf(stderr, "Error: unrecognized .data line %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if (sec == SEC_TEXT) {
            // skip lines with only labels (no instructions)
            size_t bufLen = strlen(buf);
            if (buf[bufLen-1] == ':' && strchr(buf, ' ') == NULL) continue;

            // ignore label for lines with label + instruction
            char* s = buf;
            char* colon = strchr(s, ':');
            if (colon) s = colon+1;
            s = ltrim(s);

            // split line into tokens
            char* tok[4] = {0};
            int ntok = splitTokens(s, tok, 4);
            if (ntok <= 0) continue;

            char* mn = tok[0];
            char* a1 = (ntok > 1 ? tok[1] : NULL);
            char* a2 = (ntok > 2 ? tok[2] : NULL);
            char* a3 = (ntok > 3 ? tok[3] : NULL);

            char bufmn[8];
            char bufa1[8];
            char bufa2[8];
            char bufa3[8];
            char bufimm[8];

            // replace pseudo-ops
            lowerPseudo(&mn, &a1, &a2, &a3, bufmn, bufa1, bufa2, bufa3, bufimm);
            
            uint8_t op;
            instr_kind_t i;
            int policy;
            
            if (!lookupOpcode(mn, &op, &i, &policy)) {
                fprintf(stderr, "Error: unknown mneumonic %s\n", mn);
                exit (EXIT_FAILURE);
            }

            uint8_t rd;
            uint8_t rs1;
            uint8_t rs2;
            int16_t imm;
            uint32_t bin;

            switch (i) {
                case RR:
                    // EX: ADD RD RS1 RS2
                    // EX: SRA R4 R2 R3
                    parseReg(a1, &rd);
                    parseReg(a2, &rs1);
                    parseReg(a3, &rs2);

                    bin = packRR(op, rd, rs1, rs2);
                    emit32(bin);

                    break;

                case RI:
                    // EX: XORI RD RS1 -1
                    parseReg(a1, &rd);
                    parseReg(a2, &rs1);

                    parseImm(a3, &imm, policy);

                    bin = packRI(op, rd, rs1, imm);
                    emit32(bin);

                    break;

                case MEM:
                    // EX: LW RD ADDR16 -- note 2 operands
                    // EX: SW R2 ADDR16
                    // EX: LA R2 mmio -- where mmio is a .word in data section
                    parseReg(a1, &rd);

                    // parse address?

                    // assume a2 contains raw address if label doesn't exist in symbol table
                    if ((imm = search(symbolTable, a2)) == -1) {
                        parseImm(a2, &imm, policy);
                    }

                    // FIX ADDRESSING MODE REG-REG FOR MEM INSTRUCTIONS
                    // printf("%s %s %s --> %#x %#x %#hx\n", mn, a1, a2, op, rd, imm);

                    
                    bin = packMEM(op, rd, imm);
                    emit32(bin);

                    break;

                case BR:
                    // EX: BLT R1 R2 endloop
                    parseReg(a1, &rs1);
                    parseReg(a2, &rs2);

                    // assume a3 contains raw address if label doesn't exist in symbol table
                    if ((imm = search(symbolTable, a3)) == -1) {
                        parseImm(a3, &imm, policy);
                    }

                    bin = packBR(op, rs1, rs2, imm);
                    emit32(bin);

                    break;

                case J:
                    // unconditional jump
                    // EX: J LABEL / J ADDR

                    // assume a1 contains raw address if label doesn't exist in symbol table
                    if ((imm = search(symbolTable, a1)) == -1) {
                        parseImm(a1, &imm, policy);
                    }

                    bin = packJ(op, imm);
                    emit32(bin);

                    break;

                case HLT:
                    bin = packHLT(op);
                    emit32(bin);

                    break;

                default:
                    fprintf(stderr, "Error: undetected instruction format\n");
                    exit(EXIT_FAILURE);
            }
        }
    }

    // write to .bin file
    writeToBin(outputBin, output);
}
