#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semaphore.h"
#include "list.h"

int pidCounter;

PCB* runningProcess;
PCB* init;


List* priority0;
List* priority1;
List* priority2;

List* waitSend;
List* waitReceive;

List* semaphores;

void printList(List* list, char* listName, int flag) {
	printf("%s:\n", listName);
	if (List_first(list) == NULL) {
		printf("EMPTY\n");
		return;
	}
	if (flag == 0) printf("<--");
	while (List_curr(list) != NULL) {
		if (flag == 0 || flag == 3) {
			PCB* pcb = List_curr(list);
			printf("[PID:%d %s]", pcb->PID, translateState(pcb->state));
		}
		else if (flag == 1) {
			Semaphore* semaphore = List_curr(list);
			printf("[SID:%d VAL:%d]", semaphore->SID, semaphore->value);
		}
		else if (flag == 2) {
			PCB* pcb = List_curr(list);
			printf("[PID:%d %s MSG:%s TO:%d]", pcb->PID, translateState(pcb->state), pcb->message, pcb->targetPID);
		}
		List_next(list);
	}
	if (flag == 0) printf("<--\n");
	else printf("\n");
}

void totalInfo() {
	printf("------ CURRENT STATE ------>\n");
	
	if (runningProcess == NULL) printf("[RUNNING: NULL]\n\n");
	else if (runningProcess->PID == 0) printf("[RUNNING: init]\n\n");
	else printf("[RUNNING: %d]\n\n", runningProcess->PID);
	
	printList(priority0, "Priority 0 Queue", 0);
	printList(priority1, "Priority 1 Queue", 0);
	printList(priority2, "Priority 2 Queue", 0);
	printList(semaphores, "\nSemaphores", 1);
	printList(waitSend, "\nProcesses Waiting On Send", 2);	
	printList(waitReceive, "Processes Waiting On Receive", 3);	
	
	printf("--------------------------->\n");
}

PCB* findHelper(List* list, int PID) {
	List_first(list);
	while (List_curr(list) != NULL) {
		PCB* curr = List_curr(list);
		if (curr->PID == PID) return curr;
		List_next(list);
	}
	return NULL;
};

void procInfo() {
		
	char str[10000];
	printf("Enter PID of process to be displayed:");
	scanf("%s", str);
	int ret = -1;
	int pid = atoi(str);

	printf("Searching for process %d...\n", pid);
	PCB* pcb;
	if (runningProcess->PID == pid) pcb = runningProcess;
	if (pcb == NULL) pcb = findHelper(priority0, pid);
	if (pcb == NULL) pcb = findHelper(priority1, pid);
	if (pcb == NULL) pcb = findHelper(priority2, pid);
	if (pcb == NULL) {
		printf("Process not found.\n");
		return;
	}

	printf("------ PROCESS STATE ------>\n");
	printf("PID: %d\n", pcb->PID);
	printf("priority: %d\n", pcb->priority);
	printf("state: %s\n", translateState(pcb->state));
	printf("message: %s\n", pcb->message);
	printf("target PID: %d\n", pcb->targetPID);
	printf("inbox: %s\n", pcb->reply);
	printf("--------------------------->\n");
}

void loadProcess() {

	printf("Loading successor process from priority queue...\n");
	PCB* next;
	int priority;
	if (List_first(priority0) != NULL) {
		next = List_first(priority0);
		List_remove(priority0);
		priority = 0;
	}
	else if (List_first(priority1) != NULL) {
		next = List_first(priority1);
		List_remove(priority1);
		priority = 1;
	}
	else if (List_first(priority2) != NULL) {
		next = List_first(priority2);
		List_remove(priority2);
		priority = 2;
	}
	else {
		printf("No other process found, running init.\n");
		runningProcess = init;
		runningProcess->state = RUNNING;
		return;
	}
	printf("Process %d loaded from head of priority %d queue.\n", next->PID, priority);

	runningProcess = next;
	runningProcess->state = RUNNING;
}

