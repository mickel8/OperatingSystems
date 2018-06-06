#define _OPEN_THREADS
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// **** Functions definitions **** //

void read_data(char **);
void mutex_version(void);
void sem_version(void);
void *go_to_work(void *);
void *go_for_shopping(void *);
void *go_to_work_2(void *);
void *go_for_shopping_2(void *);

// **** END **** //



// **** Variables in config.txt **** //

static int P;
static int K;
static int N;
static char *filename;
static int L;
static int searchingVersion;
static int verbose;
static int nk;
static int comparison;

// **** END **** //



// **** User variables **** //

static FILE *in;

static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t *mutexes;
static pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_empt = PTHREAD_COND_INITIALIZER;

static char **buffor;

static int last_input = -1;
static int last_reading = -1;
static int buffor_fill = 0;

static size_t getlineRes = 0;

static sem_t fillCount;
static sem_t emptCount;
static sem_t bufforSem;

static int threadP = 0;
static int threadK = 0;

// **** END **** //



int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Bad number of arguments\n");
        exit(EXIT_FAILURE);
    }

    read_data(argv);

    if(searchingVersion == 0) mutex_version();
    else if(searchingVersion == 1) sem_version();
    else 
    {
        printf("Bad searching version number\n");
        exit(EXIT_FAILURE);
    }

}


