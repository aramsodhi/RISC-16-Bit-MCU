#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "parser.h"

int parseReg(const char* str, uint8_t* out) {
    if (!str) return -1;
    if (str[0] != 'R' && str[0] != 'r') return 0;

    char* end;
    unsigned long val = strtoul(str+1, &end, 10);
    
    if (*end!= '\0' || val<0 || val>15) return 0;
    *out = (uint8_t)val;
    return 1;
}

int parseU16(const char* str, int16_t* out) {
    if (!str) return 0;

    char* end;
    unsigned long val;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        val = strtoul(str+2, &end, 16);
    } else {
        val = strtoul(str, &end, 10);
    }

    if (*end != '\0' && val> 0xFFFF) return 0;
    *out = (uint16_t)val;
    return 1;
}

int parseS16(const char* str, int16_t* out) {
    if (!str) return 0;

    char* end;
    long val;

    // make sure str is in correct format for strtol (for neg numbers)

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        val = strtol(str+2, &end, 16);
    } else{
      val = strtol(str, &end, 10);
    }

    if (*end!= '\0' || val < INT16_MIN || val > INT16_MAX) return 0;
    *out = (int16_t)val;
    return 1;
}

static int testParseReg(void) {
    char* reg1 = "R7";
    char* reg2 = "r2";

    uint8_t reg1num;
    uint8_t reg2num;

    int reg1status = parseReg(reg1, &reg1num);
    int reg2status = parseReg(reg2, &reg2num);

    int status;

    status = (reg1num == 7 \
        && reg2num == 2 \
        && reg1status \
        && reg2status);

    return status;
}

static int testParseU16(void) {
    char* str = "0x3FFF";
    int16_t num;
    int numstatus = parseU16(str, &num);

    int status;
    status = (num == 0x3FFF && numstatus);

    return status;
}


int testParser(void) {
    return (testParseReg() && \
            testParseU16()
    );
}
