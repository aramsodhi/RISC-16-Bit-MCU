#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "string_ops.h"

char* ltrim(char* s) {
    while(isspace((unsigned char) *s)) ++s;
    return s;
}

// remove trailing whitespace in place
void rtrimInplace(char* s) {
    size_t n = strlen(s);
    while (n && isspace((unsigned char) s[n-1])) s[--n]=0;
}

// tokenizes OP ARG1 ARG2 ARG3, returns count
// tokens are pointers into line buf
int splitTokens(char* line, char* tok[], int max_tok) {
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
