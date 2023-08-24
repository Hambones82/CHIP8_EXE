#include <stdio.h>
#include "c8dsm.h"

void disasmOut(int op, char *disassembly) {
	int charsWritten = 0;
	switch((op & 0xF000) >> 12) {
		case 0:
		switch(op) {
			case 0xE0:
			strcpy(disassembly, "CLS");
			break;
			case 0xEE:
			strcpy(disassembly, "RET");
			break;
		}
		break;
		case 1:
		charsWritten = sprintf(disassembly, "JP %X", op & 0x0FFF);
		break;
		case 2:
		charsWritten = sprintf(disassembly, "CALL %X", op & 0x0FFF);
		break;
		case 3:
		charsWritten = sprintf(disassembly, "SE V%X, %X",
			(op & 0x0F00) >> 8, op & 0x00FF);
		break;
		case 4:
		charsWritten = sprintf(disassembly, "SNE V%X, %X",
			(op & 0x0F00) >> 8, op & 0x00FF);
		break;
		case 5:
		charsWritten = sprintf(disassembly, "SE V%X, V%X",
			(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
		break;
		case 6:
		charsWritten = sprintf(disassembly, "LD V%X, %X",
			(op & 0x0F00) >> 8, op & 0x00FF);
		break;
		case 7:
		charsWritten = sprintf(disassembly, "ADD V%X, %X",
			(op & 0x0F00) >> 8, op & 0x00FF);
		break;
		case 8:
		switch(op & 0x000F) {
			 case 0:
			 charsWritten = sprintf(disassembly, "LD V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 1:
			 charsWritten = sprintf(disassembly, "OR V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 2:
			 charsWritten = sprintf(disassembly, "AND V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 3:
			 charsWritten = sprintf(disassembly, "XOR V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 4:
			 charsWritten = sprintf(disassembly, "ADD V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 5:
			 charsWritten = sprintf(disassembly, "SUB V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 6:
			 charsWritten = sprintf(disassembly, "SHR V%X {, V%X}",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 7:
			 charsWritten = sprintf(disassembly, "SUBN V%X, V%X",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 case 0xE:
			 charsWritten = sprintf(disassembly, "SHL V%X {, V%X}",
				(op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
			 break;
			 default:
			 charsWritten = sprintf(disassembly, "%X", op);
			 break;
		}
		break;
		case 9:
		charsWritten = sprintf(disassembly, "SNE V%X, V%X",
			(op & 0x0F00) >> 8,
			(op & 0x00F0) >> 4);
		break;
		case 0xA:
		charsWritten = sprintf(disassembly, "LD I, %X", op & 0x0FFF);
		break;
		case 0xB:
		charsWritten = sprintf(disassembly, "JP V0, %X", op & 0x0FFF);
		break;
		case 0xC:
		charsWritten = sprintf(disassembly, "RND V%X, %d",
			(op & 0x0F00) >> 8, op & 0x00FF);
		break;
		case 0xD:
		charsWritten = sprintf(disassembly, "DRW V%X, v%X, %X",
			(op & 0x0F00) >> 8, (op & 0x00F0) >> 4, op & 0x000F);
		break;
		case 0xE:
		switch(op & 0x00FF) {
			case 0x009E:
			charsWritten = sprintf(disassembly, "SKP V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x00A1:
			charsWritten = sprintf(disassembly, "SKNP V%X",
				(op & 0x0F00) >> 8);
			break;
			default:
			charsWritten = sprintf(disassembly, "%X", op);
			break;
		}
		break;
		case 0xF:
		switch((op & 0x00FF)) {
			case 0x07:
			charsWritten = sprintf(disassembly, "LD V%X, DT",
				(op & 0x0F00) >> 8);
			break;
			case 0x0A:
			charsWritten = sprintf(disassembly, "LD V%X, K",
				(op & 0x0F00) >> 8);
			break;
			case 0x15:
			charsWritten = sprintf(disassembly, "LD DT, V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x18:
			charsWritten = sprintf(disassembly, "LD ST, V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x1E:
			charsWritten = sprintf(disassembly, "ADD I, V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x29:
			charsWritten = sprintf(disassembly, "LD F, V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x33:
			charsWritten = sprintf(disassembly, "LD B, V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x55:
			charsWritten = sprintf(disassembly, "LD [I], V%X",
				(op & 0x0F00) >> 8);
			break;
			case 0x65:
			charsWritten = sprintf(disassembly, "LD V%X, [I]",
				(op & 0x0F00) >> 8);
			break;
			default:
			charsWritten = sprintf(disassembly, "%04X", op);
		}
		break;
		default:
		charsWritten = sprintf(disassembly, "%X", op);
		break;
	}
	if(charsWritten > MAX_DSM_MSG - 1) {
		printf("buffer overflow in disassembly function\n");
		exit(1);
	}
}
