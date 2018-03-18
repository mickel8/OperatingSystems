#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include "structure.h"
#include <dlfcn.h>

#define COLOR   "\x1b[31m"
#define RESET   "\x1b[0m"

// zmienne zegarowe
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

void printTimeInformation(double measurements[], int n){
   
    printf("Real time: " COLOR "%f, " RESET
     "user time: " COLOR "%f, " RESET
     "system time: " COLOR "%f\n" RESET, measurements[0], measurements[1], measurements[2]);
}

void writeToFileTimeInformation(double measurements[], int n, int operation){

    char *name = "raport3b.txt";
    char *title;

    char *realTime = "Real time: ";
    char *userTime = ", user time: ";
    char *sysTime  = ", system time: ";

    FILE *plik = fopen(name, "a+");

    switch(operation){
        case 1:
            title = "* Tworzenie tablicy *\n";
            break;
        case 2:
            title = "\n* Dodawanie kolejnego wiersza tablicy x razy * \n";
            break;
        case 3:
            title = "\n* Usuwanie kolejnego wiersza tablicy x razy * \n";
            break;
        case 4:
            title = "\n* Znajdowanie bloku *\n";
            break;
        case 5:
            title = "\n* Wstawianie i usuwanie x razy * \n";
            break;
        default:
            break;

    }

    fwrite(title,    sizeof(char), strlen(title),    plik);
    fwrite(realTime, sizeof(char), strlen(realTime), plik);
    fprintf(plik, "%f", measurements[0]);
    fwrite(userTime, sizeof(char), strlen(userTime), plik);
    fprintf(plik, "%f", measurements[1]);
    fwrite(sysTime,  sizeof(char), strlen(sysTime),  plik);
    fprintf(plik, "%f", measurements[2]);

    fclose(plik);

}

void st_clock(){
    st_time = times(&st_cpu);
}

void en_clock(int operation){
    static long int clk = 0;
    clk = sysconf(_SC_CLK_TCK);
    en_time = times(&en_cpu);

    int n = 3;
    double measurements[n];
    measurements[0] = (en_time - st_time) / (double)clk;
    measurements[1] = (en_cpu.tms_utime - st_cpu.tms_utime) / (double)clk;
    measurements[2] = (en_cpu.tms_stime - st_cpu.tms_stime) / (double)clk;

    printTimeInformation(measurements, n);
    writeToFileTimeInformation(measurements, n, operation);

}

void printStaticArray(StaticTable t, int n){
    
    int j = 0;
    int i = 0;
    for(i; i < n; i++){
        j = 0;
        for(j; j < n; j++)
            printf("%c", t.t[i][j]);
        printf("\n");
    }
}

void printDynamicArray(char **t, int n, int size){

    int i = 0;
    for(i; i < n; i++)
        printf("%d %s\n", i, t[i]);
}

void operationForStaticArray(int argc, char **argv, int n){
    
    #ifdef DLL

    void *handle = dlopen("./libarray_lib.so", RTLD_LAZY);
    if(!handle){
        printf("Nie udało się załadować biblioteki.\n");
        
        #ifdef DDL
        dlclose(handle);
        #endif

        return;
    }
    typedef struct StaticTable StaticTable;

    char ** (*createArray)(int);
    createArray = dlsym(handle, "createArray");

    void (*deleteArray)(char **, int );
    deleteArray = dlsym(handle, "deleteArray");

    char ** (*addToArray)(char **, int *, int);
    addToArray = dlsym(handle, "addToArray");

    char ** (*deleteFromArray)(char **, int *, int);
    deleteFromArray = dlsym(handle, "deleteFromArray");

    char * (*findBlock)(char **, int, int, int);
    findBlock = dlsym(handle, "findBlock");

    StaticTable (*createArrayS)(int);
    createArrayS = dlsym(handle, "createArrayS");

    StaticTable (*fillInArrayS)(StaticTable, int);
    fillInArrayS = dlsym(handle, "fillInArrayS");

    StaticTable (*deleteFromArrayS)(StaticTable, int);
    deleteFromArrayS = dlsym(handle, "deleteFromArrayS");

    int (*findBlockS)(StaticTable, int, int);
    findBlockS = dlsym(handle, "findBlockS");

    #endif

    printf("\n* Tworzenie tablicy *\n");
    st_clock();
    StaticTable t = createArrayS(n);
    en_clock(1);

    int j = 0;
    int i = 4;
    for(i; i < argc; i += 2){
        j = 0;
        if(strcmp(argv[i], "fill") == 0){
            
            printf("* Uzupełnianie kolejnego wiersza tablicy %d razy * \n", atoi(argv[i+1]));
            
            st_clock();
            for(j; j < atoi(argv[i+1]); j++)
                t = fillInArrayS(t, n);
            en_clock(2);
        }
        if(strcmp(argv[i], "delete") == 0){
            
            printf("* Usuwanie kolejnego wiersza tablicy %d razy * \n", atoi(argv[i+1]));
            
            st_clock();
            for(j; j < atoi(argv[i+1]); j++)
                t = deleteFromArrayS(t, n);
            en_clock(3);
        }
        if(strcmp(argv[i], "find") == 0){
            
            printf("* Znajdowanie bloku *\n");
            
            st_clock();
            int x = findBlockS(t, n, atoi(argv[i+1]));
            en_clock(4);           
        }
        if(strcmp(argv[i], "fill_and_delete") == 0){
            
            printf("* Wstawianie i usuwanie %d razy * \n", atoi(argv[i+1]));
            
            st_clock();
            for(j; j < atoi(argv[i+1]); j++){
                t = fillInArrayS(t, n);
                t = deleteFromArrayS(t, n);
            }
            en_clock(5);
        }
    }

    #ifdef DLL
    dlclose(handle);
    #endif
}

