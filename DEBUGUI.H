#include "c8state.h"
#include "scncodes.h"

#ifndef DEBUGUI_H
#define DEBUGUI_H

#define SHOW_NOTHING    0
#define SHOW_SPEED      1
#define SHOW_REGS       2   //0010
#define SHOW_MEM        4   //0100
//#define SHOW_DISASM     8   //1000


void debugUIProcessInput(signed char scancode, struct c8State *state);
void initDebugUI(void);
void refreshUI(struct c8State *state);
int UIIsDirty(void);

#endif
