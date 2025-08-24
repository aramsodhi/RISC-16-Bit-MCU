#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "parser.h"

void parseReg(const char* str, uint8_t* out) {
    if (!str) {
        fprintf(stderr, "Error - Cannot parse null string '%s'.\n", str);
        exit(EXIT_FAILURE);
    }

    if (str[0] != 'R' && str[0] != 'r') {
        fprintf(stderr, "Error - Invalid register name. Must begin with 'R'/'r'.\n");
        exit(EXIT_FAILURE);
    }

    char* end;
    unsigned long val = strtoul(str+1, &end, 10);
    
    if (*end!= '\0') {
        fprintf(stderr, "Error - Unable to parse token '%s' as register.\n", str);
        exit(EXIT_FAILURE);
    }
    
    else if (val<0 || val>15){
        fprintf(stderr, "Error - Invalid register number. Must be between 0 and 15.\n");
    }

    *out = (uint8_t)val;
}

static void parseU16(const char* str, int16_t* out) {
    if (!str) {
        fprintf(stderr, "Error - Attempted to parse null-string as unsigned 16-bit integer.\n");
        exit(EXIT_FAILURE);
    }

    char* end;
    unsigned long val;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        val = strtoul(str+2, &end, 16);
    } else {
        val = strtoul(str, &end, 10);
    }

    if (*end != '\0') {
        fprintf(stderr, "Error - Unable to parse token '%s' as unsigned 16-bit integer.\n", str);
        exit(EXIT_FAILURE);
    }

    else if (val > 0xFFFF) {
        fprintf(stderr, "Error - value '%s' is too large for unsigned 16-bit integer.\n", str);
        exit(EXIT_FAILURE);
    }
    
    *out = (uint16_t)val;
}

static void parseS16(const char* str, int16_t* out) {
    if (!str) {
        fprintf(stderr, "Error - Attempted to parse null-string as signed 16-bit integer.\n");
        exit(EXIT_FAILURE);
    }

    char* end;
    long val;

    // make sure str is in correct format for strtol (for neg numbers)

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        val = strtol(str+2, &end, 16);
    } else{
        val = strtol(str, &end, 10);
    }

    if (*end!= '\0') {
        fprintf(stderr, "Error - Unable to parse token '%s' as signed 16-bit integer.\n", str);
        exit(EXIT_FAILURE);
    }
    else if (val < INT16_MIN || val > INT16_MAX) {
        fprintf(stderr, "Error - value '%s' is outside 16-bit signed integer range.\n", str);
        exit(EXIT_FAILURE);
    }
    
    *out = (int16_t)val;
}

// 0 = unsigned, 1 = signed, 2 = shift4
void parseImm(const char* str, int16_t* out, int policy) {
    if (policy == 1) parseS16(str, out);
    else if (policy == 0 || policy == 2) parseU16(str, out);
    else {
        fprintf(stderr, "Error - Invalid immediate value policy: %d.\n", policy);
        exit(EXIT_FAILURE);
    }

    if (policy == 2 && *out > 0xF) {
        fprintf(stderr, "Error - Shift value '%d', is too large.\n", *out);
        exit(EXIT_FAILURE);
    }
}
