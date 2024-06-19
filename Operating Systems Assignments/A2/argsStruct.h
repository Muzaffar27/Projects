#ifndef ARGSSTRUCT_H
#define ARGSSTRUCT_H

#include "queue.h"
#include "tasks.h"

//struct to pass to thread functions
typedef struct RUNARGS{
	char *filename;
	int cpuNo;
	queue *runningQ;
}runArgs;

//used as a data pool so that workers 
//can get the task from this queue
typedef struct DATAPOOL{
	queue readyQ;	
	queue waitingQ;
}dataPool;

#endif
