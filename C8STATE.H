#include "c8hist.h"

#ifndef C8STATE_H
#define C8STATE_H

#define HISTORY_STAGING_BUFFER_SIZE     256
struct c8State {
	/*regs*/
	unsigned char v[16];
	/*unsigned char VF;*/
	unsigned int PC;
	unsigned int I;
	unsigned char SP;
	/*timing, sound*/
	unsigned char keyPressedReg;

	unsigned int stack[16];
	unsigned char mem[4096];

	struct stateHistory *history;
	struct historyTraverser *traverser;
	unsigned char historyStagingBuffer[HISTORY_STAGING_BUFFER_SIZE];
	unsigned char HSBSize;
};

int getOp(struct c8State *state);
int getOpAt(struct c8State *state, int address);
int loadROMFile(struct c8State *state, char *file);
struct c8State *initC8State(void);
void testInit(struct c8State *state);

#endif
