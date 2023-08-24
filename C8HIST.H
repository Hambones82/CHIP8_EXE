#ifndef C8HIST_H
#define C8HIST_H

struct CLSChangeHistory {
	unsigned char bytes[256];//256 = 32 * 8
};

struct wideUpdateHistory {
	unsigned char bytes[16];
	unsigned int Idiff;
};

struct pushPCHistory {
	unsigned int stackDiff;
};

struct drawChangeHistory {
	unsigned char VFChange;
};

struct jumpHistory {
	unsigned int PCdiff;
};

struct callRetHistory {
	unsigned int PCdiff;
	unsigned int stackDiff;
};

struct setVxHistory {
	unsigned char VxDiff;
};

struct setVxVFHistory {
	unsigned char VxDiff;
	unsigned char VFDiff;
};

struct setIHistory {
	unsigned int iDiff;
};

struct BCDHistory {
	unsigned char digitDiffs[3];
};
//--end specific structs
struct opcodeRecord {
	unsigned int PC;
	unsigned int opcode;
	unsigned char dataSize;
};
//these values must be a power of 2
#define MAX_NUMBER_OF_HISTORIES     64
#define HISTORY_BUFFER_SIZE         1024
#define BUFFER_OVERFLOW_SIZE        256
struct stateHistory {
	unsigned int opcodeStart;//lowest valid entry
	unsigned int opcodeLength; //highest valid entry
	struct opcodeRecord opcodes[MAX_NUMBER_OF_HISTORIES];
	unsigned int historyStart;
	unsigned int historyLength;
	unsigned char instructionHistories
		[HISTORY_BUFFER_SIZE+BUFFER_OVERFLOW_SIZE];
};

struct historyTraverser {
	unsigned int opcodePointer;
	unsigned int historyPointer;
		//points to the first change for the instruction
	struct stateHistory *history;
};

struct historyTraverser *getHistoryTraverser(struct stateHistory *history);
struct stateHistory *initStateHistory(void);

#define HISTORY_SEEK_RESULT_FAIL    0
#define HISTORY_SEEK_RESULT_SUCCESS 1
unsigned int rewindTraverser(struct historyTraverser *traverser);
unsigned int fastForward(struct historyTraverser *traverser);
void advanceTraverserWithAllocate(struct historyTraverser *traverser,
				   unsigned int size);
//remove what's below?
//void allocateHistoryEntry(struct historyTraverser *traverser,
//    unsigned int size);
void recordInstruction(struct historyTraverser *traverser,
					   unsigned char *bytes,
					   struct opcodeRecord *opcode);
void *getChanges(struct stateHistory *history,
				 struct historyTraverser *traverser);
struct opcodeRecord *getOpcodeRecord(struct historyTraverser *traverser);

//add stuff in...

#endif

