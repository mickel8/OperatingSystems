#ifndef array_lib
#define array_lib

#include <stdio.h>

typedef struct StaticTable{
    char t[1000][1001];
}StaticTable;


char ** createArray(int);

void deleteArray(char **, int );

char ** addToArray(char **, int *, int);

char ** deleteFromArray(char **, int *, int);

char * findBlock(char **, int, int, int);

StaticTable createArrayS(int);

StaticTable fillInArrayS(StaticTable, int);

StaticTable deleteFromArrayS(StaticTable, int);

int findBlockS(StaticTable, int, int);

#endif