/*#define DEBUG*/
/*#define DEBUG_KEYPRESSED*/
#include "c8hist.h"
#include "debug.h"
#include "c8state.h"
#include "draw.h"
#include "c8core.h"
#include "keyboard.h"
#include "sound.h"
#include "c8ctrls.h"
#include "timer.h"
#include "c8hist.h"

unsigned char spriteMap[16] = {
0,  5,  10, 15, 20, 25, 30, 35,
40, 45, 50, 55, 60, 65, 70, 75
};

/*maps from hex values 0-F to IBM PC scancodes*/
unsigned char keyMap[16] = {
0x2d, 2, 3, 4, 0x10, 0x11, 0x12, 0x1e, 0x1f, 0x20, 0x2c,
0x2e, 5, 0x13, 0x21, 0x2f
};

#define MEMSIZE         0x1000
#define MEM_WRAP        0x0FFF

void saveVxHistory(struct c8State *state,
							unsigned char oldValue,
							unsigned char newValue) {
	struct setVxHistory *history =
		(struct setVxHistory *)(state->historyStagingBuffer);
	history->VxDiff = newValue - oldValue;
	state->HSBSize = sizeof(struct setVxHistory);
}

void saveVxVFHistory(struct c8State *state,
							unsigned char oldVX,
							unsigned char newVX,
							unsigned char oldVF) {
	struct setVxVFHistory *history =
		(struct setVxVFHistory *)(state->historyStagingBuffer);
	history->VxDiff= newVX - oldVX;
	history->VFDiff = oldVF;
	state->HSBSize = sizeof(struct setVxVFHistory);
}


void setPC(struct c8State *state, int PC) {
	state->PC = PC & MEM_WRAP;
}

int getPC(struct c8State *state) {
	return state->PC;
}

void setMem(struct c8State *state,
	unsigned int address,
	unsigned char value) {
	state->mem[address & MEM_WRAP] = value;
}

unsigned char getMem(struct c8State *state, unsigned int address) {
	return state->mem[address & MEM_WRAP];
}

unsigned char setReg(struct c8State *state,
	unsigned char regNo,
	unsigned char value) {
	saveVxHistory(state, state->v[regNo & 0x0F], value);
	state->v[regNo & 0x0F] = value;
}

void setRegAndVF(struct c8State *state,
	unsigned char regNo,
	unsigned char value,
	unsigned char VFvalue) {
	saveVxVFHistory(state, state->v[regNo & 0x0F], value, state->v[0xF]);
	state->v[regNo & 0x0F] = value;
	state->v[0xF] = VFvalue;
}

unsigned char getReg(struct c8State *state, unsigned char regNo) {
	return state->v[regNo & 0x0F];
}

void saveIHistory(struct c8State *state, unsigned int value) {
	struct setIHistory *history = (struct setIHistory *)
		(state->historyStagingBuffer);
	history->iDiff = value - state->I;
	state->HSBSize = sizeof(struct setIHistory);
}

void setI(struct c8State *state, unsigned int value) {
	state->I = value;
	saveIHistory(state, value);
}

unsigned int getI(struct c8State *state) {
	return state->I;
}

void pushPC(struct c8State *state) {
	state->stack[state->SP] = state->PC;
	state->SP++;
}

void popPC(struct c8State *state) {
	state->SP--;
	state->PC = state->stack[state->SP];
}

void advancePC(struct c8State *state) {
	state->PC += 2;
}

int c8KeytoScanCode(unsigned char c8Key) {
	if((c8Key < 0) || (c8Key > 15)) return 0;
	return keyMap[c8Key];
}