void update() {

	if (runningProcess->PID != 0) {
		printf("Appending current running process to end of queue...\n");
		List* priorityList;
		if (runningProcess->priority == 0) priorityList = priority0;
		else if (runningProcess->priority == 1) priorityList = priority1;
		else if (runningProcess->priority == 2) priorityList = priority2;
		List_append(priorityList, runningProcess);
		runningProcess->state = READY;
		printf("Appended process %d to the end of priority queue %d.\n", runningProcess->PID, runningProcess->priority);
	}

	loadProcess();

	totalInfo();
	printf("END\n");
}

int createProcess() {
	
	char str[1];
	printf("Enter process priority:");
	scanf("%s", str);
	int ret;

	printf("Creating new process...\n");
	int priority = str[0] - '0';
	PCB* pcb = PCB_create(pidCounter, priority, READY, NULL, -1);
	printf("New process created.\n");

	if (runningProcess->PID == 0) {
		printf("Replacing init with process...\n");
		runningProcess = pcb;
		pidCounter++;
		ret = pcb->PID;
		totalInfo();
		printf("END\n");
		return ret;
	}
	else {
		printf("Adding process to priority queue %d...\n", priority);
		List* priorityList;
		if (priority == 0) priorityList = priority0;
		else if (priority == 1) priorityList = priority1;
		else if (priority == 2) priorityList = priority2;
		else {
			printf("ERROR: Invalid input for priority.\n");
			return -1;
		}
		if (List_append(priorityList, pcb) == -1) {
			printf("ERROR: Failed to add PCB to priority list.\n");
			return -1;
		}
		else ret = pcb->PID;
		pidCounter++;
		printf("Add successful.\n");
	}

	totalInfo();
	printf("END\n");
	return ret;
};

int processFork() {
	
	if (runningProcess->PID == 0) {
		printf("ERROR: Cannot fork from init.\n");
		return -1;
	}

	printf("Copying running process...\n");
	PCB* pcb = PCB_copy(runningProcess, pidCounter);
	pidCounter++;

	printf("Adding to corresponding priority list...\n");
	List* priorityList;
	if (pcb->priority == 0) priorityList = priority0;
	else if (pcb->priority == 1) priorityList = priority1;
	else if (pcb->priority == 2) priorityList = priority2;
	else {
		printf("ERROR: Invalid target priority.\n");
		return -1;
	}

	if (List_append(priorityList, pcb) == -1) {
		printf("ERROR: Failed to add PCB to priority list.\n");
		return -1;
	}

	printf("END\n");
	return pcb->PID;
};

int killHelper(List* list, int PID) {
	List_first(list);
	while (List_curr(list) != NULL) {
		PCB* curr = List_curr(list);
		if (curr->PID == PID) {
			printf("Found, removing process...\n");
			List_remove(list);
			PCB_free(curr);
			return 0;
		}
		List_next(list);
	}
	printf("Not found.\n");
	return -1;
};

int processExit() {

	if (runningProcess->PID == 0) exit(0); 
	if (runningProcess == NULL) {
		printf("No running process found.\n");
		return -1;
	}
	else {
		int PID = runningProcess->PID;
		PCB_free(runningProcess);
		loadProcess();
		totalInfo();
		return 0;
	}
};

int kill() {
	
	char str[1024];
	printf("Enter PID of process to be terminated:");
	scanf("%s", str);
	int ret = -1;
	int PID = atoi(str);

	if (PID == 0 && runningProcess == init) exit(0);

	printf("Checking current running process...\n");
	if (runningProcess->PID == PID) {
		printf("Found, removing process...\n");
		processExit();
		return 0;
	}

	printf("Initiating kill helper...\n");
	printf("Searching priority 0...\n");
	if (ret = killHelper(priority0, PID) == 0) return ret;
	printf("Searching priority 1...\n");
	if (ret = killHelper(priority1, PID) == 0) return ret;
	printf("Searching priority 2...\n");
	if (ret = killHelper(priority2, PID) == 0) return ret;

	totalInfo();
	printf("END\n");
	return ret;
};

