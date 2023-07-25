#include <stdlib.h>
#include <stdio.h>
#include "c8ctrls.h"
#include "timer.h"
#include "c8core.h"
#include "c8state.h"
#include "keyboard.h"

/*maps from hex values 0-F to IBM PC scancodes*/
extern unsigned char keyMap[16];
/*
unsigned char keyMap[16] = {
0x2d, 2, 3, 4, 0x10, 0x11, 0x12, 0x1e, 0x1f, 0x20, 0x2c,
0x2e, 5, 0x13, 0x21, 0x2f
};*/

/*0 = already pressed when started
//1 = not pressed
//2 = released and pressed
//3 = released, pressed, released
*/
unsigned char keysPressed[16] = {
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0
};

void startKeyPressTracking(void) {
	int i = 0;
	for(i; i < 16; i++) {
		keysPressed[i] = !getScanCodeDown(keyMap[i]);
	}
}

signed char keyPressed(void) {
	signed char retVal = -1;
	int i = 0;
	for(i; i < 16; i++) {
		/*update transition to first state -- up to down*/
		if((keysPressed[i] == 0) && !getScanCodeDown(keyMap[i])) {
			keysPressed[i] = 1;
		}
		if((keysPressed[i] == 1) && getScanCodeDown(keyMap[i])) {
			keysPressed[i] = 2;
		}
		if((keysPressed[i] == 2) && !getScanCodeDown(keyMap[i])) {
			keysPressed[i] = 3;
		}
		if(keysPressed[i] == 3) {
			retVal = i;
			break;
		}
	}
	return retVal;
}

struct c8Controls controls;

int isPaused() {
	return controls.step_halted;
}

void setDrawHalt(void) {
	controls.draw_halted = C8SETTING_ON;
}

#pragma check_stack(off)
void resetDrawHalt(void) {
	controls.draw_halted = C8SETTING_OFF;
}

void initControls(void) {
	controls.draw_halted = C8SETTING_OFF;
	controls.step_halted = C8SETTING_OFF;
	controls.step_once = C8SETTING_OFF;
	controls.FX0A_halted = C8SETTING_OFF;
	RegisterTimerCallback(resetDrawHalt);
}

int getSetting(int settingType) {
	switch(settingType) {
		case C8_STEP_HALT:
			return controls.step_halted;
			break;
		case C8_DRAW_HALT:
			return controls.draw_halted;
			break;
		case C8_FX0A_HALT:
			return controls.FX0A_halted;
			break;
		default:
			printf("requesting unsupported setting in c8ctrls\n");
			exit(1);
	}
}

void updateCtrls(struct c8Setting *setting) {
	switch(setting->settingType) {
		case C8_STEP_HALT:
			controls.step_halted = setting->onOff;
			break;
		case C8_DRAW_HALT:
			controls.draw_halted = setting->onOff;
			break;
		case C8_FX0A_HALT:
			controls.FX0A_halted = setting->onOff;
			break;
		case C8_STEP_ONCE:
			controls.step_once = setting->onOff;
			break;
		default:
			printf("updating unsupported setting in c8ctrls\n");
			exit(1);
	}
}

void c8CtrlExecute(struct c8State *state) {
	int stall_type = C8_NO_HALT;
	signed char keyPressedVal = -1;

	if(controls.draw_halted == C8SETTING_ON) { /*draw halt*/
		return;
	}
	else if(controls.step_halted == C8SETTING_ON &&
			controls.step_once == C8SETTING_OFF) { /*step paused*/
		return;/*fix...??  or maybe just with P, call execute?*/
		/*maybe we just do executeOnce or something...*/
	}
	else if(controls.FX0A_halted == C8SETTING_OFF) { /*dealing w fx0a stall*/
		stall_type = c8Execute(state);
		controls.step_once = C8SETTING_OFF;

		if(stall_type == C8_FX0A_HALT) {
			controls.FX0A_halted = C8SETTING_ON;
			startKeyPressTracking();
		}
		else if(stall_type == C8_DRAW_HALT) {
			controls.draw_halted = C8SETTING_ON;
		}
	}
	else { /*this is the fx0a stall*/
		keyPressedVal = keyPressed();
		if(keyPressedVal>=0) {
			int VxRegNo = state->keyPressedReg;
			if((VxRegNo < 0) || (VxRegNo > 15)) {
				printf("reg no oob\n");
				exit(1);
			}
			controls.FX0A_halted = C8SETTING_OFF;
			state->v[state->keyPressedReg] = keyPressedVal;
		}
	}

}

