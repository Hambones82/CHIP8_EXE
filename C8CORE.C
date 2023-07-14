/*#define DEBUG*/
/*#define DEBUG_KEYPRESSED*/
#include "debug.h"
#include "c8state.h"
#include "draw.h"
#include "c8core.h"
#include "keyboard.h"
#include "sound.h"
#include "c8ctrls.h"
#include "timer.h"

unsigned char spriteMap[16] = {
0,  5,  10, 15, 20, 25, 30, 35,
40, 45, 50, 55, 60, 65, 70, 75
};

/*maps from hex values 0-F to IBM PC scancodes*/
unsigned char keyMap[16] = {
0x2d, 2, 3, 4, 0x10, 0x11, 0x12, 0x1e, 0x1f, 0x20, 0x2c,
0x2e, 5, 0x13, 0x21, 0x2f
};

int c8KeytoScanCode(unsigned char c8Key) {
	if((c8Key < 0) || (c8Key > 15)) return 0;
	return keyMap[c8Key];
}

/*return value is whether to stall for keyboard*/
int c8Execute(struct c8State *state) {
	int retVal = C8_NO_HALT;
	unsigned int op = (((int)(state->mem[state->PC])) << 8) +
		(int)(state->mem[state->PC+1]);
	int three_nibble = op & 0x0FFF;
	int byte = op & 0x00FF;
	int nibble = op & 0xF;
	int VxReg = ((op & 0x0F00) >> 8) & 0x000F;
	int VyReg = ((op & 0x00F0) >> 4) & 0x000F;

#ifdef DEBUG
	dLog("DEC: %04X ", op);
	dLog("3N: %04X ", three_nibble);
	dLog("BYT: %02X ", byte);
	dLog("NIB: %X ", nibble);
	dLog("Vx: %X ", VxReg);
	dLog("Vy: %X ", VyReg);
#endif

	state->PC+=2;
	switch((op & 0xF000) >> 12) {
		case 0:
			switch(op) {
				case 0xE0:
				clearDisplay();
				break;
				case 0xEE:
				state->PC = state->stack[state->SP];
				state->SP--;
				break;
			}
			break;
		case 1:
			state->PC = three_nibble;
			break;
		case 2:
			state->SP++;
			state->stack[state->SP] = state->PC;
			state->PC = three_nibble;
			break;
		case 3:
			if(state->v[VxReg] == byte) state->PC+=2;
			break;
		case 4:
			if(state->v[VxReg] != byte) state->PC+=2;
			break;
		case 5:
			if(state->v[VxReg] == state->v[VyReg])
				state->PC+=2;
			break;
		case 6:
			state->v[VxReg] = byte;
			break;
		case 7:
			state->v[VxReg] += byte;
			break;
		case 8:
			switch(op & 0x000F) {
			 case 0:
				 state->v[VxReg] = state->v[VyReg];
				 break;
			 case 1:
				 state->v[VxReg] |= state->v[VyReg];
				 state->v[0xF] = 0;
				 break;
			 case 2:
				 state->v[VxReg] &= state->v[VyReg];
				 state->v[0xF] = 0;
				 break;
			 case 3:
				 state->v[VxReg] ^= state->v[VyReg];
				 state->v[0xF] = 0;
				 break;
			 case 4:
				{
				 int working = 0;
				 working = state->v[VxReg] + state->v[VyReg];
				 state->v[VxReg] = working & 0x00FF;
				 state->v[0xF] = working > 255 ? 1 : 0;
				}
				break;
			 case 5:
				{
				int working = (state->v[VxReg] > state->v[VyReg]) ? 1 : 0;
				 state->v[VxReg]-=state->v[VyReg];
				 state->v[0xF] = working;
				 break;
				}
			 case 6:
				{
				 unsigned char inputReg = VyReg;
				 int working = state->v[inputReg] & 1;
				 state->v[VxReg] = state->v[inputReg] >> 1;
				 state->v[0xF] = working;
				 break;
				}
			 case 7:
				{
				 int working = (state->v[VxReg] < state->v[VyReg]) ? 1 : 0;
				 state->v[VxReg]=state->v[VyReg] - state->v[VxReg];
				 state->v[0xF] = working;
				 break;
				}
			 case 0xE:
				{
				 unsigned char inputReg = VyReg;
				 int working = (state->v[inputReg] & 0x80) >> 7;
				 state->v[VxReg] = state->v[inputReg] << 1;
				 state->v[0xF] = working;
				 break;
				}
			 default:
				 /*todo: default case for opcode 8*/
				 break;
		}
		break;
		case 9:
			if(state->v[VxReg] != state->v[VyReg])
				state->PC+=2;
			break;
		case 0xA:
			state->I = three_nibble;
			break;
		case 0xB:
			state->PC = state->v[0] + three_nibble;
			break;
		case 0xC:
			/*todo: fix random number generation*/
			state->v[VxReg] = state->PC ^
				state->v[0] ^ state->v[1] ^ state->SP;
			break;
		case 0xD:
			state->v[0xF] = drawCH8(&(state->mem[0]),
				state->v[VxReg],
				state->v[VyReg],
				nibble,
				state->I);
				retVal = C8_DRAW_HALT;
			break;
		case 0xE:
			{
				int keyDown = getScanCodeDown(
					c8KeytoScanCode(state->v[VxReg]));
				switch(op & 0x00FF) {
					case 0x009E:
					if(keyDown) {
						state->PC+=2;
					}
					break;
					case 0x00A1:
					if(!keyDown) {
						state->PC+=2;
					}
					break;
					default:
					break;
				}
			}
			break;
		  case 0xF:
			switch((op & 0x00FF)) {
				case 0x07:
				/*todo: implement delay timer*/
				state->v[VxReg] = getTimer();
				break;
				case 0x0A:
				state->keyPressedReg = VxReg;
				retVal = C8_FX0A_HALT;
				dLog("encountered wait for input instruction\n");
				dLog("PC is %X, instruction is %X", state->PC,
					state->mem[state->PC]);
				break;
				case 0x15:
				/*todo: implement delay timer*/
				setTimer(state->v[VxReg]);
				break;
				case 0x18:
				/*todo: implement sound timer*/
				{
					unsigned char Vx = state->v[VxReg];
					if(VxReg != 0) {
						setSoundTimer(state->v[VxReg]);
						startBeep();
					}
				}
				break;
				case 0x1E:
				state->I+=state->v[VxReg];
				break;
				case 0x29:
				/*todo: implement sprite location for character*/
				state->I = spriteMap[state->v[VxReg]];
				break;
				case 0x33:
					{
						unsigned char value = state->v[VxReg];
						unsigned int I = state->I;
#ifdef DEBUG
						dLog("\n\tBCD input: %d", value);
#endif
						state->mem[I] = 0;
						state->mem[I+1] = 0;
						state->mem[I+2] = 0;
						while(value >= 100) {
							value -= 100;
							state->mem[I]++;
						}
						while(value >= 10) {
							value -= 10;
							state->mem[I+1]++;
						}
						state->mem[I+2] = value;
#ifdef DEBUG
						dLog("stored as: %d%d%d\n",
							state->mem[I],
							state->mem[I+1],
							state->mem[I+2]);
#endif
					}
				break;
				case 0x55:
				{
					int i = 0;
					for(i = 0; i <= VxReg; i++) {
						state->mem[state->I+i] = state->v[i];
					}
					state->I += VxReg + 1;
				}
				break;
				case 0x65:
				{
					int i = 0;
					for(i = 0; i <= VxReg; i++) {
						state->v[i] = state->mem[state->I+i];
					}
					state->I += VxReg + 1;
				}
				break;
				default:
				/*todo: implement 0xF default*/
				break;
			}
			break;
		default:
			/*todo: implement overall default*/
			break;
	}
	return retVal;
}
