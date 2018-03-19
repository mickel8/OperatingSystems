#include <stdio.h>
#include <dirent.h>

int main(int argc, char **argv){

    char *dirname = argv[1];
    DIR *dirp;
    if(dirp = opendir(dirname) == NULL){
        printf("Nie udało się otworzyć katalogu");
        return 0;
    }


    struct dirent *plik = readdir(dirp);

    struct stat *buf;
    stat(plik.d_name, )



    if(closedir(dirp) == 0) printf("Poprawnie zamknieto strumien");
    else printf("Nie udało sie zamknąć strumienia");



}