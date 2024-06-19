//------------------------------------------------------------
// Name: Bhye Muzaffar Alii Rohomuthally
// Student Id : 7871794
// Cours: Comp 3430
// Intructor: Saulo Santos
// Assignment: Assignment 1, Question 2
//------------------------------------------------------------

//-------------------
//libraries
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
//-------------------
//Declaring functions 
int openFile(char *);
void createProcess(int);
void storePid(pid_t);
void handleSigInt();
void handleSigHup();

//upper limit for number of threads
#define MAX 9

//Global Variables:
//global array so that all process have access to it
pid_t *arrayPid;
//to know how many process are actives
int pidCount = 0 ;

// global filename so that sig hup can open file
char *filename = "";
//to know how many process are already created
int configVal;
//--------------------------------------------------------
int main( int argc , char *argv[]){

	printf("Program starts\n");

	//Variable declarations
	//size of array List
	int size = MAX;

	//signals
	signal(SIGHUP, handleSigHup);
	signal(SIGINT, handleSigInt);
	
	//allocate array of pids
	//allocate 3 processes intially
	arrayPid = (pid_t *) malloc (size * sizeof (int) );

	//check if a file is supplied 
	if(argc != 2 ){
		printf("File supplied not correct\n");
		//stop program 
		return 1; 
	}

	//get file name
	filename = argv[1];
	
	printf("Parent id = %d\n", getpid());
		
        //get value from file	
	configVal = openFile(filename); 

	//check if value is not negative
	if (configVal < 0)	
		printf("Problem while opening file\n");
	else
		printf("Config value = %d\n", configVal);	

	//create processes
        createProcess(configVal);
	
	
	//free after all child process ends
	free(arrayPid);

	return 0;
}
//-------------------------------------------------------------
void handleSigHup(){
	printf("Sig hup signal received\n");

	//get new congi value from file
	int newConfig = openFile(filename);
	printf("New config = %d\n",  newConfig);
	
	//temp value used to store the differences
	int temp;
	
	//check if new value is bigger
	//thus we need to create more processes
	if (newConfig > configVal){
			
		temp = newConfig - configVal;

		//if value is more than max value
		//create how much process can be created
		if (newConfig > MAX){
			printf("Cannot create more than MAX of %d\n", MAX);
			temp = MAX - configVal;
			printf("Creating only %d\n", temp);
	
		}	
		printf("%d more processes are being created\n", temp);

		//create the processes
		createProcess((temp));
	}
	
	//if new config value is less
	//we need to kill some of them
	else if(configVal > newConfig){
		
		temp = configVal - newConfig;

		for (int i = 0; i< temp ; i++){
			kill(arrayPid[i], SIGINT);
			pidCount--;
		}
	}	
}
//-------------------------------------------------------------
//function to create number of processes
void createProcess(int number){
	//assert conditions
	assert(number >= 0);

	//local variable	
	pid_t pid;
	
	//loop to create processes
	for(int i = 0 ; i < number ; i++){

		//do not fork over the limit processes
		if (pidCount < MAX)
			pid = fork();

		//we are in child process if pid is 0
		if(pid == 0){
			printf("Child process %d created \n", getpid());		
			signal(SIGINT, handleSigInt);
			while(1){}

		}else{
			//in parent parent, we store the child pid in 
			//array
			if(pidCount < MAX)
				arrayPid[pidCount++] = pid;
			
		}
	}

	//wait in parent process	
	while( wait(NULL) != -1){}
}

//------------------------------------------------------------------
void handleSigInt(){
	printf("Process id %d terminated\n ", getpid());
	_exit(0);	
}
//-------------------------------------------------------------------
int openFile( char * filename){
	//assert conditons
	assert(filename != NULL);

	int number;

	//open file
	FILE * file = fopen(filename , "r");

	//check if file is not null first
	assert(file != NULL);

	//get config value in file
	fscanf(file, "%d", &number);			
	
	//close file after reading
	fclose(file);
	
	//successful
	return number;
}
