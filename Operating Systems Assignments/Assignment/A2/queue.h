#ifndef QUEUE_H
#define QUEUE_H

#include "tasks.h"

//struct for node 
//to create queue
typedef struct NODE{
	task *taskData;
	struct NODE *next;
}node;

//queue struct and count to know the 
//number of elements in the queue
typedef struct QUEUE{
	node *head;
	node *tail;
	int count;
}queue;


//function declarations
void queueInit(queue*);
void enqueue( queue*, task);
int getQueueCount();
node* dequeue( queue *);
void printQueue(queue *);
#endif
