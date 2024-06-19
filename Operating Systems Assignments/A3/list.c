#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"
#include "fat32.h"
#include "fillStructs.h"
#include "main.h"
#include "get.h"
#include <fcntl.h>

#define DIRSIZE 32
#define LONGSIZE 20

//GLOBAL VARIABLE
char* longNameData[LONGSIZE];
int longNameAttr[LONGSIZE];
int longNameCount = 0;


//the get var:
//false for list
//true for get 
//function for the list command
void list(int cluster ,int space, char *filename, fat32BS *header, bool isGet, char* toGet){

	//LOCAL VARIABLES
  	//open file to read data
	int fd = openFile(filename);
	//convert the cluster number to bytes
  	int offset = clusterToByte(cluster, header);
	//get how many directors are there per sectors
  	int dirPerSec = header->BPB_BytesPerSec/32;
	//variables used to clean(trim, add dot etc) names received from fat32 image
 	char name[12], tempName[12];
	//to store directories data	
	struct DirInfo currDir; 
	int dirClus;
	char *loweredName = ""; 	
	char *loweredToFind = "";
	//skip to cluster offset to start reading
  	lseek(fd, offset, SEEK_SET);

	//loop for each sector in a cluster
	for (int i = 0; i< header->BPB_SecPerClus;i++){
		//loop for each directory in a sector
		for (int j =0;j<dirPerSec;j++){
        	
			//read directory
			currDir = fillDirInfo(fd);
	
			//copy the name
			//and add null byte
        		memcpy(name, currDir.dir_name,11);
        		name[11]= '\0';

			//check if long name and process accordingly
			longName(&currDir);

			//check if name is valid to process
			if ( isNameValid(name)){

				//check for directory
				if (((uint32_t) currDir.dir_attr == ATTR_DIRECTORY)){
                    			if (!isGet){
						printSpace(space++);
						printf("%s\n", name);
					}
					//check if entry is in use/valid or not
                    			if ( (currDir.dir_first_cluster_hi <= 0)){
						//get the cluster for the contents of the directory
                       				dirClus = (currDir.dir_first_cluster_hi << 16) + currDir.dir_first_cluster_lo;
						//recursively call list again
						list( dirClus,space, filename, header, isGet, toGet);
             				}
				}

				//check if it is regular file
				//make sure it is not a hidden file
            			if (((uint32_t)currDir.dir_attr & ATTR_HIDDEN) != ATTR_HIDDEN && currDir.dir_attr == 0x20){
                   			//remove white spaces
					trimName(name,tempName);
					//add dot between the name and the extension in the filename
                    			addDotInName(tempName);
                       			
					printSpace(space);
					if (!isGet)
						printf("Short Name = %s\n",tempName);
					else {
						//change both to lower case to easily manage them	
						loweredName = changeToLower(tempName);			
						toGet = breakPathName(toGet);
						loweredToFind = changeToLower(toGet);
						//copy if names are equal	
						if (compareString(loweredName, loweredToFind)){
							printf("File found in system\n");
							printf("Copying file to current directory........\n");
							copyFile(filename ,loweredToFind , &currDir, header);
							printf("File successfully copied\n");
						}
					}

					//check for long name and print
					if (longNameCount > 0 && !isGet)
						printLongName();
             			}

            		}
     		}
 	}

	//after reading the cluster fully
	//check if cluster if chained or not
	//if it is we continue to read
	cluster =  isClusterChained(cluster, filename, header);
  	if (cluster >= 0x00000002 && cluster <= 0x0FFFFFEF ){
      		list( cluster,space, filename, header, isGet, toGet);
  	}
}
//------------------------------------------------------------------------------
//print the long name
void printLongName(){
	
	int i = 0;

	//check when we have only 1 long name stored
	//if valid we print if not we do nothing
	if (longNameCount == 1 && (longNameAttr[0] - (int) MASKVALUE) == longNameCount){
		printf("\t\tLongName = %s\n", longNameData[longNameCount-1]);
	}

	
	//if we have more than 1, we print until it 
	//is continuous
	if (longNameCount > 1){
		//if valid long name
		if (longNameAttr[longNameCount-1] == 1){
			//we start from the end of the array
			//the long name is stored reversely
			i = longNameCount - 1;
			printf("\t\tLongName = ");
			
			//while we do not fine the end of the long name
			//which is masked with 0x40, we print
			while(i >=0  && (longNameAttr[i] & MASKVALUE) != MASKVALUE){
				printf("%s", longNameData[i]);
				i--;
			}
			
			//print the end name, which was the stopping condition in the while
			//loop above
			if (i >= 0)
				printf("%s", longNameData[i]);
			printf("\n");
		}
	}	
	
	//clean both arrays
	for (int j = 0; j< longNameCount ;j ++){
		longNameAttr[j] = 0;
		longNameData[j] ="";
	}
	longNameCount = 0;
}
//------------------------------------------------------------------------------
//check if we are able to process the 
//structure or not
int isNameValid(char *name){
	int flag = 0;
	
	if ( (uint32_t) name[0] != E5){
		if ( (uint8_t) name[0] != 0){
			if (name[0] != '.' && name[1] != '.'){
				flag =1;
			}
		}
	}
	
	return flag;
}
//------------------------------------------------------------------------------
//function to process the long names
void longName(struct DirInfo *header){
	
	//Local Variables
	char *temp = (char*)malloc(256*sizeof(char));
	struct DirLongInfo longDir;
	int count = 0;
	int sizeName1 = 10;
	int sizeName2 = 12;
	int sizeName3 = 4;
	int totalLength = sizeName1 + sizeName2 + sizeName3;

	//check if we are reading a directory 
	if (header->dir_attr == 0x0F){
        	
		//convert to long directory structure
		longDir = *(struct DirLongInfo*) header;

		//check if directory is valid
	        if(longDir.ldir_ord != 0xE5){

			//concatenate all three names into 1
              		if ((char) longDir.ldir_name1[0] != '.'){
				count = concatLongName(count , sizeName1, longDir.ldir_name1, temp );
				count = concatLongName(count , sizeName2, longDir.ldir_name2, temp );
				count = concatLongName(count , sizeName3, longDir.ldir_name3, temp );
           		 	//printf("Long Name = %s = ox%X\n", temp, longDir.ldir_ord);
         		
				//store the long name to process later when short name is found	
				storeLongName(totalLength , longDir.ldir_ord, temp);		
			}
                }
	}

	//free resources
	free(temp);
}

