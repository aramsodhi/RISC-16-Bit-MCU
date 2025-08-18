#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "second_pass.h"
#include "hashtable.h"
#include "assembler.h"

#define MAX_LINE_LEN 256
#define PROGRAM_SIZE 1024

static uint32_t outputWords[PROGRAM_SIZE];
static size_t outputLength = 0;

typedef enum { RR, RI, BR, J, HLT } instr_type_t;

typedef struct {
    char mneumonic[10];
    uint8_t opcode;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t imm;
    instr_type_t type;
} instr_t;

static instr_t program[PROGRAM_SIZE];

enum {
    OP_ADD = 0x00,
    OP_SUB = 0x01,
    OP_AND = 0x02,
    OP_OR = 0x03,
    OP_XOR = 0x04,
    OP_SLL = 0x05,
    OP_SRL = 0x06,
    OP_SRA = 0x07,
    OP_LD = 0x10,
    OP_ST = 0x11,
    OP_J = 0x18,
    OP_JNZ = 0x19,
    OP_JLT = 0x1A,
    OP_JGT = 0x1B,
    OP_HLT = 0x3F
};

/*
32-bit encoding (opcode in lower bits)
[31:16] payload (imm16, addr16, rt4)
[15]    N/A (reserved)
[14:11] rs (4 bits)
[10:7]  rd (4 bits)
[6]     i1 (1 if reg-imm or jump)
[5:0]   opcode (6 bits)
*/
static uint32_t pack32(uint8_t op6, uint8_t i1, uint8_t rd4, uint8_t rs4, uint16_t upper16) {
    return ((uint32_t) upper16 << 16)
        | ((uint32_t)(rs4 & 0x0F) << 11)
        | ((uint32_t)(rd4 & 0x0F) << 7)
        | ((uint32_t)(i1 & 0x01) << 6)
        | (uint32_t)(op6 & 0x3F);
}

// ----- helper functions -----
static char* ltrim(char* s) {
    while(isspace((unsigned char) *s)) ++s;
    return s;
}

// remove trailing whitespace in place
static void rtrim(char* s) {
    size_t n = strlen(s);
    while (n && isspace((unsigned char) s[n-1])) s[--n]=0;
}

static int parseReg(const char* t) {
    if (!t) return -1;
    if (t[0] != 'R' && t[0] != 'r') return -1;

    char* e;
    unsigned long v = strtoul(t+1, &e, 10);
    
    if (*e != '\0' || v<0 || v>15) return -1;
    return (int)v;
}

static int parseU16(const char* t, uint16_t* out) {
    if (!t) return 0;

    char* e;
    unsigned long v;

    if (t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) v=strtoul(t+2, &e, 16);
    else v = strtoul(t, &e, 10);

    if (*e != '\0' && v > 0xFFFF) return 0;
    *out = (uint16_t)v;
    return 1;
}

static int parseS16(const char* t, uint16_t* out) {
    if (!t) return 0;

    char* e;
    long v;

    if (t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) v = strtol(t+2, &e, 16);
    else v = strtol(t, &e, 10);

    if (*e != '\0' || v < INT16_MIN || v > INT16_MAX) return 0;
    *out = (int16_t)v;
    return 1;
}

/*
LI RD IMM   -> ADDI RD R0 IMM
MV RD RS    -> ADD RD RS R0
NEG RD RS   -> SUB RD R0 RS
NOT RD RS   -> XORI RD RS -1
NOP         -> ADD R0 R0 R
*/

// tokenizes OP ARG1 ARG2 ARG3, returns count
// tokens are pointers into line buf
static int splitTokens(char* line, char* tok[], int max_tok) {
    int n = 0;
    char* p = line;

    while (*p && n < max_tok) {
        p = ltrim(p);
        if (!*p) break;

        tok[n++] = p;
        while (*p && !isspace((unsigned char) *p)) ++p;

        if (*p) *p++ = 0;
    }
    return n;
}

static void lowerPseudo(char** mn, char** a1, char** a2, char** a3, char* bufmn, char* bufa1, char* bufa2, char* bufa3, char* bufimm) {
    if (!mn) return;

    if (!strcasecmp(*mn, "LI")) {
        strcpy(bufmn, "ADD"); *mn = bufmn;
        strcpy(bufa2, "R0"); *a2 = bufa2;
        return;
    }

    if (!strcasecmp(*mn, "MV")) {
        strcpy(bufmn, "ADD"); *mn = bufmn;
        strcpy(bufa3, "R0"); *a2=bufa2;
        return;
    }

    if (!strcasecmp(*mn, "NEG")) {
        strcpy(bufmn, "SUB"); *mn = bufmn;
        strcpy(bufa2, "R0"); *a2 = bufa2;
        // shift rs?
    }

    if (!strcasecmp(*mn, "NOT")) {
        strcpy(bufmn, "XOR"); *mn = bufmn;
        strcpy(bufimm, "-1"); *a3 = bufimm;
        return;
    }

    if (!strcasecmp(*mn, "NOP")) {
        strcpy(bufmn, "ADD"); *mn = bufmn;
        strcpy(bufa1, "R0"); *a1 = bufa1;
        strcpy(bufa2, "R0"); *a2 = bufa2;
        strcpy(bufa3, "R0"); *a3 = bufa3;
        return;
    }

}

// symbol table lookup
static int32_t symbolTableLookupInstrIndex(struct hashTable* st, const char* name) {
    // finish this later
}

void secondPass(FILE* preprocessed, FILE* output, struct hashTable* symbolTable) {
    char buf[MAX_LINE_LEN];
    enum {SEC_NONE, SEC_TEXT, SEC_DATA} sec = SEC_NONE;

    while (fgets(buf, sizeof(buf), preprocessed)) {
        rtrim(buf);

        // section switches
        if (!strncmp(buf, ".text", 5)) {sec = SEC_TEXT; continue; }
        if (!strncmp(buf, ".data", 5)) {sec = SEC_DATA; continue; }

        if (sec == SEC_DATA) {
            // handle data section and .word directives
        }

        if (sec == SEC_TEXT) {
            // handle text section
        }

    }
}
