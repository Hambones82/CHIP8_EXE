/*#define DEBUG_MEM*/
/*#define DEBUG_OPCODE*/

/*#define DEBUG*/

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "c8state.h"
#include "c8dsm.h"
#include "c8core.h"
#include "draw.h"
#include "debug.h"
#include "timer.h"
#include "keyboard.h"
#include "sound.h"
#include "c8ctrls.h"
#include "debugui.h"

int main(int argc, char** argv) {
	int i = 0;
	struct c8State *coreState;
	int op = 0;
	int fileSize = 0;
	int stall = 0;
	int stepPaused = 0;
	int stepNext = 0;


	if(initDebug()== -1) {
		printf("failed to open debug out file.\n");
		exit(1);
	}


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
	initDrawing();
	initDebugUI();
	startTimer();
	initControls();
	startKeyboard();
	initSound();
	while(!getScanCodeDown(1)) {
		c8CtrlExecute(coreState);
		if(isPaused() && UIIsDirty()) {
			refreshUI(coreState);
		}
		if(isRecentKeyAvailable()) {
			debugUIProcessInput(getRecentKeyPressed(), coreState);
		}
	}
	endDebug();
	deInitDrawing();
	endTimer();
	endKeyboard();
	return 0;
}