//------------------------------------------------------------------------------
//store the long name in the data base to use later
void storeLongName(int length , int attr , char* name){
	
	longNameData[longNameCount] = (char*)malloc(length*sizeof(char));
        memcpy(longNameData[longNameCount], name, length);
	longNameAttr[longNameCount] = attr;
	longNameCount++;

}

//------------------------------------------------------------------------------
int  concatLongName(int count, int size, char *name , char *fullName){
	
	//step value
	int incVal = 2;
	//the position we arrived in the fullname 
	int position = count;

	for (int i = 0;i < size ; i+=incVal){
		
		//check if we found the null byte terminator
		//else we just copy the char at that position
		if ( (char) name[i] == '\0')
			fullName[position++] = '\0';
		else
			fullName[position++] = (char) name[i];
	}	

	//return the position value we ended at
	return position;
}
//------------------------------------------------------------------------------
//remove white spaces in the input
//and return the resulted char as output
void trimName(char *input, char *output){
   
	//local var
	int count = 0;

	//loop for each char in the input string
	for (int i = 0;i < (int) strlen(input);i++){

		//if the char is not a white space
		//we copy it to the output string
      		if(input[i] != ' ')
        		output[count++] = input[i];
	}

	//add null terminator at the end of the output string
   	output[count] = '\0';
}

//add '.' before the last 3 char 
//-----------------------------------------------------------------------------------
void addDotInName(char *input){

	//local var
	char temp = '.';
	char temp2;
  
	//extension is the last 3 char
	//we add a dot before these 3 dots 
	//and move the rest of the char 1 place ahead 
	for(int i = strlen(input) - 3; i < (int) strlen(input) + 1; i++){
		temp2 = input[i];
        	input[i] = temp;
        	temp = temp2;
   	}
}

//-----------------------------------------------------------------------------------
//check if the cluster is chained or not
uint32_t isClusterChained(uint32_t cluster , char *filename, fat32BS *header){

	uint64_t offsetToFAT = header->BPB_RsvdSecCnt * header->BPB_BytesPerSec + cluster * 4;
	uint32_t clusterVal;
	
	int fd = openFile(filename);
	
	lseek(fd, offsetToFAT,SEEK_SET);
	
	//read value to know if cluseter is chained or not
	read(fd, &clusterVal, 4);

  	return clusterVal;
}
//----------------------------------------------------------------
//convert the cluster value into bytes
//to be able to jump to that cluster and read data
int clusterToByte(int clusterVal, fat32BS *header){
	
	int bytesPerSec = header->BPB_BytesPerSec;
	int sizeOfFATs = header->BPB_NumFATs * header->BPB_FATSz32 * bytesPerSec;
	int clusterInBytes = (clusterVal - 2) * header->BPB_SecPerClus * bytesPerSec;
	int reservedBytes = header->BPB_RsvdSecCnt * bytesPerSec;

 	int offsetVal = clusterInBytes + reservedBytes + sizeOfFATs;

  	return offsetVal;
}
//-------------------------------------------------------------------------------------------
void printSpace(int n){
	for(int i=0;i< n;i++){
		printf("  ");
	}
}