//this is a pain in the ass -- should add the things to the setters rather
//than retyping it over and over...
/*return value is whether to stall for keyboard*/
int c8Execute(struct c8State *state) {
	int retVal = C8_NO_HALT;
	unsigned int op = getOp(state);
	int three_nibble = op & 0x0FFF;
	int byte = op & 0x00FF;
	int nibble = op & 0xF;
	int VxReg = ((op & 0x0F00) >> 8) & 0x000F;
	int VyReg = ((op & 0x00F0) >> 4) & 0x000F;
	struct opcodeRecord opRecord;
	//unsigned char historySize = 0;
	state->HSBSize = 0;
	opRecord.PC = state->PC;
	opRecord.opcode = op;

	advancePC(state);
	switch((op & 0xF000) >> 12) {
		case 0:
			switch(op) {
				case 0xE0:
					state->HSBSize = 256;
					serializeFB(state->historyStagingBuffer);
					clearDisplay();
					break;
				case 0xEE:
					popPC(state);
					break;
			}
			break;
		case 1:
			setPC(state, three_nibble);
			break;
		case 2:
		{
			struct pushPCHistory *history =
				(struct pushPCHistory *)(state->historyStagingBuffer);
			state->HSBSize = sizeof(struct pushPCHistory);
			history->stackDiff = state->PC - state->stack[state->SP];
			pushPC(state);
			setPC(state, three_nibble);
			break;
		}
		case 3:
			if(state->v[VxReg] == byte) advancePC(state);
			break;
		case 4:
			if(state->v[VxReg] != byte) advancePC(state);
			break;
		case 5:
			if(state->v[VxReg] == state->v[VyReg]) advancePC(state);
			break;
		case 6:
			setReg(state, VxReg, byte); //maybe nmake setreg do its own
			break;
		case 7:
			setReg(state, VxReg, state->v[VxReg] + byte);
			break;
		case 8:
			switch(op & 0x000F) {
			 case 0:
				 setReg(state, VxReg, state->v[VyReg]);
				 break;
			 case 1:
			 {
				 unsigned char newVx = getReg(state, VyReg) | state->v[VxReg];
				 setRegAndVF(state, VxReg, newVx, 0);
				 break;
			 }
			 case 2:
				 setRegAndVF
					(state, VxReg, state->v[VxReg] & state->v[VyReg], 0);
				 break;
			 case 3:
				 setRegAndVF
					(state, VxReg, state->v[VxReg] ^ state->v[VyReg], 0);
				 setReg(state, 0xF, 0);
				 break;
			 case 4:
				{
				 int working = 0;
				 working = state->v[VxReg] + state->v[VyReg];
				 setRegAndVF(state, VxReg, working & 0x00FF,
					working > 255 ? 1 : 0);
				}
				break;
			 case 5:
				{
				 int working = (state->v[VxReg] > state->v[VyReg]) ? 1 : 0;
				 setRegAndVF
					(state, VxReg, state->v[VxReg] - state->v[VyReg], working);
				 break;
				}
			 case 6:
				{
				 int working = state->v[VyReg] & 1;
				 setRegAndVF
					(state, VxReg, state->v[VyReg] >> 1, working);
				 break;
				}
			 case 7:
				{
				 int working = (state->v[VxReg] < state->v[VyReg]) ? 1 : 0;
				 setRegAndVF(state, VxReg,
					state->v[VyReg] - state->v[VxReg], working);
				 setReg(state, 0xF, working);
				 break;
				}
			 case 0xE:
				{
				 int working = (state->v[VyReg] & 0x80) >> 7;
				 setRegAndVF(state, VxReg, state->v[VyReg] << 1, working);
				 break;
				}
			 default:
				printf("unhandled opcode 8 sub-op\n");
				exit(1);
				break;
		}
		break;
		case 9:
			if(state->v[VxReg] != state->v[VyReg])
				advancePC(state);
			break;
		case 0xA:
			setI(state, three_nibble);
			break;
		case 0xB:
			setPC(state, state->v[0] + three_nibble);
			break;
		case 0xC:
			/*todo: fix random number generation*/
			setReg(state, VxReg,
				state->PC ^ state->v[0] ^ state->v[1] ^ state->SP);
			break;
		case 0xD:
			{
			unsigned char hit =
				drawCH8(&(state->mem[0]),
					state->v[VxReg],
					state->v[VyReg],
					nibble,
					state->I);
					retVal = C8_DRAW_HALT;
			setReg(state, 0xF, hit);
			}
			break;
		case 0xE:
			{
				int keyDown = getScanCodeDown(
					c8KeytoScanCode(state->v[VxReg]));
				switch(op & 0x00FF) {
					case 0x009E:
					if(keyDown) {
						advancePC(state);
					}
					break;
					case 0x00A1:
					if(!keyDown) {
						advancePC(state);
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
					setReg(state, VxReg, getTimer());
					break;
				case 0x0A:
					setReg(state, state->keyPressedReg, VxReg);
					retVal = C8_FX0A_HALT;
					break;
				case 0x15:
					setTimer(state->v[VxReg]);
				break;
				case 0x18:
				{
					unsigned char Vx = state->v[VxReg];
					if(VxReg != 0) {
						setSoundTimer(state->v[VxReg]);
						startBeep();
					}
				}
				break;
				case 0x1E:
					setI(state, state->I + state->v[VxReg]);
				break;
				case 0x29:
					setI(state, spriteMap[state->v[VxReg]]);
				break;
				case 0x33:
					{
						unsigned char value = state->v[VxReg];
						unsigned int I = state->I;
						unsigned char hundreds = 0, tens = 0, ones = 0;
						while(value >= 100) {
							value -= 100;
							hundreds++;
						}
						while(value >= 10) {
							value -= 10;
							tens++;
						}
						setMem(state, I, hundreds);
						setMem(state, I+1, tens);
						setMem(state, I+2, value);
						{
							struct BCDHistory * history =
								(struct BCDHistory *)
								(state->historyStagingBuffer);
							state->HSBSize = sizeof(struct BCDHistory);
							history->digitDiffs[0] = hundreds -
								getMem(state, I);
							history->digitDiffs[1] = tens -
								getMem(state, I+1);
							history->digitDiffs[2] = ones -
								getMem(state, I+2);
						}
					}
				break;
				case 0x55:
				{
					int i = 0;
					struct wideUpdateHistory *history =
						(struct wideUpdateHistory *)
						(state->historyStagingBuffer);
					state->HSBSize = sizeof(struct wideUpdateHistory);
					for(i = 0; i <= VxReg; i++) {
						unsigned char updateVal = state->v[i];
						history->bytes[i] = updateVal -
							state->mem[state->I+i];
						state->mem[state->I+i] = updateVal;
						//setMem(state, state->I+i, state->v[i]);
					}
					history->Idiff = state->I + state->v[VxReg] + 1
						- state->I;
					state->I += state->v[VxReg] + 1;
					//setI(state, state->I + VxReg + 1);
				}
				break;
				case 0x65:
				{
					int i = 0;
					struct wideUpdateHistory *history =
						(struct wideUpdateHistory *)
						(state->historyStagingBuffer);
					state->HSBSize = sizeof(struct wideUpdateHistory);
					for(i = 0; i <= VxReg; i++) {
						unsigned char updateVal = state->mem[state->I+i];
						history->bytes[i] = updateVal - state->v[i];
						//setReg(state, i, state->mem[state->I+i]);
						state->v[i] = state->mem[state->I+i];
					}
					history->Idiff = state->I + state->v[VxReg] + 1
						- state->I;
					state->I += state->v[VxReg] + 1;
					//setI(state, state->I + VxReg + 1);
				}
				break;
				default:
					printf("unhandled 0xF instruction\n");
					exit(1);
				break;
			}
			break;
		default:
			printf("unhandled instruction\n");
			exit(1);
			break;
	}
	//save the record
	advanceTraverserWithAllocate(state->traverser, state->HSBSize);
	recordInstruction(state->traverser, state->historyStagingBuffer,
		&opRecord);

	return retVal;
}
