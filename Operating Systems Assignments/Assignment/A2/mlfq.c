#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "mlfq.h"
#include "queue.h"
#include "tasks.h"

#define TIMEALLO 200
//function to initialize the mlfq
//and allocating the necessary variables
void mlfqInit(mlfq *mlfqObj){
	*mlfqObj = *(mlfq*) malloc(sizeof(mlfq));

	mlfqObj->priority1 = (queue*) malloc(sizeof(queue));
	queueInit(mlfqObj->priority1);
	
	mlfqObj->priority2 = (queue*) malloc(sizeof(queue));
	queueInit(mlfqObj->priority2);
	
	mlfqObj->priority3 = (queue*) malloc(sizeof(queue));
	queueInit(mlfqObj->priority3);

	mlfqObj->priority4 = (queue*) malloc(sizeof(queue));
	queueInit(mlfqObj->priority4);


}

//function to add to mlfq
void enterMLFQ(task toAdd, mlfq* mlfqObj){
	enqueue(mlfqObj->priority1, toAdd);
}
//function to send the task back to the mlfq
void sendToMLFQ(task *toAdd, mlfq* mlfqObj){
	int getPrio = toAdd->priority;

	//just in case priority is incremented more than 4
	if (getPrio >4){
		getPrio = 4;
	}

	if (getPrio == 1)
		enqueue(mlfqObj->priority1, *toAdd);
	else if (getPrio == 2)
		enqueue(mlfqObj->priority2, *toAdd);
	else if (getPrio == 3)
		enqueue(mlfqObj->priority3, *toAdd);
	else	
		enqueue(mlfqObj->priority4, *toAdd);
	
}
//function to get next 
task* getNext(mlfq *mlfqObj){
	assert(mlfqObj !=NULL);	
	
	node *getNode;
	task *getTask = NULL; 
   	int mlfqCount = mlfqObj->priority1->count + mlfqObj->priority2->count + mlfqObj->priority3->count + mlfqObj->priority4->count;	
	
	//check if mlfq not empty
	if (mlfqCount != 0){
		if (mlfqObj->priority1->count != 0){
			getNode = dequeue(mlfqObj->priority1);
			getTask = getNode->taskData;	
		}else if (mlfqObj->priority2 ->count != 0 ){
			getNode = dequeue(mlfqObj->priority2);
			getTask = getNode->taskData;		
		}else if(mlfqObj->priority3->count !=0 ){
			getNode = dequeue(mlfqObj->priority3);
			getTask = getNode->taskData;	
		}
		else{
			getNode = dequeue(mlfqObj->priority4);
			getTask = getNode->taskData;	
		}
	}

	return getTask;
}

//get task and decrease its priority
void decreasePriority(task *toDecrease){
	assert(toDecrease!= NULL);

	//decrease the priority level	
	if (toDecrease->priority < 4)
		toDecrease->priority++;
	
	//reset its time to run on CPU
	toDecrease->timeAllotment = TIMEALLO;
}

//implement destroy mlfq
//clean after use
void cleanMLFQ(mlfq *mlfqObj){
	free(mlfqObj->priority1);
	free(mlfqObj->priority2);
	free(mlfqObj->priority3);
	free(mlfqObj->priority4);

}