int send() {

	char str[1024];
	printf("Enter PID of process to send to:");
	scanf("%s", str);
	int targetPID = atoi(str);

	char str2[40];
	printf("Enter message to send:");
	scanf("%s", str2);

	printf("Searching in processes waiting on receive...\n");
	List_first(waitReceive);
	while (List_curr(waitReceive) != NULL) {
		PCB* curr = List_curr(waitReceive);
		if (curr->PID == targetPID) {
			printf("Receiver found.\n");
			curr->reply = str2;
			curr->state = READY;
			List* priorityList;
			if (curr->priority == 0) priorityList = priority0;
			else if (curr->priority == 1) priorityList = priority1;
			else if (curr->priority == 2) priorityList = priority2;
			else {
				printf("ERROR: Invalid target priority.\n");
				return -1;
			}
			List_prepend(priorityList, curr);
			List_remove(waitReceive);
			return 0;
		}
		List_next(waitReceive);
	}

	printf("Blocking and saving current running process to waitlist...\n");
	runningProcess->targetPID = targetPID;
	runningProcess->message = str2;
	runningProcess->state = BLOCKED;	
	List_append(waitSend, runningProcess);
	loadProcess();

	return 0;
};

void receive() {
	
	printf("Searching for incoming messages...\n");
	List_first(waitSend);
	while (List_curr(waitSend) != NULL) {
		PCB* curr = List_curr(waitSend);
		if (curr->targetPID == runningProcess->PID) {
			printf("Message received: %s\n", curr->message);
			return;
		}
		List_next(waitSend);
	}

	printf("No message found. Blocking and saving current running process to waitlist...\n");
	runningProcess->state = BLOCKED;
	List_append(waitReceive, runningProcess);
	loadProcess();
};

int reply() {

	char str[1024];
	printf("Enter PID of process to reply to:");
	scanf("%s", str);
	int PID = atoi(str);

	char str2[40];
	printf("Enter message of reply:");
	scanf("%s", str2);

	printf("Searching for blocked sender...\n");
	List_first(waitSend);
	while (List_curr(waitSend) != NULL) {
		PCB* curr = List_curr(waitSend);
		if (curr->PID == PID) {
			printf("Sender found, replying...\n");
			curr->reply = str2;
			printf("Unblocking sender...\n");
			curr->state = READY;
			printf("Moving to corresponding priority list...\n");
			List* priorityList;
			if (curr->priority == 0) priorityList = priority0;
			else if (curr->priority == 1) priorityList = priority1;
			else if (curr->priority == 2) priorityList = priority2;
			else {
				printf("ERROR: Invalid target priority.\n");
				return -1;
			}
			List_prepend(priorityList, curr);
			List_remove(waitSend);
			return 0;
		}
		List_next(waitSend);
	}

	printf("ERROR: Sender cannot be found.\n");
	return -1;
};

int newSemaphore() {

	char str[1024];
	printf("Enter ID of new semaphore:");
	scanf("%s", str);
	int SID = atoi(str);
	List_first(semaphores);
	while (List_curr(semaphores) != NULL) {
		Semaphore* curr = List_curr(semaphores);
		if (curr->SID == SID) {
			printf("ERROR: SID already exists.\n");
			return -1;
		}
		List_next(semaphores);
	}

	char str2[1024];
	printf("Enter value of the semaphore:");
	scanf("%s", str2);
	int value = atoi(str2);
	if (value != 0 && value != 1 && value != 2 && value != 3 && value != 4) {
		printf("ERROR: Invalid value input.\n");
		return -1;
	}

	printf("Creating new semaphore...\n");
	Semaphore* semaphore = createSemaphore(SID, value);

	printf("Adding semaphore to list...\n");
	if (List_append(semaphores, semaphore) == -1) {
		printf("ERROR: Failed to add semaphore to list.\n");
		return -1;
	}

	printf("END\n");
	return 0;
};

