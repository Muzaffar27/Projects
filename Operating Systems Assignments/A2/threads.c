#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "queue.h"
#include "threads.h"
#include <pthread.h>
#include "argsStruct.h"
#include "tasks.h"
#include <string.h>
#include "mlfq.h"
#include <time.h>
#define NANOS_PER_USEC 1000
#define USEC_PER_SEC   1000000
#define QUANTUM 50
#define TIMEALLO 200



//declaring locks
pthread_mutex_t loadLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t readyLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dispatchLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t jobLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t waitLock = PTHREAD_MUTEX_INITIALIZER;

//declaring condtions
pthread_cond_t readyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t loadCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t dispatchCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t jobCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t waitCond = PTHREAD_COND_INITIALIZER;

//Global varable
//shared variables
int over = 0;   //check if file read is over
int totalWorkers = 0; 
int startDispatcher = 0;
int ready = 0;  //check if the data pool is ready
int mlfqDone = 0; //check if mlfq is empty or not

//data pool for wokers thread to get data from
dataPool dataPoolObj;

//to know the number of total jobs
int numOfJobs = 0;

//to knwo how many jobs are done
int jobDone = 0;

//our workers pointers
pthread_t *workers;
//--------------------------------------------------------------------------------------
//dispatcher threads to make work available
void *scheduler(void *data){

	//our local variables
	node *dequeued;
	task *deTask;
	mlfq mlfqObj;	
	pthread_t dispatch;	

	//convert argument to task queue
	queue *taskQ = (queue*) data;
	
	//initialize multi level feedback Queue
	mlfqInit(&mlfqObj);
		
	while(taskQ->count != 0 || over != -1){
		//lock before chcking if we can dequeue
		pthread_mutex_lock(&loadLock);
			
		//check if queue is empty or not	
		if(taskQ->count == 0){
			
			//wait for at least a task is added to ready queue
			pthread_cond_wait(&loadCond, &loadLock);	
		}
	
		//dequeue and get task
		//as  queue not empty
		dequeued = dequeue(taskQ);
		pthread_mutex_unlock(&loadLock);
		
		//we can now use the dequeued task as its not in 
		//critical section anymore
		//and set its priority to 1
		deTask = dequeued->taskData;
		deTask->priority = 1;		
		deTask->timeAllotment = TIMEALLO;	
		//enters the mlfq for the first time
		enterMLFQ(*deTask, &mlfqObj);
	}

	printf("All %d jobs entered the mlfq\n", numOfJobs);
	
	//start dispatcher after entering all task in mlfq
	pthread_create(&dispatch, NULL, dispatcher, (void*) &mlfqObj);
	
	//make dispatcher know that it can start working now
	pthread_mutex_lock(&dispatchLock);
	startDispatcher = 1;
	pthread_cond_signal(&dispatchCond);
	pthread_mutex_unlock(&dispatchLock);	

	//wait for dispatcher to end
	pthread_join(dispatch, NULL);

	//clean our resources	
	cleanThreads();

	//end scheduler
	pthread_exit(NULL);
}
//----------------------------------------------------------------------------------
//dispatcher to notify the CPUs
void *dispatcher(void *data){
	
	//local variable
	mlfq *mlfqObj = (mlfq*) data;
	assert(mlfqObj !=NULL);
	task* temp;
	node* getNode;
	
	//struct timespec startTime, endTime;
		
	//initialize a data pool queue
	//so that workerscan take from the ready queue	
	queueInit(&dataPoolObj.readyQ);

	//if task not complete it goes to waiting queue
	queueInit(&dataPoolObj.waitingQ);	

	//wait for the mlfq to be loaded
	pthread_mutex_lock(&dispatchLock);
	while(startDispatcher != 1){
		pthread_cond_wait(&dispatchCond, &dispatchLock);
	}
	pthread_mutex_unlock(&dispatchLock);
	
	//arrival time
	//clock_gettime(CLOCK_MONOTONIC, &startTime);

	//runs while tasks are still available to run
	while(jobDone!=numOfJobs){
	//check if there is job in mlfq
		pthread_mutex_lock(&jobLock);	
		if (mlfqObj->priority1->count != 0 ||
	   	   (mlfqObj->priority1->count == 0 && mlfqObj->priority2->count != 0) ||
	   	   (mlfqObj->priority1->count == 0 && mlfqObj->priority2->count == 0 && mlfqObj->priority3->count != 0) || 
           	   (mlfqObj->priority1->count == 0 && mlfqObj->priority2->count == 0 && mlfqObj->priority3->count == 0 && mlfqObj->priority4->count != 0)) {
    			if (mlfqObj->priority1->count != 0)
        			getNode = dequeue(mlfqObj->priority1);
    			else if (mlfqObj->priority2->count != 0)
        			getNode = dequeue(mlfqObj->priority2);
    			else if (mlfqObj->priority3->count != 0)
        			getNode = dequeue(mlfqObj->priority3);
    			else
        			getNode = dequeue(mlfqObj->priority4);

			//get task and send to readyQ
    			temp = getNode->taskData;
    			enqueue(&dataPoolObj.readyQ, *temp);
    			ready = 1;
    			pthread_cond_broadcast(&jobCond);
		}
		//check if there is job in waitingQ
		pthread_mutex_trylock(&waitLock);
		if (dataPoolObj.waitingQ.count != 0){
			getNode = dequeue(&dataPoolObj.waitingQ);
			temp = getNode->taskData;
			sendToMLFQ(temp, mlfqObj);
		}
		pthread_mutex_unlock(&waitLock);
	
		pthread_mutex_unlock(&jobLock);		
		
		
	}
	
	//clock_gettime(CLOCK_MONOTONIC, &endTime);

  	//long timeDiff = (endTime.tv_sec - startTime.tv_sec) * 1000000 + (endTime.tv_nsec - startTime.tv_nsec) / 1000;	
	
//	printf("Turnaround time = %lu\n", timeDiff);
	return NULL;	
}
//--------------------------------------------------------------------------------------
//workers thread
void *job(){
	
	//a task variable
	//that will be executed
	task *taskToExe;	
 	//int randNum ;
	//int IODone = 0;

	while(jobDone != numOfJobs){

		pthread_mutex_lock(&jobLock);
		//wait for job to be ready
		if (!ready){
			pthread_cond_wait(&jobCond, &jobLock);
	
		}
		//if there is data in the data pool
		//we consume it and do the work
		if (dataPoolObj.readyQ.count > 0){
			taskToExe = (dequeue(&dataPoolObj.readyQ))->taskData;	
			
			//run for the time slice
			if (taskToExe->length > QUANTUM ){
				microsleep(QUANTUM);
				
				//reflect the amount of time left to run on CPU
				taskToExe->length = taskToExe->length-QUANTUM;

				//reduce the amount of time in the current queue
				//adhere to time allotment
				taskToExe->timeAllotment = taskToExe->timeAllotment - QUANTUM;
				
				if (taskToExe->timeAllotment <= 0){
					//decrease priority
					decreasePriority(taskToExe);
				}

				//current task goes to waiting list after using time slice	
				pthread_mutex_lock(&waitLock);
				enqueue(&dataPoolObj.waitingQ, *taskToExe);
				pthread_mutex_unlock(&waitLock);
		
			}
			else{
				//jobDone++ HERE
				microsleep(taskToExe->length);
				printf("%d Job %s Done\n",jobDone, taskToExe->name);
		
				jobDone++;	
			}
			
		}else{
			//we notify that is no task in the data pool
			ready = 0;
		}
		pthread_mutex_unlock(&jobLock);

	}
	
	return NULL;
}
//----------------------------------------------------------------------------------------
// function to load tasks from file
void *loadTask(void *data){
	runArgs *args = ( runArgs*) data;
	
	printf("Filename = %s\n", args->filename);	

	//Declarations of local variables		
	FILE *file = fopen(args->filename, "r");
	size_t bufferSize = 100;
	char *tempLine = (char*) malloc (bufferSize * sizeof (char)) ;
	char *token[4];
	task *currTask;
	int cpuNum = args->cpuNo;	

	//stop if we could not allocate memory
	if (tempLine == NULL){
		printf("No more memory to allocate\n");
		exit(EXIT_FAILURE);
	}
	
	//initialize the workers thread
	createThreads(cpuNum);

	//read line by line
	//break it into 4 part
	//with the 1st part (the name) used, to know if delay or not

	over = getline(&tempLine, &bufferSize, file);
	
	while( over != -1 ){

		//get first token to know
		//if it's DELAY or a TASK
		token[0] = strtok(tempLine, " ");
	
		//if delay we make go to sleep that much time	
		if (strcmp(token[0], "DELAY") == 0){
			token[1] = strtok(NULL," ");
			printf("Delaying for %d millisec\n", atoi(token[1]));
			microsleep(atoi(token[1]) * 1000);
		}else{	
			//allocate memory for the task node and the name of the task
			currTask = (task*) malloc(sizeof(task));
			currTask->name = (char*) malloc(bufferSize * sizeof (char));
		
			//py name and store into task node
			strcpy(currTask->name, token[0]);

			//store the rest into the array
			for (int i = 1; i< 4 ; i++)
				token[i] = strtok(NULL, " ");

			//cpy the rest of the data
			currTask->type = atoi(token[1]);
			currTask->length = atoi(token[2]);
			currTask->odds = atoi(token[3]);
	
			//lock before
			//add to queue	
			pthread_mutex_lock(&loadLock);
			numOfJobs++;
			enqueue(args->runningQ, *currTask);
				
			//signal scheduler after enqueueing
			pthread_cond_signal(&loadCond);
			pthread_mutex_unlock(&loadLock);
			
		}
		//read next line
		over = getline(&tempLine, &bufferSize, file);
	}

	//wait for jobs

	fclose(file);
	
	//deallocate memory
	free(tempLine);
	free(args);

	return NULL;
}
//--------------------------------------------------------------------------------------
//create the threads based on the number of cpu given
void createThreads(int num){
	//assert we have at least 1 CPU
	assert(num >= 0);

    	workers = (pthread_t *)malloc(num * sizeof(pthread_t));	

	//store in global var to clean later on		
	totalWorkers = num;

	//create threads
	for (int i = 0 ; i< num ;i ++){
		pthread_create(&workers[i], NULL, job , NULL );
	}	
}	
//-------------------------------------------------------------------------------------
void microsleep(unsigned int usecs)
{
    long seconds = usecs / USEC_PER_SEC;
    long nanos   = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t = { .tv_sec = seconds, .tv_nsec = nanos };
    int ret;
    do
    {
        ret = nanosleep( &t, &t );
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}
//-------------------------------------------------------------------------------------
void cleanThreads(){
	//all threads over
	//cleaning resources
	printf("Cleaning after threads\n");
	pthread_mutex_destroy(&loadLock);
	pthread_cond_destroy(&loadCond);
	pthread_mutex_destroy(&readyLock);
	pthread_cond_destroy(&readyCond);	
	pthread_mutex_destroy(&dispatchLock);
	pthread_cond_destroy(&dispatchCond);	
	pthread_mutex_destroy(&jobLock);
	pthread_cond_destroy(&jobCond);	

	//wait for all workers to finish
	for (int i = 0; i< totalWorkers; i++){
		pthread_join(workers[i], NULL);
		printf("Workers dead\n");
	}
}

 
