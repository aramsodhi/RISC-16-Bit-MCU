#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include <stdio.h>
#include "hashtable.h"

void secondPass(FILE* preprocessed, FILE* output, struct hashTable* symbolTable);

#endif
