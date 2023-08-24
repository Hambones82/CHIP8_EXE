#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "c8hist.h"

//cls is a bit hard to reverse...you need to store the entire contents of the
//screen buffer...

struct stateHistory *initStateHistory(void) {
	struct stateHistory *history = malloc(sizeof(struct stateHistory));
	history->opcodeStart = 0;
	history->opcodeLength = 0;
	history->historyStart = 0;
	history->historyLength = 0;
	return history;
};

struct historyTraverser *getHistoryTraverser(struct stateHistory *history) {
	struct historyTraverser *retVal = malloc(sizeof(struct historyTraverser));
	retVal->opcodePointer = 0;
	retVal->historyPointer = 0;
	retVal->history = history;
	return retVal;
};

void *getChanges(struct stateHistory *history,
				  struct historyTraverser *reader) {
	return &(history->instructionHistories[reader->historyPointer]);
}

void recordInstruction(struct historyTraverser *traverser,
							unsigned char *bytes,
							struct opcodeRecord *opcode) {
	struct stateHistory *history = traverser->history;
	if(opcode->dataSize > BUFFER_OVERFLOW_SIZE) {
		printf("trying to record data that is too big\n");
		exit(1);
	}
	advanceTraverserWithAllocate(traverser, opcode->dataSize);
	memcpy(&(history->instructionHistories[traverser->historyPointer]),
		   bytes, opcode->dataSize);
	memcpy(&(history->opcodes[traverser->opcodePointer]),
		opcode,
		sizeof(struct opcodeRecord));
}

void advanceTraverserWithAllocate(struct historyTraverser *traverser,
				   unsigned int size) {
	struct stateHistory *history = traverser->history;
	unsigned char currentRecordSize =
		history->opcodes[traverser->opcodePointer].dataSize;

	if(history->opcodeLength == 0) {
		history->historyLength+=size;
		history->opcodeLength++;
		return;
	}

	history->historyLength =
		((traverser->historyPointer - history->historyStart) &
		(HISTORY_BUFFER_SIZE - 1)) + currentRecordSize;

	history->opcodeLength =
		((traverser->opcodePointer - history->opcodeStart)&
		(MAX_NUMBER_OF_HISTORIES - 1)) + 1;

	history->historyLength+=size;
	if(history->historyLength > HISTORY_BUFFER_SIZE) {
		history->historyStart +=
			history->historyLength - HISTORY_BUFFER_SIZE;
		history->historyStart &= HISTORY_BUFFER_SIZE - 1;
		history->historyLength = HISTORY_BUFFER_SIZE;
	}

	history->opcodeLength++;
	if(history->opcodeLength > MAX_NUMBER_OF_HISTORIES) {
		history->opcodeLength--;
		history->opcodeStart++;
		history->opcodeStart &= (MAX_NUMBER_OF_HISTORIES - 1);
	}

	traverser->historyPointer+=currentRecordSize;
	traverser->historyPointer &= (HISTORY_BUFFER_SIZE-1);
	traverser->opcodePointer++;
	traverser->opcodePointer &= MAX_NUMBER_OF_HISTORIES - 1;
}

unsigned int rewindTraverser(struct historyTraverser *traverser) {
	struct stateHistory *history = traverser->history;
	unsigned int opcodePointer = traverser->opcodePointer;
	unsigned char dataSize = history->opcodes[opcodePointer].dataSize;
	if((opcodePointer == history->opcodeStart)
		||
		(((traverser->historyPointer - history->historyStart) &
			(HISTORY_BUFFER_SIZE - 1)))
			< dataSize) {
		return HISTORY_SEEK_RESULT_FAIL;
	}
	else {
		traverser->historyPointer -= dataSize;
		traverser->historyPointer &= HISTORY_BUFFER_SIZE - 1;
		traverser->opcodePointer--;
		traverser->opcodePointer &= MAX_NUMBER_OF_HISTORIES - 1;
		return HISTORY_SEEK_RESULT_SUCCESS;
	}
}

unsigned int fastForward(struct historyTraverser *traverser) {

}

struct opcodeRecord *getOpcodeRecord(struct historyTraverser *traverser) {
	return &traverser->history->opcodes[traverser->opcodePointer];
}

//write history...if there is an overflow, write both at the end and
//at the beginning...  technically we don't even have to write at beginning


