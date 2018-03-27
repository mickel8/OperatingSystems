#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#define _GNU_SOURCE
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ftw.h>
#include <stdint.h>
#include <unistd.h>
#define COLOR  "\x1B[36m"
#define RESET "\x1B[0m"

// variables for file structs
static struct dirent *file;
static struct stat stats;

static time_t dateTime; // for time_t representation of given date in cmd
static char* operator;


void examine_dir_v1(char *); 

void print_info(char *fullPath, time_t *modTime){
    
    char *modTimeStr = calloc(20, sizeof(char));
    int res = strftime(modTimeStr, 20, "%Y-%m-%d %H:%M:%S", localtime(modTime));
    if(res == 0 && errno){
        perror("printf_info() - error during conversion mod time to string");
    }   
    
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
    
    free(modTimeStr);
}

void prepare_data_and_print(const char *fpath, time_t *modTime){
   
    char *fullPath = realpath(fpath, NULL);
    if(fullPath == NULL){
        perror("print_info_v2() - couldn't resolve path");
        return;
    }

    print_info(fullPath, modTime);

    free(fullPath);
}

void date_time(char *date){
    
    // initialization of tm struct fileds with 0, in other case there will be wrong conversion 
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
    
    char *resFrmStrDt = strptime(date, "%Y-%m-%d", &tmDF); // result of formating string date
    if(resFrmStrDt == NULL){
        perror("date_time() - error during formating string date");  
        return;                                 
    }
    if(*resFrmStrDt != '\0'){
        printf("date_time() - wrong date format was given. ISO standard is YYYY-mm-dd\n");
        return;
    }

    dateTime = mktime(&tmDF);
    if(dateTime == -1){
        perror("date_time() - couldn't format date tm to time_t");
        return;
    }
}

char *makePath(char *dirname){
   
    char *path = calloc(strlen(dirname) + strlen(file -> d_name) + 1, sizeof(char));
    strcpy(path, dirname); strcat(path, "/"); strcat(path, file -> d_name);

    return path;
}

void search_lower(DIR *dirp, char *dirname){
    
    while((file = readdir(dirp)) != NULL && !errno){
        
        // make path of current file
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
        // taking stats of linked file
        if(S_ISLNK(stats.st_mode) && S_ISREG(stats.st_mode)){
            if(stat(fullPath, &stats) == -1){
                perror("examine_dir_v1() - error during examining file stats");
                free(path);
                break;
            }
            modTime = stats.st_mtime;
        }
        else if(modTime < dateTime && S_ISREG(stats.st_mode) && !S_ISLNK(stats.st_mode))
            print_info(fullPath, &modTime);
        else if(modTime < dateTime && S_ISDIR(stats.st_mode) && !S_ISLNK(stats.st_mode)
        && strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0 ){
            examine_dir_v1(path);
            printf(COLOR "Wyjscie do procesu PID: %d\n" RESET, (int)getpid());
            if(errno) return;
        }


        free(path);
        free(fullPath);
    }
    if(errno){
        perror("examine_dir_v1() - error during examining directory");
    }
    
    return;    
}

void search_greater(DIR *dirp, char *dirname){

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
        // taking stats of linked file
        if(S_ISLNK(stats.st_mode) && S_ISREG(stats.st_mode)){
            if(stat(fullPath, &stats) == -1){
                perror("examine_dir_v1() - error during examining file stats");
                free(path);
                break;
            }
            modTime = stats.st_mtime;
        }
        else if(modTime > dateTime && S_ISREG(stats.st_mode) && !S_ISLNK(stats.st_mode))
            print_info(fullPath, &modTime);
        else if(modTime > dateTime && S_ISDIR(stats.st_mode) && !S_ISLNK(stats.st_mode)
        && strcmp(file -> d_name, ".") != 0 && strcmp(file -> d_name, "..") != 0 ){
            examine_dir_v1(path);
            printf(COLOR "Wyjscie do procesu PID: %d\n" RESET, (int)getpid());
            if(errno) return;
        }

        free(path);
        free(fullPath);
    }
    if(errno){
        perror("examine_dir_v1() - error during examining directory");
    }

    return;    
}

