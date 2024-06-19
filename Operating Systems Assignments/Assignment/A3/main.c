//     NAME : 	Bhye Muzaffar Alii Rohomuthally
//Student ID:   7871794	
//   COURSE :   Comp 3430
//Instructor:   Saulo Santos
//Assignment:   Assignment 3
//-------------------------------------------------------------
//Remarks: This program read an image and is able to do 3 tasks
//         depending on the user input:
//	   1: It prints the info of an image
//         2: It lists the files and directories of an image
//         3: It fetches a file from the image if it exists
//-------------------------------------------------------------

//libraries
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "fillStructs.h"
#include "info.h"
#include "list.h"
#include "main.h"
#include "get.h"

//---------------------------------------------------
//main function
int main(int argc, char *argv[]){
	assert(argv != NULL);

	printf("Program Starting\n");
	
	//local variables
	fat32BS struct32;
	char* filename = argv[1];
	//open file
	int fd = 0;
	char *fileToGet = "";

	fd = openFile(filename);
	if (fd == -1){
		printf("Error on opening file. Try Again\n");
		return EXIT_FAILURE;
	}
	if (argc == 4){
		fileToGet = argv[3];
	}

	//load fat32 struct first ie the boot sector
	fillFat32Struct(fd, &struct32);

	//do neccessary action based on command	
	useCommand(argv[2],filename,fd,  &struct32, fileToGet);	

	printf("Closing file\n");

	close(fd);
	
	printf("Program Terminating\n");
	return EXIT_SUCCESS;
}
//----------------------------------------------------
//functions to open file 
int openFile(char *filename){
	//assert conditions
	assert(filename != NULL);

	int fd = open(filename , O_RDONLY);

	//check if successfully opened
	if (fd < 0){
		fd = -1;
	}	
	
	return fd;

}
//function to manage which command is passed as argument
//and do the necessary actions
//----------------------------------------------------
void useCommand(char *command ,char *filename , int fd, fat32BS *header, char* fileToGet){
	
	assert(command != NULL);
	//compare command and do actions
	//based on the command
	
	int startingCluster = header->BPB_RootClus;

	if (strcmp(command, "info") == 0){
		printInfo(fd, header);
	}
	else if (strcmp(command, "list") == 0){
		printf("Listing the contents of the image:\n");
		list( startingCluster,0,  filename, header, false,"");
	}
	else if (strcmp(command, "get") == 0){
		printf("Searching for file %s in image\n", fileToGet);
		printf("Note: If not successful message is printed means FILE NOT IN IMAGE \n");
		get(filename , header, fileToGet);
	}
	else {
		printf("Wrong command entered.\n");
	}
}
//----------------------------------------------------
