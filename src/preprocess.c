#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "preprocess.h"

// remove comments and whitespace from a line of assmebly
void preprocessString(char* str) {
    // Edge case: Null or empty string
    if (str == NULL || *str == '\0') {
        return;
    }

    // Remove comments (everything after '#')
    char* commentAddress = strchr(str, '#');
    if (commentAddress != NULL) {
        *commentAddress = '\0';
    }

    // Trim trailing whitespace
    int lastCharIndex = strlen(str) - 1;
    while (lastCharIndex >= 0 && isspace((unsigned char)str[lastCharIndex])) {
        lastCharIndex--;
    }
    str[lastCharIndex + 1] = '\0'; // Null-terminate after the last non-whitespace character

    // Trim leading whitespace
    int i = 0;
    while (str[i] != '\0' && isspace((unsigned char)str[i])) {
        i++;
    }

    // If the string is empty after trimming, set it to an empty string
    if (str[i] == '\0') {
        str[0] = '\0';
        return;
    }

    // Shift characters to remove leading whitespace
    int j = 0;
    while (str[i] != '\0') {
        str[j++] = str[i++];
    }
    str[j] = '\0'; // Null-terminate the string
}

void preprocessFile(char* _source, char* _dest) {
    FILE* source = fopen(_source, "r");
    FILE* dest = fopen(_dest, "w");
    char lineBuffer[MAX_LINE_LEN];

    if (source == NULL) {
        fprintf(stderr, "Error: Could not open \"%s\" for reading.\n", _source);
        exit(EXIT_FAILURE);
    }

    if (dest == NULL) {
        fprintf(stderr, "Error: Could not open \"%s\" for writing.\n", _dest);
        fclose(source);
        exit(EXIT_FAILURE);
    }

    while (fgets(lineBuffer, MAX_LINE_LEN, source)) {
        preprocessString(lineBuffer);

        // Skip empty lines
        if (lineBuffer[0] == '\0') {
            continue;
        }

        fprintf(dest, "%s\n", lineBuffer);
    }

    fclose(source);
    fclose(dest);
}
