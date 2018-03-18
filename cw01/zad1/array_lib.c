#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structure.h"
#include <time.h>

char ** createArray(int n){

    char ** t = (char **)calloc(n, sizeof(char *));
    return t;
}

void deleteArray(char **t, int n){

    int i = 0;
    for(i; i < n; i++){
        free(t[i]);
    }

    free(t);
}

char ** addToArray(char **t, int *n, int size){

    int i = 0;

    // losowanie danych do wstawienia
    char *str = (char *)calloc(size + 1, sizeof(char));
    for(i; i < size; i++)
        str[i] = rand()*1000%10+68;


    i = 0;
    while(i < *n  && t[i] != NULL)
        i++;
    
    if(i > *n - 1){
        char **safe = realloc(t, 2 * i * sizeof(char*));
        if(safe == NULL)
            return t;
        t = safe;
        *n = *n + 1;
    }

    t[i] = (char *)calloc(size + 1, sizeof(char));
    strcpy(t[i], str);

    return t;
}

char ** deleteFromArray(char **t, int *n, int size){
    
    if(0 > *n -1){
        printf("Nie ma elementu o podanym indeksie.");
        return t;
    }
    else{
        free(t[*n - 1]);
        char **safe = realloc(t, (*n-1)*size*sizeof(char));
        if(safe == NULL)
            return t;
        t = safe;
        *n = *n - 1;
        return t;
    }

}

char * findBlock(char **t, int n, int index, int size){
    
    char *block;
    int min = 0;
    int sum = 0;
    int tmpSum = 0;
    int i = 1;
    int j = 0;

    for(j; j < size; j++){
        sum += t[index][j];
    }

    j = 0;
    for(j; j < size; j++){
        tmpSum += t[0][j];
    }

    min = abs(sum - tmpSum);
    block = t[0];


    while(i < n || t[i] != NULL){
        tmpSum = 0;
        j = 0;
        
        if(i != index){
            for(j; j < size; j++){
                tmpSum += t[i][j];
            }

            if(abs(sum - tmpSum) < min){
                min = tmpSum;
                block = t[i];
            }
        }
        
        i++;
    }

    return block;
}

StaticTable createArrayS(int n){
    StaticTable t;
    int i, j;
    
    i = j = 0;
    for(i; i < n; i++){
        j = 0;
        for(j; j < n; j++)
            t.t[i][j] = ' ';
    }
    return t;
}

StaticTable fillInArrayS(StaticTable t, int n){
    
    int i, j, index, findEmptyRow;

    i = j = findEmptyRow = 0;
    while (findEmptyRow != 1 && i < n){
        if(t.t[i][j] != ' ' && j < n){
            i++;
            j = 0;
        }
        else if(j == n){
            index = i;
            findEmptyRow = 1;
        }     
        else{
            j++;
        }         
    }

    if(!findEmptyRow){
        printf("Nie można uzupełnić wiecej wierszy tablicy statycznej\n");
        return t;
    }
    else{
        j = 0;
        for(j; j < n; j++) t.t[index][j] = ((rand()*1000)%10)+65;
        return t;
    }
}

StaticTable deleteFromArrayS(StaticTable t, int n){
    
    int i, j, index, findBusyRow;

    i = j = findBusyRow = 0;
    while (findBusyRow != 1 && i < n){
        if(j < n && t.t[i][j] != ' '){
            index = i;
            findBusyRow = 1;
            
        }
        else if(j == n){
            i++;
            j = 0;
        }
        else
            j++;
    }

    if(!findBusyRow){
        printf("Nie można usunąć wiecej wierszy tablicy statycznej\n");
        return t;
    }
    else{
        j = 0;
        for(j; j < n; j++) t.t[index][j] = ' ';
        return t;
    }
}

int findBlockS(StaticTable t, int n, int index){
    
    int tmpSum, min, sum, indexM;
    tmpSum = sum = indexM = 0;
    min = 10000000;

    int i, j;
    i = j = 0;

    for(j; j < n; j++){
        sum += t.t[index][j];
    }

    i = 0;
    for(i; i < n; i++){
        j = 0;
        tmpSum = 0;

        if(i != index){
            for(j; j < n; j++)
                tmpSum += t.t[i][j];

            if(min > abs(sum - tmpSum)){
                min = tmpSum;
                indexM = i;
            }
        }
    }

    return indexM;
}