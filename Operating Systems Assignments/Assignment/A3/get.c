#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "fat32.h"
#include "list.h"
#include "get.h"
#include "main.h"

//------------------------------------------------------------
//get the txt file from the image
void get(char *filename, fat32BS *header, char *textFile){
        
	//start looking in the root cluster
	int startingCluster = header->BPB_RootClus;

	//list does the search when it set to true
	list( startingCluster,0,  filename, header, true, textFile);
}
//------------------------------------------------------------
//compare if string 1 is equal to string 2
int compareString(char* str1, char* str2){
	//local variable		
	bool same = false;	

	//check if str1 equal str2
	if(strcmp(str1, str2) == 0){
		same = true;
	}
	return same;
}
//------------------------------------------------------------
//make the name into lower case
char* changeToLower(char *fileName){
	
	//local variables
	int length = 12;
	char charAti = ' ';
	//allocate memory
	char* lowerCase = (char*)malloc( length *sizeof(char));
	
	for(int i = 0; i < length; i++){
		//copy to memory first, then change to lower case
		charAti = fileName[i];
		lowerCase[i] = tolower(charAti);
	}
	return lowerCase;
}
//------------------------------------------------------------
//function to break the path of the name and get the txt file name
char* breakPathName(char* pathName){
	//break the path names
	int length  = 12;
	char*  filename = (char*)malloc( length * sizeof(char)) ;
	char *token = (char*) malloc (length * sizeof(char));
	
	token = strtok(pathName, "/");
	
	//get the filename from the path
	while(token != NULL){
		memcpy(filename, token, length);
		token = strtok(NULL, "/");
	}
	//releae memory
	free(token);
	return filename;	

}
//------------------------------------------------------------
//function to copy the file into current directory
void copyFile(char *filename,char *fileToGet ,struct DirInfo *header, fat32BS* fatHeader){
	
	//LOCAL VARIABLES
	//change the text file name to lower case first
	char* lowerCaseName = changeToLower(fileToGet);
	//set buffer size 
	int bufferSize = 64;
	//get where the data is in the cluster
	int dataCluster = header->dir_first_cluster_lo;
	int dataOffset = clusterToByte(dataCluster, fatHeader);

	//get the size of the content
	int size = header->dir_file_size;

	//our to file descriptors
	//1 to read and 1 to write into
	int fdWrite = open(lowerCaseName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
	int fdRead = openFile(filename);
	
	//saving the read bytes into the buffer and then
	//get that number of bytes to write into the other file
	char buffer[bufferSize];
	int noOfBytes;

	//skip to start of the data in the cluster	
	lseek(fdRead, dataOffset, SEEK_SET);

	//read 64 bytes ata time and then write into file simultaneously	
	for (int i = 0; i< size; i+= bufferSize){
		noOfBytes = read(fdRead, &buffer, bufferSize);
		write(fdWrite, &buffer, noOfBytes);
	}	

	//clean resources
	close(fdRead);
	close(fdWrite);
}
