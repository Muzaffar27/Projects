//------------------------------------------------------------
// Name: Bhye Muzaffar Alii Rohomuthally
// Student Id : 7871794
// Cours: Comp 3430
// Intructor: Saulo Santos
// Assignment: Assignment 1, Question 1 
//------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
//-----------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

//defining struct elf header--------------------------------
typedef struct ELFHEADER{
	uint8_t format;
	uint8_t endian;
	uint8_t os;
	uint16_t fileType;
	uint16_t isa;
	uint64_t entryPoint;
	uint64_t startProgram;
	uint64_t startSection;
	uint16_t sizeProgram;
	uint16_t numProgram;		
	uint16_t sizeSection;	
	uint16_t numSection;	
	uint16_t sectionNames;	
}elfHeader;
//-----------------------------------------------------------
typedef struct PROGHEADER{
	uint16_t segType;
	uint16_t segType2;

	uint64_t fileOffset;

	uint64_t virAddress;
	uint64_t sizeFile;
}progHeader;
//-----------------------------------------------------------
typedef struct SECTHEADER{
	uint32_t offsetString;
	uint32_t type;
	uint64_t virAddress;
	uint64_t fileOffset;
	uint64_t sizeFile;
		
}sectHeader;
//-----------------------------------------------------------
#pragma pack(pop)

void readElf(elfHeader *, int);
void printElf(elfHeader *);

void readProgramHelper(progHeader *, elfHeader *, int);
void readProgram(progHeader *, int);
void printProgram(progHeader*, int,int);

