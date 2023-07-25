#ifndef C8STATE_H
#define C8STATE_H

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
};

int getOp(struct c8State *state);
int getOpAt(struct c8State *state, int address);
int loadROMFile(struct c8State *state, char *file);
struct c8State *initC8State(void);
void testInit(struct c8State *state);

#endif
