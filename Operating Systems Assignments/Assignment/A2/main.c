//----------------------------
//Name: Bhye Muzaffar Alii Rohomuthally
//Student Id: 7871794
//Course : Comp 3430
//Instructor: Saulo Santos
//Assignment: Assignment 2
//------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include "tasks.h"
#include "queue.h"
#include <pthread.h>
#include "threads.h"
#include "argsStruct.h"

//---------------------------------
//define functions
//------------------------------

int main(int argc , char *argv[]){
	
	//assert conditions	
	assert(argc > 0);
	assert(argv != NULL);

	//local variables
	char *filename = "";
	int cpuNum = 0;
	pthread_t running, schedule;
 
	//check if all arguments are passed with the program
	if (argc != 4 ){
		printf("Missing Arguments, try again!\n");
		return EXIT_FAILURE;
	}

	//store the filename   
	filename = argv[3];	
	
	//get amount of cpu to create
	cpuNum = atoi(argv[1]);

	//create out taskQ 
	queue taskQ;

	//initialize the queue
	queueInit(&taskQ);

		
	//setting up the argument to send to
	//thread loading task	
	runArgs* args = malloc(sizeof (runArgs));
	args-> filename = filename;
	args->cpuNo = cpuNum;
	args->runningQ = &taskQ;
	//--------------------
	
	//creating running thread to load tasks
	if (pthread_create (&running, NULL, loadTask,args ) != 0){
		printf("Error creating running thread\n");	
		exit(EXIT_FAILURE);
	}

	//creating the schedule thread
	if (pthread_create (&schedule, NULL, scheduler, (void*) &taskQ ) != 0){
		printf("Error creating dispatcher thread\n");	
		exit(EXIT_FAILURE);
	}

	//wait for the 2 above threads to end
	pthread_join(running, NULL);
	pthread_join(schedule,NULL);

	printf("Main Program successfully Ended\n");
	return 0;
}