void operationForDynamicArray(int argc, char **argv, int n, int size){
    
    #ifdef DLL

    void *handle = dlopen("./libarray_lib.so", RTLD_LAZY);
    if(!handle){
        printf("Nie udało się załadować biblioteki.\n");
        
        #ifdef DDL
        dlclose(handle);
        #endif

        return;
    }
    typedef struct StaticTable StaticTable;

    char ** (*createArray)(int);
    createArray = dlsym(handle, "createArray");

    void (*deleteArray)(char **, int );
    deleteArray = dlsym(handle, "deleteArray");

    char ** (*addToArray)(char **, int *, int);
    addToArray = dlsym(handle, "addToArray");

    char ** (*deleteFromArray)(char **, int *, int);
    deleteFromArray = dlsym(handle, "deleteFromArray");

    char * (*findBlock)(char **, int, int, int);
    findBlock = dlsym(handle, "findBlock");

    StaticTable (*createArrayS)(int);
    createArrayS = dlsym(handle, "createArrayS");

    StaticTable (*fillInArrayS)(StaticTable, int);
    fillInArrayS = dlsym(handle, "fillInArrayS");

    StaticTable (*deleteFromArrayS)(StaticTable, int);
    deleteFromArrayS = dlsym(handle, "deleteFromArrayS");

    int (*findBlockS)(StaticTable, int, int);
    findBlockS = dlsym(handle, "findBlockS");

    #endif


    printf("\n* Tworzenie tablicy *\n");
    st_clock();
    char **t = createArray(n); 
    en_clock(1);

    int j = 0;
    int i = 4;
    for(i; i < argc; i += 2){
        j = 0;
        if(strcmp(argv[i], "add") == 0){
            
            printf("* Dodawanie kolejnego wiersza tablicy %d razy * \n", atoi(argv[i+1]));
            st_clock();
            for(j; j < atoi(argv[i+1]); j++) 
                t = addToArray(t, &n, size);
            en_clock(2);
        }
        if(strcmp(argv[i], "delete") == 0){

            printf("* Usuwanie kolejnego wiersza tablicy %d razy * \n", atoi(argv[i+1]));
            st_clock();
            for(j; j < atoi(argv[i+1]); j++)
               t = deleteFromArray(t, &n, size);
            en_clock(3);
        }

        if(strcmp(argv[i], "find") == 0){

            printf("* Znajdowanie bloku *\n");
            st_clock();
            char *block = findBlock(t, n, atoi(argv[i+1]), size);
            en_clock(4);
             
        }
        if(strcmp(argv[i], "fill_and_delete") == 0){

            printf("* Wstawianie i usuwanie %d razy * \n", atoi(argv[i+1]));
            st_clock();
            for(j; j < atoi(argv[i+1]); j++){
                t = addToArray(t, &n, size);
                t = deleteFromArray(t, &n, size);
            }
            en_clock(5);
        }

    }
    
    deleteArray(t, n);

    #ifdef DLL
    dlclose(handle);
    #endif

}

int main(int argc, char **argv){

    srand(time(NULL));

    int n = atoi(argv[1]);
    int size = atoi(argv[2]);
    char *allocation = argv[3];

    if(strcmp(allocation, "static") == 0)
        operationForStaticArray(argc, argv, n);
    else if (strcmp(allocation, "dynamic") == 0)
        operationForDynamicArray(argc, argv, n, size);
    else
        printf("Jako trzeci argument podaj typ alokacji - dynamic lub static\n");
    
    return 0;


}