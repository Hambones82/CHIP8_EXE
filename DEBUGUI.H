#include "c8state.h"
#include "scncodes.h"

#ifndef DEBUGUI_H
#define DEBUGUI_H

signed char debugUIProcessInput(signed char scancode);
void drawDebugUI(struct c8State *state);

#endif
