//todo: fix up a bunch of things -- e.g.:
//  bold section headings
//  alignment of bottom row things
//  most recent instruction -- more complex than i thought - come back

#define LOOKAHEAD 5

#include "c8dsm.h"
#include "draw.h"
#include "c8state.h"
#include "c8ctrls.h"
#include "scncodes.h"
#include "debug.h"
#include "debugui.h"

#define SPEED_PAUSED    0
#define SPEED_UNPAUSED  1

#define PURPOSE_MEMORY 0

void clearEnteringNumber(void);
void drawTextField(void);
void clearTextField(void);
void drawHotkeys(void);

#define CAPTURE_DECIMAL     10
#define CAPTURE_HEX         16

struct textFieldInfo {
	int mode;
	char label[20];
	char postfix[5];
	struct screenPos pos;
	int inputSize;
};

struct numberCapture {
	int enteringNumber;//0 = not, 1 = yes
	int enteringPurpose;//defined with defines
	int currentEnteringVal;//value currently being entered
	int numDigitsEntered;
	int capture_base;
};

struct UIState {
	struct screenPos top_write_pos;
	struct screenPos left_write_pos;
	struct screenPos right_write_pos;
	struct screenPos bottom_write_pos;
	struct screenPos speedUIPosition;
	struct screenPos regsPosition;
	struct screenPos memPosition;
	struct screenPos iPosition;
	struct screenPos stackPosition;
	struct screenPos lastIPosition;
	int memViewStart;
	int stackStart;
	struct numberCapture nCap;
	int isDirty; //0 if not dirty, 1 if dirty
	int showDisasm; //0 if no, 1 if yes
	struct textFieldInfo tfInfo;
	//other things like current row, state of the elements... could even
	//break this odwn further and give structs for each element...
	//perhaps only for those that actually have state
};

struct UIState uiState;

int debug_display_flags = SHOW_SPEED;

/*returns 1 for consumes input
//returns 0 for doesn't consume input
*/
void resetStartPositions() {
	GetAreaOffset(AREA_RIGHT, &uiState.right_write_pos);
	GetAreaOffset(AREA_LEFT, &uiState.left_write_pos);
	GetAreaOffset(AREA_TOP, &uiState.top_write_pos);
	GetAreaOffset(AREA_BOTTOM, &uiState.bottom_write_pos);
}

void initDebugUI() {
	resetStartPositions();
	uiState.regsPosition.x = uiState.bottom_write_pos.x;
	uiState.regsPosition.y = uiState.bottom_write_pos.y;
	uiState.speedUIPosition.x = uiState.right_write_pos.x;
	uiState.speedUIPosition.y = uiState.right_write_pos.y;
	uiState.memPosition.x = uiState.bottom_write_pos.x;
	uiState.memPosition.y = uiState.bottom_write_pos.y+3;
	uiState.memViewStart = 0x200;
	uiState.stackStart = 0;
	uiState.iPosition.x = uiState.right_write_pos.x;
	uiState.iPosition.y = uiState.right_write_pos.y+1;
	uiState.lastIPosition.x = uiState.right_write_pos.x;
	uiState.lastIPosition.y = uiState.right_write_pos.y+3+LOOKAHEAD;
	uiState.stackPosition.x = uiState.bottom_write_pos.x;
	uiState.stackPosition.y = uiState.bottom_write_pos.y + 2;
	clearEnteringNumber();
	uiState.isDirty = 0;
	uiState.showDisasm = 1;
	drawHotkeys();
}

#define REGS_DIRTY      1
#define MEM_DIRTY       2
#define STACK_DIRTY     4
#define INSTR_DIRTY     8
#define TEXTFIELD_DIRTY 16
#define SPEED_DIRTY     32

int UIIsDirty(void) {
	return uiState.isDirty;
}

#define BOTTOM_OFFSET     10

void updateRegs(struct c8State *state) {
	int i = 0;
	int width = 3;
	printfAt(uiState.regsPosition.x, uiState.regsPosition.y,
		"Regs: ");
	for(i; i < 16; i++) {
		printfAt(uiState.regsPosition.x + i * width + BOTTOM_OFFSET,
			uiState.regsPosition.y,
			"V%X ",
			i);
		printfAt(uiState.regsPosition.x + i * width + BOTTOM_OFFSET,
			uiState.regsPosition.y+1,
			"%02X ",
			state->v[i]);
	}
	printfAt(uiState.regsPosition.x + 16*3 + BOTTOM_OFFSET,
		uiState.regsPosition.y,
		"   I SP");
	printfAt(uiState.regsPosition.x + 16*3 + BOTTOM_OFFSET,
		uiState.regsPosition.y+1,
		"%4X%3X", state->I, state->SP);
}

