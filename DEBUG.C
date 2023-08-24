#include <stdarg.h>
#include <stdio.h>
#include "debug.h"

FILE *debugFile;

int initDebug(void) {
	debugFile = fopen(DEBUG_FILENAME, "w");
	if(debugFile == NULL) return -1;
	else return 0;
}

void endDebug(void) {
	fclose(debugFile);
}

void dLog(char *msg, ...) {
	va_list args;
	if(debugFile == NULL) {
		printf("debug file is null!!!@@@\n");
		exit(1);
	}
	va_start(args, msg);
	vfprintf(debugFile, msg, args);
	va_end(args);
}
