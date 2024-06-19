#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "info.h"
#include "fat32.h"
#define LENGTH 11

void printInfo(int fd, fat32BS *header){
	assert( header!=NULL);

	//local variables for ease of usage
	uint32_t reservedSectors = header->BPB_RsvdSecCnt;	
	uint32_t bytesPerSec = header->BPB_BytesPerSec;
	uint32_t secPerClus = header->BPB_SecPerClus;

	//used to lseek to FAT to find free space
	uint32_t jumpForFat = reservedSectors * bytesPerSec;

	//get the total space FATs takes
	uint32_t totalFats = header->BPB_FATSz32 * header->BPB_NumFATs;

	//directory sector	
	uint32_t dirSec = ( (header->BPB_RootEntCnt *32)  + (bytesPerSec-1)) / bytesPerSec;	
	//total sector for data
	uint32_t sectorData = header->BPB_TotSec32 - (reservedSectors + totalFats + dirSec);
	
	uint32_t totalSpace = (header->BPB_TotSec32 * bytesPerSec) / 1024;
	//usable space
	uint32_t usableSpace = (sectorData/secPerClus * bytesPerSec) / 1024;
	
	//get free space
	uint32_t clustersInFAT = sectorData / secPerClus;
	int noFreeClusters = getFreeCluster(fd, jumpForFat, clustersInFAT);	
	//free space
	uint32_t freeSpace = noFreeClusters * secPerClus * bytesPerSec / 1024;		
	printf("----------Info Command-----------\n\n");

	printf("OEMName = %s \n", header->BS_OEMName);	

	printf("Volume Label = ");		
	for(int i =0; i < LENGTH;i++)
		printf("%c", header->BS_VolLab[i]);

	printf("\n");
	printf("Total space =  %d Kb\n", totalSpace);
	printf("Usable space = %d Kb\n", usableSpace);
	printf("Free space =   %d Kb\n",  freeSpace);
	printf("\n-----------End of Info-----------\n");
}
//------------------------------------------------------------------------------------------
int getFreeCluster(int fd, int jumpBytes , int numberOfClus){

	//local variables
	uint32_t fatSig0, fatSig1;
	uint32_t bitmask = 0x0FFFFFFF;
	uint32_t temp;
	int free = 0;

	//jump to FAT
	lseek(fd , jumpBytes, SEEK_SET);

	//assert the first 2 signatures
	//Signature 0
	read(fd, &fatSig0, 4);
	assert(fatSig0 == 0x0FFFFFF8);

	//Signature 1
	read(fd, &fatSig1, 4);
  	assert((fatSig1 & bitmask) == 0x0FFFFFFF);	

	//check for free clusters	
	for (int i =0; i< numberOfClus;i++){
		read(fd, &temp, 4);
		if(temp == 0)
			free++;
	}



	return free;
}

