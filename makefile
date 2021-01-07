os: os.o pcb.o semaphore.o list.o
	gcc os.o pcb.o semaphore.o list.o -o os

os.o: os.c pcb.h semaphore.h list.h
	gcc -c os.c

semaphore.o: semaphore.c semaphore.h
	gcc -c semaphore.c

pcb.o: pcb.c pcb.h
	gcc -c pcb.c 

clean:
	rm os.o pcb.o semaphore.o os