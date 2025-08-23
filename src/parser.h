#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

int parseReg(const char* str, uint8_t* out);
int parseU16(const char* str, int16_t* out);
int parseS16(const char* str, int16_t* out);

int testParser(void);

#endif
