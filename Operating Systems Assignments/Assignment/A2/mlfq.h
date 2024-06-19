
#ifndef MLFQ_H
#define MLFQ_H

#include "mlfq.h"
#include "queue.h"

//struct for the mlfq
typedef struct MLFQ{
	queue *priority1;
	queue *priority2;
	queue *priority3;
	queue *priority4;
}mlfq;

//functions declarations
void mlfqInit(mlfq *);
void enterMLFQ(task, mlfq*);
void sendToMLFQ(task*, mlfq*);
void cleanMLFQ(mlfq *);
task* getNext(mlfq *);
void decreasePriority(task*);
#endif