int semaphoreP() {

	if (runningProcess->PID == 0) {
		printf("ERROR: PV operations cannot be applied to init.\n");
		return -1;
	}

	char str[1024];
	printf("Enter ID of semaphore:");
	scanf("%s", str);
	int SID = atoi(str);
	List_first(semaphores);
	while (List_curr(semaphores) != NULL) {
		Semaphore* curr = List_curr(semaphores);
		if (curr->SID == SID) {
			printf("Semaphore found.\n");
			Semaphore_P(curr, runningProcess);
			loadProcess();
			return 0;
		}
		List_next(semaphores);
	}
	printf("Semaphore not found.\n");
	return -1;
};

int semaphoreV() {

	char str[1024];
	printf("Enter ID of semaphore:");
	scanf("%s", str);
	int SID = atoi(str);
	List_first(semaphores);
	while (List_curr(semaphores) != NULL) {
		Semaphore* curr = List_curr(semaphores);
		if (curr->SID == SID) {
			printf("Semaphore found.\n");
			PCB* pcb = Semaphore_V(curr);
			if (pcb == NULL) return -1;
			List* priorityList;
			if (pcb->priority == 0) priorityList = priority0;
			else if (pcb->priority == 1) priorityList = priority1;
			else if (pcb->priority == 2) priorityList = priority2;
			else {
				printf("ERROR: Invalid target priority.\n");
				return -1;
			}
			List_prepend(priorityList, pcb);
			return 0;
		}
		List_next(semaphores);
	}
	printf("Semaphore not found.\n");
	return -1;
};

int main(int argc, char *argv[]) {

	printf("Program Started.\n");

	pidCounter = 0;

	priority0 = List_create();
	priority1 = List_create();
	priority2 = List_create();

	waitSend = List_create();
	waitReceive = List_create();

	semaphores = List_create();

	init = PCB_create(pidCounter, 2, RUNNING, "init", -1);
	pidCounter++;
	runningProcess = init;
	
	char str[1];
	while (true) {
		int ret = -2;
		printf("\nEnter command:\n");
		scanf("%s", str);
		switch(str[0]) {
			
			case 'C':
			ret = createProcess();
			if (ret == -1) printf("Failure, process creation did not proceed as expected.\n");
			else printf("Success, process created. PID: %d\n", ret);
			break;

			case 'F':
			ret = processFork();
			if (ret == -1) printf("Failure, process fork did not proceed as expected.\n");
			else printf("Success, process forked. PID: %d\n", ret);
			break;

			case 'K':
			ret = kill();
			if (ret == -1) printf("Failure, process kill did not proceed as expected.\n");
			else printf("Process successfully killed.\n");
			break;

			case 'E':
			ret = processExit();
			if (ret == -1) printf("Failure, process exit did not proceed as expected.\n");
			else printf("Running process successfully killed.\n");
			break;

			case 'Q':
			printf("Update initiated.\n");
			update();
			break;

			case 'S':
			ret = send();
			if (ret == -1) printf("Failure, cannot send message.\n");
			else printf("Message sent.\n");
			break;	

			case 'R':
			receive();
			break;

			case 'Y':
			ret = reply();
			if (ret == -1) printf("Failure, reply did not proceed as expected.\n");
			else printf("Reply sent.\n");
			break;

			case 'N':
			ret = newSemaphore();
			if (ret == -1) printf("Failed to create semaphore.\n");
			else printf("Semaphore created successfully\n");
			break;

			case 'P':
			ret = semaphoreP();
			if (ret == -1) printf("Failed to execute P operation on current running process.\n");
			else printf("P operation successfully executed.\n");
			break;	 

			case 'V':
			ret = semaphoreV();
			if (ret == -1) printf("Failed to execute V operation on semaphore.\n");
			else printf("V operation successfully executed.\n");
			break;

			case 'I':
			procInfo();
			break;

			case 'T':
			totalInfo();
			break;	    		    		    	   		    		    		    	   		    		    		    
		}
	}
}