void updateMem(struct c8State *state) {
	int i = 0;
	int width = 4;
	printfAt(uiState.memPosition.x, uiState.memPosition.y, "Mem: ");
	for(i; i < 16; i++) {
		printfAt(uiState.memPosition.x + i * width + BOTTOM_OFFSET,
			uiState.memPosition.y,
			"%3X ", i + uiState.memViewStart);
		printfAt(uiState.memPosition.x + i * width + BOTTOM_OFFSET,
			uiState.memPosition.y+1,
			"%02X ", state->mem[i + uiState.memViewStart]);
	}
}
#define STACK_DISPLAY_ELEMENTS 8

void updateStack(struct c8State *state) {
	int i = 0;
	int width = 5;
	printfAt(uiState.stackPosition.x, uiState.stackPosition.y, "SP%X-SP%X:",
		uiState.stackStart,
		uiState.stackStart + STACK_DISPLAY_ELEMENTS - 1);
	for(i; i < STACK_DISPLAY_ELEMENTS; i++) {
		printfAt(uiState.stackPosition.x+i*width + BOTTOM_OFFSET,
			uiState.stackPosition.y,
			"%04X ", state->stack[i + uiState.stackStart]);
	}
}


void drawSpeedUI(struct c8State *state) {
	printfAt(uiState.speedUIPosition.x, uiState.speedUIPosition.y,
		"Paused");
}

void drawInstruction(struct c8State *state) {
	int i = 0;
	int xClear = 0;
	int y = uiState.iPosition.y;
	char dsmMsg[MAX_DSM_MSG];
	printfAt(uiState.iPosition.x, y++,
		"PC: %04X", state->PC);
	printfAt(uiState.iPosition.x, y++, "---------");
	for(i; i < LOOKAHEAD; i++) {
		int row = y+i;
		for(xClear = uiState.iPosition.x; xClear < TEXT_X; xClear++) {
			drawCharAt(xClear, row, ' ');
		}
		if(uiState.showDisasm) {
			disasmOut(getOpAt(state, state->PC + i*2), dsmMsg);
		}
		else {
			sprintf(dsmMsg, "%04X", getOpAt(state, state->PC + i*2));
		}
		printfAt(uiState.iPosition.x, row, dsmMsg);
	}
	printfAt(uiState.lastIPosition.x, uiState.lastIPosition.y, "Last:");
	if(uiState.showDisasm) {
		disasmOut(getOpcodeRecord(state->traverser)->opcode, dsmMsg);
	}
	else {
		sprintf(dsmMsg, "%04X", getOpcodeRecord(state->traverser)->opcode);
	}
	for(xClear = uiState.iPosition.x; xClear < TEXT_X; xClear++) {
		drawCharAt(xClear, uiState.lastIPosition.y+1, ' ');
	}
	//printfAt(uiState.lastIPosition.x, uiState.lastIPosition.y+1, "         ");
	printfAt(uiState.lastIPosition.x, uiState.lastIPosition.y+1, dsmMsg);
}

void drawHotkeys() {
	unsigned char attr = ATTR_BLACK_BG | ATTR_RED_TEXT;
	printfAt(0, TEXT_Y, "MEM   STACK PAUSE STEP> INSTR");
	setAttribute(0, TEXT_Y-1, attr);
	setAttribute(7, TEXT_Y-1, attr);
	setAttribute(12, TEXT_Y-1, attr);
	setAttribute(22, TEXT_Y-1, attr);
	setAttribute(24, TEXT_Y-1, attr);
}

void refreshUI(struct c8State *state) {
	if(uiState.isDirty & SPEED_DIRTY) {
		drawSpeedUI(state);
	}
	if(uiState.isDirty & REGS_DIRTY) {
		updateRegs(state);
	}
	if(uiState.isDirty & MEM_DIRTY) {
		updateMem(state);
	}
	if(uiState.isDirty & INSTR_DIRTY) {
		drawInstruction(state);
	}
	if(uiState.isDirty & STACK_DIRTY) {
		updateStack(state);
	}
	if(uiState.isDirty & TEXTFIELD_DIRTY) {
		drawTextField();
	}
	uiState.isDirty = 0;
}

void clearUI(void) {
	clearOutsideDisplay();
	drawHotkeys();
}

//#define ENTERING_DIGITS_LIMIT 4

void clearEnteringNumber(void) {
	uiState.nCap.enteringNumber = 0;
	uiState.nCap.enteringPurpose = 0;
	uiState.nCap.currentEnteringVal = 0;
	uiState.nCap.numDigitsEntered = 0;
	uiState.nCap.capture_base = CAPTURE_HEX;
}

void finalizeEnteringNumber(void) {
	switch(uiState.nCap.enteringPurpose) {
		case PURPOSE_MEMORY:
			uiState.memViewStart = uiState.nCap.currentEnteringVal;
			uiState.isDirty |= MEM_DIRTY;
			break;
		default:
			break;
			//Todo: uh oh
	}
	clearEnteringNumber();
	uiState.isDirty |= TEXTFIELD_DIRTY;
	clearTextField();
}

