//#define DEBUG_MEM
//#define DEBUG_OPCODE

#define DEBUG

#include <conio.h>
#include <stdio.h>
#include "c8state.h"
#include "c8dsm.h"
#include "c8core.h"
#include "draw.h"
#include "debug.h"
#include "timer.h"
#include "keyboard.h"
#include "sound.h"
#include "c8ctrls.h"

int tempvar = 0;

int main(int argc, char** argv) {
	int i = 0;
	struct c8State *coreState;
	char disasm[MAX_DSM_MSG];
	int op = 0;
	int fileSize = 0;
	int stall = 0;
	int stepPaused = 0;
	int stepNext = 0;

	initDebug();


	if(argc == 1) {
		printf("Must specify rom file.\n");
		exit(1);
	}
	else {
		coreState = initC8State();
		fileSize = loadROMFile(coreState, argv[1]);
	}
	if(fileSize == -1) {
		printf("failed to load file %s\n", argv[1]);
		exit(1);
	}
	initControls();
	initDrawing();
	startTimer();
	startKeyboard();
	initSound();
	while(!getScanCodeDown(1)) {

#ifdef DEBUG_OPCODE
			dLog("PC: %04X ", coreState->PC);
			dLog("INS: %02X %02X \n",
				coreState->mem[coreState->PC],
				coreState->mem[coreState->PC+1]);
			{int i = 0;
			for(i; i < 16; i++) {
				dLog("v%d: %d", i, coreState->v[i]);
			}}
			dLog("I: %d\n", coreState->I);
#endif

			c8CtrlExecute(coreState);
/*
			if(getDrawHalt()) {
				continue;
			}//else if
			else if(stepPaused) {
				if(!stepNext) {
					continue;
				}
			}
			else if(stall == 0) {
				stall = c8Execute(coreState);

				if(stall == -1) {//this is all f-ed up bc doesn't correspond
								 //to what is needed
					startKeyPressTracking();
				}
			}
			else {
				keyPressedVal = keyPressed();
				if(keyPressedVal>=0) {
					int VxRegNo = coreState->keyPressedReg;
					if((VxRegNo < 0) || (VxRegNo > 15)) {
						printf("reg no oob\n");
						exit(1);
					}

					stall = 0; //we are arriving here but shouldn't be.
					dLog("key pressed is %d\n", keyPressedVal);
					coreState->v[coreState->keyPressedReg] = keyPressedVal;
				}
			}

			//maybe we want a c8controls struct...
			//then we can just execute based on the controls...
			//so we pass it to the ui and also to c8core

			clearRecentKeyAvailable();
*/
#ifdef DEBUG_MEM
			dLog("\n\tMEM: ");
			{int i = coreState->I;
			for(i; i < coreState->I+20; i++){
				dLog("%02X ", coreState->mem[i]);
			}}
			dLog("\n");
#endif

		}
	endDebug();
	deInitDrawing();
	endTimer();
	endKeyboard();
	return 0;
}