void readSectionHelper(sectHeader *, elfHeader *, int);
void readSection(sectHeader *, int);
void printSection(sectHeader *,elfHeader*, int, int);
//------------------------------------------------------------
int main (int argc, char *argv[])
{
	//initializing of variables
	char *filename = "";	

	elfHeader elfHead;
	progHeader progHead;
	sectHeader sectHead;

	int elfFd, progFd, sectFd;

	//check and then open file
	if (argc > 1){
		filename = argv[1];
		printf("File %s is opened. \n", filename);
	}
	else
		printf("no file supplied\n");
	printf("\n");

	//opening file to read and print the datas of the elf header	
	elfFd = open(filename,O_RDONLY);
	readElf(&elfHead, elfFd);
	printElf(&elfHead);
	//close file after done reading
	close(elfFd);
	
	//open, read and print data of program header
	progFd = open(filename, O_RDONLY);
	readProgramHelper(&progHead, &elfHead, progFd);
	//close file after done reading
	close(progFd);
	
	//open,read, and print data of section header
	sectFd = open(filename, O_RDONLY);
	readSectionHelper(&sectHead, &elfHead, sectFd);
	//close file after done
	close(sectFd);

	printf("Program terminated successfully.\n");
	return 0;
}
//---------------------------------------------------------
//function to help read section header table(to avoid for loop in main)
void readSectionHelper(sectHeader *sectHead, elfHeader *elfHead, int fd){
	//assert conditions
	assert(sectHead != NULL);
	assert(elfHead!= NULL);
	assert(fd >= 0);
	
	int offset = 0; 
	
	//loop to print all section header
	for (int i = 0; i < elfHead -> numSection; i++){
		//find the start of the section header
		offset = elfHead->startSection + (elfHead->sizeSection*i);

		//skip to start of section header
		lseek(fd, offset, SEEK_SET);
	
		readSection(sectHead, fd);		
		printSection(sectHead,elfHead, i, fd);
	}
}
//-------------------------------------------------------------------
void readSection(sectHeader *header, int fd){
	assert(header!=NULL);
	assert(fd>=0);
	
	//read offset name of the current table
	read(fd, &header->offsetString, 4);

	//read type
	read(fd, &header->type, 4);

	//skip flag
	lseek(fd, 8, SEEK_CUR);

	//read vir address
	read(fd, &header->virAddress, 8);

	//read offset of file image
	read(fd, &header->fileOffset, 8);

	//read size in file
	read(fd, &header->sizeFile,8);
}
//---------------------------------------------------------
//function to help read program header table(to avoid for loop in main)
void readProgramHelper(progHeader *progHead ,elfHeader *elfHead, int fd){
	//assert conditions
	assert(progHead!= NULL);
	assert(elfHead!= NULL);
	assert(fd >= 0);

	//a count to get to each program header
	int offset = 0;
	
	//loop to print all program header
	for (int i =0; i< elfHead -> numProgram;i++){
		//the offset of where the prgram header start
		offset = elfHead-> startProgram + ( elfHead-> sizeProgram *i);
		//skip to program table offset
		lseek(fd, offset, SEEK_SET);
		
		readProgram(progHead, fd);
		printProgram(progHead, i, fd);
	}
}
//---------------------------------------------------------
void printSection(sectHeader *header, elfHeader *elfHead, int count , int fd){
	//assert conditions
	assert(header != NULL);
	assert(count >= 0);
	assert(fd >= 0);

	uint64_t data;
	
	//print section header count
	printf("  \nSection table %d:\n", count);
	
	//skip to start of section header
	lseek(fd, elfHead->startSection, SEEK_SET);
	
	//skip to specfic section names
	lseek(fd, (elfHead->sizeSection ) * (elfHead ->sectionNames), SEEK_CUR);
	lseek(fd, 24, SEEK_CUR);
	read(fd, &data, 8);
	lseek(fd, data + header->offsetString, SEEK_SET);

	//allocate memory for the name
	unsigned char *name = malloc(64);
	read(fd, name, 64);

	printf("\tSection name <<%s>>\n", name);
	
	//print type
	printf("\t*type 0x%02x\n", header->type);
	
	//print virtual address
	printf("\t*virtual address of section 0x%016lx\n", header->virAddress);

	//print size in file
	printf("\t*size in file %lu bytes\n", header->sizeFile);

	//printing first 32 bytes
	printf("\t*first up to 32 bytes starting at file offset %016lx:\n", header->fileOffset);

	//var to store value teporarily
	uint8_t temp;

	//skip to image offset in file
	lseek(fd, header->fileOffset, SEEK_SET);
	
	//loop to print 32 bytes
	for (int i = 0; i <32 ; i++){
		read(fd, &temp, 1 );
		//if used for making the indentation
		if(i == 16 || i == 0)
			printf("\n\t");

		printf("%02x ", temp);
	}
	printf("\n");

	//free the memory allocated
	free(name);

}
//---------------------------------------------------------
//function to print program headertable
void printProgram(progHeader *header, int count, int fd){
	//assert conditions 
	assert(header!= NULL);
	assert(count >= 0);
	assert(fd >= 0);

	//print header count
	printf("  \nHeader table %d:\n", count);
	//print segment
	printf("\t*segment type 0x%x%x\n", header->segType2,header->segType);
	//print virtual address
	printf("\t*virtual address of segment 0x%016lx\n", header->virAddress);
	//print size in file
	printf("\t*size in file %lu bytes\n", header->sizeFile);

	//printing first 32 bytes
	printf("\t*first up to 32 bytes starting at file offset %016lx:\n", header->fileOffset);
	
	//var to store value temporarily
	uint8_t temp;
	
	//skip to image offset in file
	lseek(fd, header->fileOffset, SEEK_SET);

	for (int i = 0; i <32 ; i++){
		read(fd, &temp, 1 );
		//if used for making the indentation
		if(i == 16 || i == 0)
			printf("\n\t");

		printf("%02x ", temp);
	}
	printf("\n");

}
//---------------------------------------------------------
//function to read program header table
void readProgram(progHeader *header, int fd){
	//assert conditions
	assert(header != NULL);
	assert(fd >= 0);

	//read seg type
	read(fd, &header->segType, 2);
	read(fd, &header->segType2, 2);
	
	//skip flg bytes 
	lseek(fd, 4, SEEK_CUR);

	//read offset
	read(fd, &header->fileOffset, 8);

	//read virtual address
	read(fd, &header->virAddress, 8);

	//skip 8 bytes 
	lseek(fd, 8 , SEEK_CUR);	

	//read 8 bytes fors size in bytes in file image
	read(fd, &header->sizeFile, 8);
}
//---------------------------------------------------------
//this function print the eflheader values
void printElf(elfHeader *header){
	assert(header != NULL);

	printf("Elf header:\n");
	
	//print format if 64-bit
	if (header->format == 2)	
		printf("\t*64-bit\n");
	else{
		printf("\tWrong format\n");
		return;
	}

	//print endian
	if (header-> endian == 1)
		printf("\t*little\n");
	else
		printf("\t*big\n");

	//print os 
	printf("\t*compiled for 0x%02x (operating system)\n", header->os);
	
	//print type
	printf("\t*has type 0x%02x\n", header->fileType);

	//print isa
	printf("\t*compiled for 0x%02x (isa)\n", header->isa);

	//print address point
	printf("\t*entry point address 0x%016lx \n", header->entryPoint);

	//print header table start 
	printf("\t*program header table starts at 0x%016lx \n", header->startProgram);

	//print header number and size
	printf("\t*There are %d program headers, each is %d bytes\n", header->numProgram, header-> sizeProgram);

	//print section number and size
	printf("\t*There are %d section headers, each is %d bytes\n", header->numSection, header->sizeSection);

	//print header string table
	printf("\t*The section header string table is %d \n", header-> sectionNames);
}
//---------------------------------------------------------
//this function stores the elfheaders value in the struct elfHeader
void readElf(elfHeader *header, int fd){
	//check first before continue
	assert(header != NULL);
	assert(fd > 0);

	//start reading and storing in struct elf header
	
	//skip magic numbers
	lseek(fd, 4 , SEEK_SET);

	//read format
	read(fd, &header->format, 1);	

	//read endian
	read(fd, &header->endian, 1);
	
	//skip 1 byte for version of ELF
	lseek(fd , 1 , SEEK_CUR);	
	
	//read target OS
	read(fd, &header->os, 1);

	//skip 8 bytes
	lseek(fd, 8, SEEK_CUR);

	//read file type
	read(fd, &header->fileType, 2);

	//read isa
	read(fd, &header->isa, 2);

	//skip 4
	lseek(fd, 4, SEEK_CUR);

	//read entry point
	read(fd, &header->entryPoint, 8);
	
	//read start program header
	read(fd, &header->startProgram, 8);

	//read start section header
	read(fd, &header->startSection,8);

	//skip 6 bytes
	lseek(fd, 6, SEEK_CUR);

	//read size of program header
	read(fd, &header->sizeProgram, 2);

	//read num of entries for program
	read(fd,&header->numProgram, 2);
	
	//read size of section header
	read(fd, &header->sizeSection, 2);

	//read num of entries for section
	read(fd, &header->numSection, 2);

	//read index start for section names
	read(fd, &header->sectionNames,2);
	
}
