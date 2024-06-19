#include <stdlib.h>
#include "queue.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

//function to initialize queue
void queueInit(queue *myQueue){
	myQueue-> head = NULL;
	myQueue->tail = NULL;
	myQueue-> count= 0;
}

//function to enqueue tasks in a queue
void enqueue(queue *myQ , task toEnqueue){
	//create new node
	node *newNode = malloc(sizeof(node));
	
	newNode -> taskData = malloc(sizeof(task));
	
	memcpy(newNode -> taskData, &toEnqueue, sizeof(task));

	if (newNode -> taskData == NULL){
		printf("Memory allocation failed\n");
		free(newNode);
		exit(EXIT_FAILURE);
	}
	newNode ->next = NULL;

	//add to tail if there is a tail
	if(myQ->tail != NULL){
		myQ->tail->next = newNode;
	}
	myQ->tail = newNode;

	//if queue empty add to head
	if (myQ->head == NULL){
		myQ->head = newNode;
	}

	myQ -> count++;
}

//function to dequeue tasks from a queue
//and return the node
node* dequeue(queue *myQ){
	//queue empty
	if (myQ -> head == NULL){
		return NULL;
	}

	node *temp = myQ->head;
	
	//remove the node
	myQ->head = myQ->head->next;
	if (myQ->head == NULL){
		myQ->tail = NULL;
	}
	
	myQ->count--;

	return temp;
}

void printQueue(queue *myQ){
	node* current = myQ->head;

	int count = 0 ;
	while(current != NULL){

		printf("Task %d = %s\n", count , current->taskData->name);
		count++;
		
		current = current->next;
	}
}
