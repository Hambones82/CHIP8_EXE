#include <stdio.h>

#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_FILENAME "debug.txt"

int initDebug(void);
void endDebug(void);
void dLog(char *msg, ...);

#endif

