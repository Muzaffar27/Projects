//------------------------------------------------
//Name: Bhye Muzaffar Alii Rohomuthally
//Student Id : 7871794
//Course : Comp 3430
//Instructor: Saulo Santos
//Assignment: Assignment 1 Question 3
//------------------------------------------------


//---------------------
//libraries
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
//----------------------

//function declaration
int openFile(char *);
void createThreads(int);
void *worker();
void signalHup();

//global variables
#define MAX 9
//to know number of process created
int configVal;
int threadNum = 0;  //keeping count of number of threads in play
char *filename;
//global thread array
pthread_t *threadArray;

int *runningThreads;

//----------------------------------------------------------
int main(int argc, char *argv[]){
	printf("Program starts\n");
	
	//check if file is supplied
	
	if (argc != 2){
		printf("File supplied incorrectly\n");
		return 1;
	}

	//get filename path
	filename = argv[1];
	
	signal(SIGHUP, signalHup);

	printf("Parent id = %d\n", getpid());
	
	//store config value
	configVal = openFile(filename);
	
	//initializing thread array
	threadArray = (pthread_t*) malloc (MAX * sizeof(pthread_t));

	//array of int to manage the threads
	//this array is used as a flag for each threads
	runningThreads = (int *) malloc (MAX * sizeof(int));

	//create threads read in file
	createThreads(configVal);
	

	//check if all threads exited
	if (threadNum <= 0){
		printf("No more Threads\n");
		//clear reasources
		free(threadArray);
		printf("Program ends\n");
		exit(EXIT_SUCCESS);
	}

	return 0;
}
//--------------------------------------------------------------
//function to inc/dec threads when signal hup is called
void signalHup(){

	//get new config value from file
	int oldConfig = configVal;

	configVal = openFile(filename);
	printf("New config value is %d \n", configVal);
	
	//temp var used to store the difference
	int temp = configVal - oldConfig;

	if (temp > 0){

		//check if new config value is over the limit
		if (configVal > MAX){
			printf("Cannot create more than MAX %d threads\n", MAX);
			//create how much we can without going up the 
			//limit
			temp = MAX - oldConfig;
			printf("Creating only %d threads\n", temp);
		}

		printf("%d more threads are being created\n", temp);
		//create threads
		createThreads(temp);
		
	}

	else if (temp < 0){
		
		//get the absolute value and
		//delete that much threads
		temp = temp * -1;
		
		if (temp > MAX)
			temp = MAX;

		printf("Deleting %d threads\n", temp);
		
		for (int i = 0; i< temp; i++){
			//set flag to 0
			runningThreads[threadNum--] = 0;
			
			printf("Thread %d deleted\n", threadNum);
		}
		
	}

	//no changes in config file
	else if (temp == 0){
		printf("Config value stayed the same. No change\n");
	}

	//no worker running
	if (threadNum <= 0 ){
		printf("All workers exited\n");
		free(threadArray);
		exit(EXIT_SUCCESS);
	}
	
	
}
//------------------------------------------------------------
void createThreads(int number){

	//assert conditions
	assert(number >=0);	

	int threadsToCreate = number;

	//check if number is over the limit
	if (number > MAX){
		printf("Can only create a MAX of %d threads\n", MAX);
		threadsToCreate = MAX;		
	}
	
	//create the threads
	for (int i = 0; i< threadsToCreate; i++){
		pthread_create (&threadArray[threadNum], NULL, worker, NULL);
		//set flag to 1
		//1 for running, 0 for not running
		runningThreads[i] = 1;
	}

	//join threads
	for (int i = 0; i< threadsToCreate; i++){
		pthread_join(threadArray[i], NULL);
	}

}

//----------------------------------------------------------
//worker threads
void *worker(){
	printf("Thread %d has started \n", threadNum++);
	
	//make them stay in loop while flag is 1
	while(runningThreads[threadNum-1] == 1){}
	
	//thread stopped running, we can exit thread
	pthread_exit(0);
}
//function to open file and returns the value 
//----------------------------------------------------------
int openFile(char *filename){

	//assert conditions
	assert(filename != NULL);

	int num;
	
	//open file
	FILE *file = fopen(filename, "r");

	//check if file is opened
	assert(file != NULL);

	//get number in config file
	fscanf(file, "%d", &num);

	//close the file
	fclose(file);

	//return config value
	return num;
}
