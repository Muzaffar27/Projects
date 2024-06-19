#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "fillStructs.h"
#include "fat32.h"

//loading the struct for the boot sector
int fillFat32Struct(int fd, fat32BS *header){
	
	//assertions
	assert(fd > 0);
	assert(header != NULL);

	read(fd, &header->BS_jmpBoot, 3);

	//check jump boot allowed form
	//stop if not valid	
	if (header->BS_jmpBoot[0] != 0xEB){
		if (header->BS_jmpBoot[0] != 0xE9){
			printf("JumpBoot wrong\n");
			return -1;
		}
	}
 
	//check for the 3rd char if it is valid too
	if( header-> BS_jmpBoot[0] == 0xEB && header->BS_jmpBoot[2] != 0x90){
			printf("JumpBoot wrong\n");
			return -1;
	}	

	//read remaining into struct
	read(fd, &header->BS_OEMName , 8);
	
	read(fd, &header->BPB_BytesPerSec, 2);
	read(fd, &header->BPB_SecPerClus, 1);
	read(fd, &header->BPB_RsvdSecCnt, 2);
	read(fd, &header->BPB_NumFATs, 1);
	read(fd, &header->BPB_RootEntCnt, 2);
	read(fd, &header->BPB_TotSec16,2);
	read(fd, &header->BPB_Media,1);
	read(fd, &header->BPB_FATSz16,2);
	read(fd, &header->BPB_SecPerTrk, 2);
	read(fd, &header->BPB_NumHeads, 2);
	read(fd, &header->BPB_HiddSec,4);
	read(fd, &header->BPB_TotSec32, 4);

	read(fd, &header->BPB_FATSz32, 4);
	read(fd, &header->BPB_ExtFlags,2);
	read(fd, &header->BPB_FSVerLow,1);
	read(fd, &header->BPB_FSVerHigh,1);
	read(fd, &header->BPB_RootClus,4);
	read(fd, &header->BPB_FSInfo, 2);
	read(fd, &header->BPB_BkBootSec,2);
	read(fd, &header->BPB_reserved,12);	
	read(fd, &header->BS_DrvNum,1);
	read(fd, &header->BS_Reserved1,1);
	read(fd, &header->BS_BootSig,1);
	read(fd, &header->BS_VolID,4);
	read(fd, &header->BS_VolLab,11);
	read(fd, &header->BS_FilSysType,8);
	read(fd, &header->BS_CodeReserved,420);
	read(fd, &header->BS_SigA,1);
	read(fd, &header->BS_SigB,1);
	
	return 0;
}
//---------------------------------------------------------------
struct DirInfo fillDirInfo(int fd){
	struct DirInfo dirStruct;
	
	//store into structs DirInfo
	read(fd, &dirStruct.dir_name,11);
	read(fd, &dirStruct.dir_attr,1);
	read(fd, &dirStruct.dir_ntres,1);
	read(fd, &dirStruct.dir_crt_time_tenth,1);
	read(fd, &dirStruct.dir_crt_time,2);
	read(fd, &dirStruct.dir_crt_date,2);
	read(fd, &dirStruct.dir_last_access_time,2);
	read(fd, &dirStruct.dir_first_cluster_hi,2);
	read(fd, &dirStruct.dir_wrt_time,2);
	read(fd, &dirStruct.dir_wrt_date,2);
	read(fd, &dirStruct.dir_first_cluster_lo,2);
	read(fd, &dirStruct.dir_file_size,4);
	
	return dirStruct;
	
}
//---------------------------------------------------------------
struct DirLongInfo fillLongDir(int fd){
	struct DirLongInfo header;	

	//fill struct
        read(fd, &header.ldir_ord,1);
        read(fd, &header.ldir_name1,10);
        read(fd, &header.ldir_attr,1);
        read(fd, &header.ldir_type,1);
        read(fd, &header.ldir_chksum,1);
        read(fd, &header.ldir_name2,12);
        read(fd, &header.ldir_fstClusLo,2);
        read(fd, &header.ldir_name3,4);
	
	return header;
}
