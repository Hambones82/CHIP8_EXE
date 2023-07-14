#include "draw.h"
#include "c8state.h"
#include "c8ctrls.h"
#include "scncodes.h"
#include "debug.h"

int areaID = 0;

/*returns 1 for consumes input
//returns 0 for doesn't consume input
*/
signed char debugUIProcessInput(signed char scancode) {
	dLog("debug ui processing input\n");
	if(scancode == SCNCD_DASH) {
		struct screenPos drawPos;
		GetAreaOffset((areaID+3) & 0x03, &drawPos);
		if(drawPos.x != -1 && drawPos.y != -1) {
			printfAt(drawPos.x, drawPos.y, "    ");
		}
		GetAreaOffset((areaID++) & 0x03, &drawPos);
		if(drawPos.x != -1 && drawPos.y != -1) {
			printfAt(drawPos.x, drawPos.y, "test");
		}
		return 1;
	}
	else {
		return 0;
	}
}

void drawDebugUI(struct c8State *state) {
	struct drawPos;

}

