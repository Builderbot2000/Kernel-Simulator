#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pcb.h"

const char* translateState(enum State state) {
	switch (state) {
		case RUNNING: return "RUNNING";
		case READY: return "READY";
		case BLOCKED: return "BLOCKED";
		case DEADLOCKED: return "DEADLOCKED";
	}
}

PCB* PCB_create(int PID, int priority, enum State state, char* message, int targetPID) {
	PCB* pcb = malloc(sizeof(struct PCB));
	pcb->PID = PID;
	pcb->priority = priority;
	pcb->state = state;
	if (message != NULL) pcb->message = strdup(message);
	else pcb->message = NULL;
	pcb->targetPID = targetPID;
	pcb->reply = NULL;
	return pcb;
};

PCB* PCB_copy(PCB* template, int PID) {
	PCB* pcb = malloc(sizeof(struct PCB));
	pcb->PID = PID;
	pcb->priority = template->priority;
	enum State state = READY;
	pcb->state = state;
	pcb->message = template->message;
	pcb->targetPID = template->targetPID;
	pcb->reply = template->reply;
	return pcb;
};

void PCB_free(PCB* pcb) {
	if (pcb == NULL) {
		printf("ERROR: Null pointer.\n");
		return;
	}
	free(pcb);
};