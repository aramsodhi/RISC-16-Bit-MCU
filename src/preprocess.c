#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "preprocess.h"

// remove comments and whitespace from a line of assmebly
void preprocessString(char* str) {
    // edge case for empty or NULL strings
    if (str == NULL || *str == '\0') {
        return;
    }

    // replace '#' character with a null terminator to get rid of comments
    char* commentAddress = strchr(str, '#');

    if (commentAddress != NULL) {
        *commentAddress = '\0';
    }

    // track index of non-whitespace character
    int lastCharIndex = strlen(str) - 1;

    // iterate backward while updating lastCharIndex
    while (lastCharIndex >= 0 && isspace((unsigned char) str[lastCharIndex])) {
        lastCharIndex--;
    }

    // place null terminater after last non-whitespace char
    str[lastCharIndex + 1] = '\0';


    // index of first non-whitespace char
    int i = 0;
    while(str[i] != '\0' && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')) {
        i++;
    }

    // if all chars are whitespace or string is empty. set to empty string
    if (str[i] == '\0') {
        str[0] = '\0';
        return;
    }

    // shift characters
    int j = 0;
    while (str[i] != '\0') {
        str[j++] = str[i++];
    }

    // null terminate the string
    str[j] = '\0';
}

void preprocessFile(char* _source, char* _dest) {
    FILE* source = fopen(_source, "r");
    FILE* dest = fopen(_dest, "w");
    char lineBuffer[MAX_LINE_LEN];
    
    if (source == NULL) {
        fprintf(stderr, "Could not open \"%s\" for reading. Make sure it exists and has read permissions.\n", _source);
        exit(EXIT_FAILURE);
    }

    while (fgets(lineBuffer, MAX_LINE_LEN, source)) {
        preprocessString(lineBuffer);

        if (lineBuffer[0] == '\0') {
            continue;
        }

        fprintf(dest, "%s\n", lineBuffer);
    }

    fclose(source);
    fclose(dest);
}
