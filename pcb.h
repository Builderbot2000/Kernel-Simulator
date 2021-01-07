enum State {RUNNING, READY, BLOCKED, DEADLOCKED};

const char* translateState(enum State state);

typedef struct PCB {
   	int PID;
   	int priority;
   	enum State state;
   	char* message;
   	int targetPID;
   	char* reply;
} PCB;

PCB* PCB_create(int PID, int priority, enum State state, char* message, int targetPID);

PCB* PCB_copy(PCB* pcb, int PID);

void PCB_free(PCB* pcb);