void addEnteringNumber(signed char scancode) {
	int addVal = 0;
	if(uiState.nCap.numDigitsEntered >= uiState.tfInfo.inputSize ||
		scancode == SCNCD_ENTER) {
		finalizeEnteringNumber();
		return;
	}
	switch(scancode) {
		case SCNCD_1:
		case SCNCD_2:
		case SCNCD_3:
		case SCNCD_4:
		case SCNCD_5:
		case SCNCD_6:
		case SCNCD_7:
		case SCNCD_8:
		case SCNCD_9:
			addVal = scancode - 1;
		case SCNCD_0:
			break;
		case SCNCD_A:
			addVal = 10;
			break;
		case SCNCD_B:
			addVal = 11;
			break;
		case SCNCD_C:
			addVal = 12;
			break;
		case SCNCD_D:
			addVal = 13;
			break;
		case SCNCD_E:
			addVal = 14;
			break;
		case SCNCD_F:
			addVal = 15;
			break;
		default:
			return;
	}

	uiState.nCap.currentEnteringVal *= uiState.nCap.capture_base;
	uiState.nCap.currentEnteringVal += addVal;
	uiState.nCap.numDigitsEntered++;
}

void clearTextField(void) {
	int x = uiState.tfInfo.pos.x;
	int length = TEXT_X - x;
	for(x; x < TEXT_X; x++) {
		drawCharAt(x, uiState.tfInfo.pos.y, ' ');
	}
	drawAttributeRange(uiState.tfInfo.pos.x, length, uiState.tfInfo.pos.y,
		ATTR_BLACK_BG | ATTR_WHITE_TEXT);
	//attributes as well
}

void drawTextField(void) {
	int x = uiState.tfInfo.pos.x;
	if(uiState.nCap.enteringNumber == 1) {
		x += printfAt(x, uiState.tfInfo.pos.y+1,
			uiState.tfInfo.label);
		printfAt(x, uiState.tfInfo.pos.y+1, "%X",
			uiState.nCap.currentEnteringVal);
		drawAttributeRange(x, uiState.tfInfo.inputSize,
			uiState.tfInfo.pos.y, ATTR_LIGHT_BG | ATTR_BLACK_TEXT);
	}
}

void startTextField(struct textFieldInfo *info) {
	uiState.nCap.enteringNumber = 1;
	uiState.nCap.capture_base = info->mode;
	memcpy(&uiState.tfInfo, info, sizeof(struct textFieldInfo));
	uiState.isDirty |= TEXTFIELD_DIRTY;
}

void debugUIProcessInput(signed char scancode, struct c8State *state) {
	if(uiState.nCap.enteringNumber == 1) {
		addEnteringNumber(scancode);
		uiState.isDirty |= TEXTFIELD_DIRTY;
	}
	else {
		if(scancode == SCNCD_M) {
			struct textFieldInfo info;
			info.mode = CAPTURE_HEX;
			strcpy(info.label, "Mem: "); //need strcpy
			strcpy(info.postfix, "");
			info.inputSize = 3;
			info.pos.x = TEXT_X - 1 - 8;
			info.pos.y = TEXT_Y - 1;
			startTextField(&info);
		}
		if(scancode == SCNCD_I) {
			uiState.showDisasm = uiState.showDisasm == 0 ? 1 : 0;
			uiState.isDirty |= INSTR_DIRTY;
		}
		if(scancode == SCNCD_T) {
			uiState.stackStart ^= 8;
			uiState.isDirty |= STACK_DIRTY;
		}
		if(scancode == SCNCD_P) {
			struct c8Setting pause_setting;
			pause_setting.settingType = C8_STEP_HALT;
			pause_setting.onOff = (getSetting(C8_STEP_HALT) == C8SETTING_ON)
								? C8SETTING_OFF : C8SETTING_ON;
			updateCtrls(&pause_setting);
			if(pause_setting.onOff == C8SETTING_OFF) {
				clearUI();
			}
			else {
				uiState.isDirty |= REGS_DIRTY | MEM_DIRTY | STACK_DIRTY
								| INSTR_DIRTY | SPEED_DIRTY;
			}
		}
		if(scancode == SCNCD_PERIOD) { //this does not seem right...
									  //missing one step cycle...
			struct c8Setting step_setting;
			step_setting.settingType = C8_STEP_ONCE;
			step_setting.onOff = C8SETTING_ON;
			updateCtrls(&step_setting);
			uiState.isDirty |= REGS_DIRTY | MEM_DIRTY | STACK_DIRTY
							| INSTR_DIRTY;
		}
	}
}


