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

#define MAX_LINE_LEN 256
#define PROGRAM_SIZE 1024

#define TEXT_ADDR 0x0040
#define DATA_ADDR 0x0FFF

static uint32_t outputBin[PROGRAM_SIZE];
static size_t outputLength = 0;

/*
32-bit encoding (opcode in lower bits)
[31:16] payload (imm16, addr16, rt4)
[15]    N/A (reserved)
[14:11] rs (4 bits)
[10:7]  rd (4 bits)
[6]     i1 (1 if reg-imm or jump)
[5:0]   opcode (6 bits)
*/
static uint32_t pack32(uint8_t op, uint8_t i, uint8_t rd, uint8_t rs, uint16_t upper16) {
    return ((uint32_t) upper16 << 16)
        | ((uint32_t)(rs & 0x0F) << 11)
        | ((uint32_t)(rd & 0x0F) << 7)
        | ((uint32_t)(i & 0x01) << 6)
        | (uint32_t)(op & 0x3F);
}

/*   
0000 0000 0000 0000 |  0  | 000 0 | 000 0 |  0  | 00 0000 |
       imm 16       | N/A |  rs1  |  rd   |  i  |   op    |
*/

/*
static void printInstruction(uint32_t instr) {
    uint8_t op = (uint8_t) instr & 0x3F;
    uint8_t i = (uint8_t) (instr >> 6) & 0x01;
    uint8_t rd = (uint8_t) (instr >> 7) & 0x0F;
    uint8_t rs = (uint8_t) (instr >> 11) & 0x0F;
    uint16_t imm = (uint16_t) (instr >> 16);
    printf("op: %" PRIu8 \
            " imm: %" PRIu8 \
            " rd: %" PRIu8 \
            " rs: %" PRIu8 \
            " imm: %#x\n", op, i, rd, rs, imm);
}
*/

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
        strcpy(bufa2, "R0"); *a2 = bufa2;
        // shift rs?
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
// 0 = signed, 1 = unsigned, 2 = shift4
static int lookupOpcode(const char* mn, uint8_t* op, instr_kind_t* i, int* p) {
    if      (!strcasecmp(mn, "ADD"))    { *op=OP_ADD; *i=RR,  *p=0; return 1; }
    else if (!strcasecmp(mn, "ADDI"))   { *op=OP_ADD; *i=RI,  *p=0; return 1; }
    else if (!strcasecmp(mn, "SUB"))    { *op=OP_SUB; *i=RR;  *p=0; return 1; }
    else if (!strcasecmp(mn, "SUBI"))   { *op=OP_SUB; *i=RI;  *p=0; return 1; }
    else if (!strcasecmp(mn, "AND"))    { *op=OP_AND; *i=RR;  *p=1; return 1; }
    else if (!strcasecmp(mn, "ANDI"))   { *op=OP_AND; *i=RI;  *p=1; return 1; }
    else if (!strcasecmp(mn, "OR"))     { *op=OP_OR;  *i=RR;  *p=1; return 1; }
    else if (!strcasecmp(mn, "ORI"))    { *op=OP_OR;  *i=RR;  *p=1; return 1; }
    else if (!strcasecmp(mn, "XOR"))    { *op=OP_XOR; *i=RI;  *p=0; return 1; }
    else if (!strcasecmp(mn, "XORI"))   { *op=OP_XOR; *i=RI;  *p=0; return 1; }
    else if (!strcasecmp(mn, "SLL"))    { *op=OP_SLL; *i=RR;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SLLI"))   { *op=OP_SLL; *i=RI;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRL"))    { *op=OP_SRL; *i=RR;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRLI"))   { *op=OP_SRL; *i=RI;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRA"))    { *op=OP_SRA; *i=RR;  *p=2; return 1; }
    else if (!strcasecmp(mn, "SRAI"))   { *op=OP_SRA; *i=RI;  *p=2; return 1; }
    else if (!strcasecmp(mn, "LW"))     { *op=OP_LW;  *i=MEM; *p=0; return 1; } // what imm polocy to use?
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
                if (!parseU16(value, &u)) {
                    fprintf(stderr, "Error: bad .word value %s\n", value);
                    exit(EXIT_FAILURE);
                }

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
            int p;
            
            if (!lookupOpcode(mn, &op, &i, &p)) {
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

                    printf("%s %s %s %s --> %#x %#x %#x %#x\n", mn, a1, a2, a3, op, rd, rs1, rs2);

                    bin = pack32(op, 0, rd, rs1, (uint16_t)rs2);
                    emit32(bin);

                    break;

                case RI:
                    // EX: XORI RD RS1 -1
                    parseReg(a1, &rd);
                    parseReg(a2, &rs1);

                    if (p) parseS16(a3, &imm);
                    else parseU16(a3, &imm);

                    // force value to 4 bits for SH4
                    if (p==2) imm = (int16_t)(imm & 0x3FFF);

                    printf("%s %s %s %s --> %#x %#x %#x %#hx\n", mn, a1, a2, a3, op, rd, rs1, imm);

                    bin = pack32(op, 1, rd, rs1, (uint16_t)imm);
                    emit32(bin);

                    break;

                case MEM:
                    // EX: LW RD ADDR16 -- note 2 operands
                    // EX: SW R2 ADDR16
                    // EX: LA R2 mmio -- where mmio is a .word in data section
                    parseReg(a1, &rd);

                    // assume a2 contains raw address if label doesn't exist in symbol table
                    if ((imm = search(symbolTable, a2)) == -1) {
                        parseU16(a2, &imm);
                    }

                    // FIX ADDRESSING MODE REG-REG FOR MEM INSTRUCTIONS
                    printf("%s %s %s --> %#x %#x %#hx\n", mn, a1, a2, op, rd, imm);

                    bin = pack32(op, 1, rd, 0, imm);
                    emit32(bin);

                    break;

                case BR:
                    // EX: BLT R1 R2 endloop
                    parseReg(a1, &rd);
                    parseReg(a2, &rs1);

                    // assume a3 contains raw address if label doesn't exist in symbol table
                    if ((imm = search(symbolTable, a3)) == -1) {
                        parseU16(a3, &imm);
                    }

                    bin = pack32(op, 1, rd, rs1, imm);
                    emit32(bin);

                    break;

                case J:
                    // unconditional jump
                    // EX: J LABEL / J ADDR

                    // assume a1 contains raw address if label doesn't exist in symbol table
                    if ((imm = search(symbolTable, a1)) == -1) {
                        parseU16(a1, &imm);
                    }

                    bin = pack32(op, 0, 0, 0, imm);
                    emit32(bin);

                    break;

                case HLT:
                    bin = pack32(op, 0, 0, 0, 0);
                    emit32(bin);

                    break;

                default:
                    fprintf(stderr, "Error: undetected instruction format\n");
                    exit(EXIT_FAILURE);
            }
        }
    }

    // write to .bin file -- run unittest later
    uint32_t instr; 
    for (int index = 0; index < outputLength; index++) {
        instr = outputBin[index];
        fwrite(&instr, sizeof(uint32_t), 1, output);
    }
}


/*
// --------- TESTING ----------
void testPack32(void) {
    // test encoding of SUB R3 R1 R2
    uint8_t op = 1;
    uint8_t rd = 3;
    uint8_t rs1 = 1;
    uint8_t rs2 = 2;
    uint32_t bin = pack32(op, 0, rd, rs1, (uint16_t)rs2);

    printInstruction(bin);
    printf("%x\n", bin);
    // 0000 0000 0000 0010 0000 1001 1000 0001
}
*/
