#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
    
    printf("Start Hello World\n");
    
    
    // while(1){
    //     printf("Hello World\n");
    // } 

    char *veryLargeTab = (char *)calloc(100000000, sizeof(char));
    for(int i=0; i<100000000; i++) {
        veryLargeTab[i] = 'a';
        //printf("%c", veryLargeTab[i]);
    }
    return 0;
}