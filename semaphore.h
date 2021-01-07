#include "list.h"
#include "pcb.h"

typedef struct Semaphore {
	int SID;
	int value;
	List* pList;
} Semaphore;

Semaphore* createSemaphore(int SID, int value);

void Semaphore_P(Semaphore* s, PCB* pcb);

PCB* Semaphore_V(Semaphore* s);