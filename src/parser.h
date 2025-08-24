#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>

void parseReg(const char* str, uint8_t* out);
void parseImm(const char* str, int16_t* out, int policy);

#endif
