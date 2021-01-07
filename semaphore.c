#include <stdlib.h>
#include <stdio.h>

#include "semaphore.h"

Semaphore* createSemaphore(int SID, int value) {
	Semaphore* s = malloc(sizeof(struct Semaphore));
	s->SID = SID;
	s->value = value;
	s->pList = List_create();
	return s;
}

void Semaphore_P(Semaphore* s, PCB* pcb) {
	printf("Executing P operation...\n");
	s->value--;
	if (s->value <= 0) {
		List_append(s->pList, pcb);
		pcb->state = BLOCKED;
		printf("Process blocked.\n");
	}
	else printf("Block failed.\n");
};

PCB* Semaphore_V(Semaphore* s) {
	printf("Executing V operation...\n");
	s->value++;
	if (List_first(s->pList) == NULL) printf("NULL!\n");
	if (s->value >= 0 && List_first(s->pList) != NULL) {
		PCB* pcb = List_remove(s->pList);
		pcb->state = READY;
		printf("Process unblocked.\n");
		return pcb;
	}
	else {
		printf("Unblock failed.\n");
		return NULL;
	}
};