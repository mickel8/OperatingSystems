#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#define _XOPEN_SOURCE
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>
#include <limits.h>
#include <sys/types.h>

// variables for file structs
static struct dirent *file;
static struct stat stats;

void print_info(char *fullPath, time_t *modTime, char *modTimeStr){
    strftime(modTimeStr, 20, "%Y-%m-%d %H:%M:%S", localtime(modTime)); // error
    printf("%s %ld %s ", fullPath, stats.st_size, modTimeStr);
    printf( (stats.st_mode & S_IRUSR) ? "r" : "-");
    printf( (stats.st_mode & S_IWUSR) ? "w" : "-");
    printf( (stats.st_mode & S_IXUSR) ? "x" : "-");
    printf( (stats.st_mode & S_IRGRP) ? "r" : "-");
    printf( (stats.st_mode & S_IWGRP) ? "w" : "-");
    printf( (stats.st_mode & S_IXGRP) ? "x" : "-");
    printf( (stats.st_mode & S_IROTH) ? "r" : "-");
    printf( (stats.st_mode & S_IWOTH) ? "w" : "-");
    printf( (stats.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
}

char *makePath(char *dirname){
    char *path = calloc(strlen(dirname) + strlen(file -> d_name) + 1, sizeof(char));
    strcpy(path, dirname); strcat(path, "/"); strcat(path, file -> d_name);

    return path;
}

void search_lower(time_t dateTime, DIR *dirp, char *dirname){
    
    char *modTimeStr = calloc(20, sizeof(char));

    while((file = readdir(dirp)) != NULL && !errno){
        
        // ustalanie ścieżki pliku
        char *path = makePath(dirname);
        if(lstat(path, &stats) == -1){
            perror("search_lower() - error during examining file stats");
            free(path);
            break;
        }

        char *fullPath = realpath(path, NULL);
        if(fullPath == NULL){
            perror("search_lower() - couldn't resolve path");
            free(path);
            break;
        }
    
        time_t modTime = stats.st_mtime;
        if(modTime < dateTime && S_ISREG(stats.st_mode) && !S_ISLNK(stats.st_mode))
            print_info(fullPath, &modTime, modTimeStr);

        free(path);
        free(fullPath);
    }
    if(errno){
        perror("examine_dir_v1() - error during examining directory");
    }

    free(modTimeStr);
    
    return;    
}

void search_greater(time_t dateTime, DIR *dirp, char *dirname){
    
    char *modTimeStr = calloc(20, sizeof(char));

    while((file = readdir(dirp)) != NULL && !errno){
        
        // ustalanie ścieżki pliku
        char *path = makePath(dirname);
        if(lstat(path, &stats) == -1){
            perror("search_greater() - error during examining file stats");
            free(path);
            break;
        }

        char *fullPath = realpath(path, NULL);
        if(fullPath == NULL){
            perror("search_greater() - couldn't resolve path");
            free(path);
            break;
        }
    
        time_t modTime = stats.st_mtime;
        if(modTime > dateTime && S_ISREG(stats.st_mode) && !S_ISLNK(stats.st_mode))
            print_info(fullPath, &modTime, modTimeStr);

        free(path);
        free(fullPath);
    }
    if(errno){
        perror("examine_dir_v1() - error during examining directory");
    }

    free(modTimeStr);
    
    return;    
}

void search_equal(time_t dateTime, DIR *dirp, char *dirname){
    
    char *modTimeStr = calloc(20, sizeof(char));

    while((file = readdir(dirp)) != NULL && !errno){
        
        // ustalanie ścieżki pliku
        char *path = makePath(dirname);
        if(lstat(path, &stats) == -1){
            perror("examine_dir_v1() - error during examining file stats");
            free(path);
            break;
        }

        char *fullPath = realpath(path, NULL);
        if(fullPath == NULL){
            perror("search_equal() - couldn't resolve path");
            free(path);
            break;
        }

        time_t modTime = stats.st_mtime; 
        if(modTime < (dateTime + 86400) && modTime > (dateTime - 86400) && S_ISREG(stats.st_mode) 
        && !S_ISLNK(stats.st_mode)) // due to the range of whole day. One day has 86400 seconds
            print_info(fullPath, &modTime, modTimeStr);
    
        free(path);
        free(fullPath);
    }
    if(errno){
        perror("examine_dir_v1() - error during examining directory");
    }

    free(modTimeStr);
    
    return;    
}

void examine_dir_v1(int argc, char **argv){
    
    char *dirname = argv[1];
    char *operator = argv[2];
    char *date = argv[3];
    // initialization of tm struct fileds with 0 in other case there will be wrong conversion 
    struct tm tmDF;
    tmDF
    .tm_sec = tmDF
    .tm_min = tmDF
    .tm_hour = tmDF
    .tm_mday = tmDF
    .tm_mon = tmDF
    .tm_year = tmDF
    .tm_wday = tmDF
    .tm_yday = tmDF
    .tm_isdst = 0;
    
    // conversion of date from cmd to time_t format
    char *resFrmStrDt = strptime(date, "%Y-%m-%d", &tmDF); // result of formating string date
    if(resFrmStrDt == NULL){
        perror("examine_dir_v1() - error during formating string date");  
        return;                                 
    }
    if(*resFrmStrDt != '\0'){
        printf("examine_dir_v1() - wrong date format was given. ISO standard is YYYY-mm-dd\n");
        return;
    }

    time_t dateTime = mktime(&tmDF);
    if(dateTime == -1){
        perror("examine_dir_v1() - couldn't format date tm to time_t");
        return;
    }

    DIR *dirp = opendir(dirname);
    if(!dirp){
        perror("Couldn't open stream to given directory");
        return;
    }

    if(strcmp(operator, "<") == 0)
        search_lower(dateTime, dirp, dirname);
    else if(strcmp(operator, ">") == 0)
        search_greater(dateTime, dirp, dirname);
    else
        search_equal(dateTime, dirp, dirname);


    int closeFlag = closedir(dirp);
    if(closeFlag == -1){
        perror("Couldn't close stream to directory");
    }

}

void examine_dir_v2(int argc, char **argv){

}


int main(int argc, char **argv){

    if(argc != 5){ // ? nazwa programu
        printf("Wrong number of arguments\n");
        return 0;
    }
    if(strcmp(argv[2], "<") != 0 && strcmp(argv[2], ">") != 0 && strcmp(argv[2], "=") != 0){
        printf("Wrong second argument. It must be < > or =\n");
        return 0;
    }
    else if(strcmp(argv[4], "v1") == 0) examine_dir_v1(argc, argv);
    else if(strcmp(argv[4], "v2") == 0) examine_dir_v2(argc, argv);
    else printf("Wrong version of program\n");

    return 0;
}