void search_equal(DIR *dirp, char *dirname){

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
        // taking stats of linked file
        if(S_ISLNK(stats.st_mode) && S_ISREG(stats.st_mode)){
            if(stat(fullPath, &stats) == -1){
                perror("examine_dir_v1() - error during examining file stats");
                free(path);
                break;
            }
            modTime = stats.st_mtime;
        }
        else if(modTime < (dateTime + 86400) && modTime > (dateTime - 1) && S_ISREG(stats.st_mode) 
        && !S_ISLNK(stats.st_mode)) // due to the range of whole day. One day has 86400 seconds
            print_info(fullPath, &modTime);
        else if(S_ISDIR(stats.st_mode) && !S_ISLNK(stats.st_mode) && strcmp(file -> d_name, ".") != 0 
        && strcmp(file -> d_name, "..") != 0 ){
            examine_dir_v1(path);
            printf(COLOR "Wyjscie do procesu PID: %d\n" RESET, (int)getpid());
            if(errno) return;
        }
    
        free(path);
        free(fullPath);
    }
    if(errno){
        perror("examine_dir_v1() - error during examining directory");
    }
 
    return;    
}

int search_files(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
    

    if(tflag == FTW_F){

        stats = *sb;
        time_t modTime = stats.st_mtime;
        
        if(strcmp(operator, "<") == 0){
            if(modTime < dateTime){
                prepare_data_and_print(fpath, &modTime);
            }
        }
        else if(strcmp(operator, ">") == 0){
            if(modTime > dateTime){
                prepare_data_and_print(fpath, &modTime);
            }
        }
        else{
            if(modTime > dateTime - 1 && modTime < dateTime + 86400){
                prepare_data_and_print(fpath, &modTime);
            }
        }
    }
    
    if(errno) return -1;
    
    return 0;
}

void examine_dir_v1(char *dirname){
       
    pid_t childPid = fork();
    if(childPid < 0){
        perror("examine_dir_v1() - error during creating new process");
        return;
    }
    if(childPid == 0){
        printf(COLOR "Proces potomy dla: %s\n" RESET, dirname);
        printf(COLOR "PPID: %d\n" RESET, (int)getppid());
        printf(COLOR "PID: %d\n" RESET, (int)getpid());
        

        DIR *dirp = opendir(dirname);
        if(!dirp){
            perror("Couldn't open stream to given directory");
            return;
        }

        if(strcmp(operator, "<") == 0)
            search_lower(dirp, dirname);
        else if(strcmp(operator, ">") == 0)
            search_greater(dirp, dirname);
        else
            search_equal(dirp, dirname);


        int closeFlag = closedir(dirp);
        if(closeFlag == -1){
            perror("Couldn't close stream to directory");
        }
        exit(0);
    }
    else{
        if(wait(NULL) == -1){
            perror("examine_dir_v1() - wait() error");
            return;
        }
    }
}

void examine_dir_v2(char *dirname){
    
    if(nftw(dirname, search_files, 20, FTW_PHYS) == -1)
        perror("examine_dir_v2() - ");
}

int main(int argc, char **argv){

    printf(COLOR "Proces programu: %d\n" RESET, (int)getpid());

    if(argc != 5){ 
        printf("Wrong number of arguments\n");
        return 0;
    }
    if(strcmp(argv[2], "<") != 0 && strcmp(argv[2], ">") != 0 && strcmp(argv[2], "=") != 0){
        printf("Wrong second argument. It must be < > or =\n");
        return 0;
    }
    if(strcmp(argv[4], "v1") != 0 && strcmp(argv[4], "v2") != 0){
        printf("Wrong version of program\n");
        return 0;
    }
    
    // setting variables 
    char *dirname = argv[1];
    operator = argv[2];
    char *date = argv[3];
    
    date_time(date); // returns time_t representation of given date to global var dateTime
    if(errno) return 0;
    
    if(strcmp(argv[4], "v1") == 0){
        examine_dir_v1(dirname);
        printf(COLOR "Wyjscie do procesu PID: %d\n" RESET, (int)getpid());
    } 
    else if(strcmp(argv[4], "v2") == 0) examine_dir_v2(dirname);
    

    return 0;
}