void read_data(char **argv)
{
    
    FILE *file = fopen(argv[1], "r");
    if(file == NULL)
    {
        perror("read_data -> fopen");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t n = 0;

    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else P = atoi(line);

    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else K = atoi(line);
    
    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else N = atoi(line);

    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else 
    {
        filename = malloc(strlen(line) * sizeof(char));
        strcpy(filename, line);
        for(int i = 0; filename[i] != '\0'; i++)
        {
            if(filename[i] == '\n') filename[i] = '\0';
        }

    }

    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else L = atoi(line);
    
    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else searchingVersion = atoi(line);
        
    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else verbose = atoi(line);
    
    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else
    {
        nk = atoi(line);
        if(nk < 0)
        {
            printf("Bad nk value\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    } 

    if(getline(&line, &n, file) == -1 && errno)
    {
        perror("main -> getline");
        exit(EXIT_FAILURE);
    }
    else
    {
        if(strcmp(line, ">") == 0) comparison = 0;
        else if(strcmp(line, "=") == 0) comparison = 1;
        else if(strcmp(line, "<") == 0) comparison = 2;
        else 
        {
            printf("Bad comparison sign\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
}


void mutex_version()
{
    int res;

    in = fopen(filename, "r");
    if(in == NULL)
    {
        perror("mutex_version -> fopen");
        exit(EXIT_FAILURE);
    }

    buffor = calloc(N, sizeof(char *));
    
    // mutexes = calloc(N, sizeof(pthread_mutex_t));
    // for(int i = 0; i < N; i++) {
    //     res = pthread_mutex_init(&mutexes[i], NULL);
    //     if(res != 0) perror("mutex_version -> pthread_mutex_init");
        
    // }

    pthread_t *threads_P = calloc(P, sizeof(pthread_t));
    pthread_t *threads_K = calloc(K, sizeof(pthread_t));

    for(int i = 0; i < P; i++)
    {
        res = pthread_create(&threads_P[i], NULL, &go_for_shopping, NULL);
        if(res != 0) perror("mutex_version -> pthread_create");
    }

    for(int i = 0; i < K; i++)
    {
        res = pthread_create(&threads_K[i], NULL, &go_to_work, NULL);
        if(res != 0) perror("mutex_version -> pthread_create");
    }

    if(nk > 0)
    {
        time_t start = time(NULL);
        time_t end = start + nk;

        while(start < end) start = time(NULL);
        
        for(int i = 0; i < P; i++)
        {
            if(pthread_cancel(threads_P[i]) != 0) perror("mutex_version -> pthread_cancel");
        }
        
        for(int i = 0; i < K; i++)
        {
            if(pthread_cancel(threads_K[i]) != 0) perror("mutex_version -> pthread_cancel");
        }

    }
    else  
    {
        for(int i = 0; i < P; i++)
        {
            res = pthread_join(threads_P[i], NULL);
            if(res != 0) perror("mutex_version -> pthread_join");
        }
        
        for(int i = 0; i < K; i++)
        {
            res = pthread_join(threads_K[i], NULL);
            if(res != 0) perror("mutex_version -> pthread_join");
        }

    }
    
    fclose(in);
    pthread_mutex_destroy(&data_mutex);
    

}

void *go_to_work(void *args)
{
    char *line = NULL;
    size_t n = 0;
    int index;
    int res;

    int myID;

    pthread_mutex_lock(&data_mutex);
    myID = ++threadP;
    pthread_mutex_unlock(&data_mutex);

    while(1)
    {
        
        pthread_mutex_lock(&data_mutex);

        while(buffor_fill == N) pthread_cond_wait(&cond_full, &data_mutex);

        if(last_input == N -1 ) last_input = 0;
        else last_input = last_input + 1;
        index = last_input;

        res = getline(&line, &n, in);        
        if( res == -1)
        {
            if(errno) perror("go_to_work -> getline");
            else 
            {

                buffor[index] = malloc(10 * sizeof(char));
                strcpy(buffor[index], "END");
                buffor_fill++; 
                pthread_cond_signal(&cond_empt);
                pthread_mutex_unlock(&data_mutex);
                
                break;
            
            }
        } 
        else
        {
            buffor[index] = malloc((strlen(line) + 10) * sizeof(char));
            strcpy(buffor[index], line);
            if(verbose == 1) printf("\e[96m %d:%d %s \e[39m \n", index, myID, buffor[index]);
        }
        buffor_fill++;
        pthread_cond_signal(&cond_empt);
        
        pthread_mutex_unlock(&data_mutex);
    }

    return NULL;
}

void *go_for_shopping(void *args)
{
    int index;
    char *line;

    int myID;

    pthread_mutex_lock(&data_mutex);
    myID = ++threadK;
    pthread_mutex_unlock(&data_mutex);
    
    while(1)
    {
        
        pthread_mutex_lock(&data_mutex);

        while(buffor_fill == 0) pthread_cond_wait(&cond_empt, &data_mutex);
     
        if(last_reading == N - 1) last_reading = 0;
        else last_reading = last_reading + 1;
        index = last_reading;

        line = malloc((strlen(buffor[index]) + 10) * sizeof(char));
        strcpy(line, buffor[index]);
        free(buffor[index]);

        buffor_fill--;
        pthread_cond_signal(&cond_full);

        pthread_mutex_unlock(&data_mutex);

        if(strcmp(line, "END") == 0)
        {
            free(line);
            break;
        }
        
        switch(comparison)
        {
            case 0:
                if(strlen(line) > L) printf("Index %d:%d, %s \n", index, myID, line);
                break;
            case 1:
                if(strlen(line) == L) printf("Index %d:%d, %s \n", index, myID, line);
                break;
            case 2:
                if(strlen(line) < L) printf("Index %d:%d, %s \n", index, myID, line);
                break;
        } 
        
        free(line);
    }

    return NULL;
}


void sem_version()
{

    int res;

    in = fopen(filename, "r");
    if(in == NULL)
    {
        perror("mutex_version -> fopen");
        exit(EXIT_FAILURE);
    }

    res = sem_init(&fillCount, 0, 0);
    if(res != 0)
    {
        perror("sem_version -> sem_init");
        exit(EXIT_FAILURE);
    }

    res = sem_init(&emptCount, 0, N);
    if(res != 0)
    {
        perror("sem_version -> sem_init");
        exit(EXIT_FAILURE);
    }
    
    res = sem_init(&bufforSem, 0, 1);
    if(res != 0)
    {
        perror("sem_version -> sem_init");
        exit(EXIT_FAILURE);
    }

    buffor = calloc(N, sizeof(char *));

    pthread_t *threads_P = calloc(P, sizeof(pthread_t));
    pthread_t *threads_K = calloc(K, sizeof(pthread_t));

    for(int i = 0; i < P; i++)
    {
        res = pthread_create(&threads_P[i], NULL, &go_for_shopping_2, NULL);
        if(res != 0) perror("mutex_version -> pthread_create");
    }

    for(int i = 0; i < K; i++)
    {
        res = pthread_create(&threads_K[i], NULL, &go_to_work_2, NULL);
        if(res != 0) perror("mutex_version -> pthread_create");
    }

    if(nk > 0)
    {
        time_t start = time(NULL);
        time_t end = start + nk;

        while(start < end) start = time(NULL);

        for(int i = 0; i < P; i++)
        {
            if(pthread_cancel(threads_P[i]) != 0) perror("sem_version -> pthread_cancel");
        }
        
        for(int i = 0; i < K; i++)
        {
            if(pthread_cancel(threads_K[i]) != 0) perror("sem_version -> pthread_cancel");
        }

    }
    else 
    {
        for(int i = 0; i < P; i++)
        {
            
            res = pthread_join(threads_P[i], NULL);
            if(res != 0) perror("sem_version -> pthread_join");
        }

        for(int i = 0; i < K; i++)
        {
            res = pthread_join(threads_K[i], NULL);
            if(res != 0) perror("sem_version -> pthread_join");
        }
    }

    fclose(in);
    sem_destroy(&fillCount);
    sem_destroy(&emptCount);
    sem_destroy(&bufforSem);

}

void *go_to_work_2(void *args)
{
    int res;

    char *line = NULL;
    size_t n = 0;
    
    int myID;
    pthread_mutex_lock(&data_mutex);
    myID = ++threadP;
    pthread_mutex_unlock(&data_mutex);


    while(1)
    {
        res = sem_wait(&emptCount);
        if(res != 0) perror("go_to_work_2 -> sem_wait");

        res = sem_wait(&bufforSem);
        if(res != 0) perror("go_to_work_2 -> sem_wait");

        if(last_input == N - 1) last_input = -1;

        getlineRes = getline(&line, &n, in);
        if(getlineRes == -1)
        {
            if(errno) perror("go_to_work_2 -> getline");
            else 
            {

                buffor[last_input + 1] = malloc(10*sizeof(char));
                strcpy(buffor[last_input + 1], "END\0");
                last_input ++;

                res = sem_post(&bufforSem);
                if(res != 0) perror("go_to_work_2 -> sem_post");
                
                res = sem_post(&fillCount);
                if(res != 0) perror("go_to_work_2 -> sem_wait");

                break;
            }
        } 
        
        buffor[last_input + 1] = malloc((strlen(line) + 10) * sizeof(char));
        strcpy(buffor[last_input + 1], line);
        last_input++;
        if(verbose == 1) printf("\e[96m %d:%d %s \e[39m \n", last_input, myID, buffor[last_input]);

        res = sem_post(&bufforSem);
        if(res != 0) perror("go_to_work_2 -> sem_wait");

        res = sem_post(&fillCount);
        if(res != 0) perror("go_to_work_2 -> sem_wait");


    }
    
    return NULL;
}

void *go_for_shopping_2(void *args)
{
    int res;
    
    char *line;
    int index;

    int myID;
    pthread_mutex_lock(&data_mutex);
    myID = ++threadK;
    pthread_mutex_unlock(&data_mutex);

    while(1)
    {
        res = sem_wait(&fillCount);
        if(res != 0) perror("go_for_shopping_2 -> sem_wait");
            
        res = sem_wait(&bufforSem);
        if(res != 0) perror("go_for_shopping_2 -> sem_wait");

        if(last_reading == N - 1) last_reading = 0;
        else last_reading = last_reading + 1;
        index = last_reading;

        line = malloc((strlen(buffor[index]) + 10) * sizeof(char));
        strcpy(line, buffor[index]);
        free(buffor[index]);

        res = sem_post(&bufforSem);
        if(res != 0) perror("go_for_shopping_2 -> sem_wait");

        res = sem_post(&emptCount);
        if(res != 0) perror("go_for_shopping_2 -> sem_wait");

        if(strcmp(line, "END\0") == 0)
        {
            free(line);
            break;
        }

        switch(comparison)
        {
            case 0:
                if(strlen(line) > L) printf("Index %d:%d, %s \n", index, myID, line);
                break;
            case 1:
                if(strlen(line) == L) printf("Index %d:%d, %s \n", index, myID, line);
                break;
            case 2:
                if(strlen(line) < L) printf("Index %d:%d, %s \n", index, myID, line);
                break;
        } 
        
        free(line);
    }
    
    return NULL;
}

