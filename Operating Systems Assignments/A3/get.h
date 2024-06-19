#include "fat32.h"

int compareString(char*, char*);
void get(char*, fat32BS*, char*);
void copyFile(char *,char*, struct DirInfo *, fat32BS *);
char* changeToLower(char *);
char* breakPathName(char*);
