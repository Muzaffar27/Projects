#include "fat32.h"
#include <stdbool.h>

int clusterToByte(int, fat32BS *);
void list( int,int, char*, fat32BS*, bool, char*);
void printSpace(int);
void longName(struct DirInfo *);
void trimName(char *, char*);
void addDotInName(char *);
uint32_t isClusterChained( uint32_t, char *, fat32BS * );
int isNameValid(char *);
int concatLongName(int,int,  char*, char*);
void storeLongName(int, int , char*);
void printLongName();
