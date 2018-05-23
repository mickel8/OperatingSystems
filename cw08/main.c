#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>

// zmienne zegarowe
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

struct args 
{
    int **outTab;
    int **inTab;
    float **filtrTab;
    int inTabSizeX;
    int inTabSizeY;
    int filtrTabSize;
    int startX;
    int endX;   
};

int     **get_inTab(int **, int *, int *, FILE *, FILE **);
float   **get_filtrTab(float **, int *, FILE *);
void    *get_part_outTab(void *);
void    st_clock(void);
void    en_clock(void);

int main(int argc, char *argv[])
{
    if(argc != 5)
    {
        printf("Bad number of arguments\n");
        exit(EXIT_FAILURE);
    }

    int threads  = atoi(argv[1]);
    char *inName     = argv[2];
    char *filtrName  = argv[3];
    char *outName    = argv[4];

    int     **inTab     = NULL;
    float   **filtrTab  = NULL;
    int     **outTab    = NULL;

    int inTabSizeX = 0;
    int inTabSizeY = 0;
    int filtrTabSize = 0;

    // **** OPEN REQUIRED FILES **** //
    
    FILE *in = fopen(inName, "r");
    if(in == NULL)
    {
        perror("main -> fopen");
        exit(EXIT_FAILURE);
    }

    FILE *filtr = fopen(filtrName, "r");
    if(filtr == NULL)
    {
        perror("main -> fopen");
        exit(EXIT_FAILURE);
    }

    FILE *out = fopen(outName, "w");
    if(out == NULL)
    {
        perror("main -> fopen");
        exit(EXIT_FAILURE);
    }

    // **** READ REQUIRED DATA **** //
    inTab       = get_inTab(inTab, &inTabSizeX, &inTabSizeY, in, &out);
    filtrTab    = get_filtrTab(filtrTab, &filtrTabSize, filtr);


    // **** CREATE OUT TAB **** //
    outTab = malloc(inTabSizeX * sizeof(int *));
    for(int i = 0; i < inTabSizeX; i++)
        outTab[i] = malloc(inTabSizeY * sizeof(int));


    // **** COUNT OUT TAB  **** //
   
    st_clock();

    int res;
    pthread_t threadsTab[threads];
    struct args argsListTab[threads];

    struct args argsList;
    argsList.outTab         = outTab;
    argsList.inTab          = inTab;
    argsList.filtrTab       = filtrTab;
    argsList.inTabSizeX     = inTabSizeX;
    argsList.inTabSizeY     = inTabSizeY;
    argsList.filtrTabSize   = filtrTabSize;

    int width = inTabSizeX / threads;

    for(int i = 0; i < threads; i++)
    {
        argsListTab[i] = argsList;
        argsListTab[i].startX = i * width;
        argsListTab[i].endX   = argsListTab[i].startX + width;
        res = pthread_create(&threadsTab[i], NULL, &get_part_outTab, (void *)&argsListTab[i]);
        if(res != 0)
        {
            perror("main -> pthread_create");
            return 0;
        }
    }

    for(int i = 0; i < threads; i++)
    {
        res = pthread_join(threadsTab[i], NULL);
        if(res != 0)
        {
            perror("main -> pthread_join");
            return 0;
        }
    }

    en_clock();

    // **** WRITE TO FILE **** // 

    for(int i = 0; i < inTabSizeY; i++)
    {
        for(int j = 0; j < inTabSizeX; j++)
        {
            char str[12];
            sprintf(str, "%d", outTab[i][j]);
            fputs(str, out);
            fputs(" ", out);
        }
        fputs("\n", out);
    }


    return 0;
}


int **get_inTab(int **inTab, int *inTabSizeX, int *inTabSizeY, FILE *in, FILE **out)
{
    ssize_t res;
    char *lineptr = NULL;
    size_t n = 0;

    for(int i = 0; i < 3; i++)
    {
        res = getline(&lineptr, &n, in);
        fputs(lineptr, *out);
        if(res == -1 && errno)
        {
            perror("get_inTab -> getline");
            exit(EXIT_FAILURE);
        }
    }


    char *inTabSize = strtok(lineptr, " ");
    *inTabSizeX     = atoi(inTabSize);
    inTabSize = strtok(NULL, " ");
    *inTabSizeY     = atoi(inTabSize);
    inTab       = malloc(*inTabSizeX * sizeof(int *));
    for(int i = 0; i < *inTabSizeX; i++)
        inTab[i] = malloc(*inTabSizeY * sizeof(int));
    

    res = getline(&lineptr, &n, in);
    if(res == -1 && errno)
    {
        perror("get_inTab -> getline");
        exit(EXIT_FAILURE);
    }
    fputs(lineptr, *out);

    int x =0;
    int y =0;

    lineptr = NULL;
    n = 0;
    char s[3] = {' '};

    int i = 0;
    int j = 0;

    while(getline(&lineptr, &n, in) != -1)
    {
        while(lineptr[i] != '\0')
        {
            if( '0' <= lineptr[i]  && lineptr[i] <= '9')
            {
                s[j] = lineptr[i];
                j++;
            }
            else 
            {
                if(j != 0 )
                {
                    inTab[y][x] = atoi(s);
                    x++;
                    if(x == *inTabSizeX)
                    {
                        x = 0;
                        y++;
                    }
                    s[0] = s[1] = s[2] = s[3] = ' ';
                    j = 0;
                }
            }
            i++;
        }
        
        i = 0;

    }

    return inTab;
}


float **get_filtrTab(float **filtrTab, int *filtrTabSize, FILE *filtr)
{

    char *lineptr = NULL;
    size_t n = 0;

    if(getline(&lineptr, &n, filtr) == -1 && errno)
    {
        perror("get_filtrTab -> geline");
        exit(EXIT_FAILURE);
    }
    
    *filtrTabSize = atoi(lineptr);
    filtrTab = malloc(sizeof(float *) * (*filtrTabSize));
    for(int i = 0; i < *filtrTabSize; i++)
        filtrTab[i] = malloc(sizeof(float) * (*filtrTabSize));


    
    int x = 0;
    int y = 0;

    lineptr = NULL;
    n = 0;
    char s[4] = {' '};
    
    int i = 0;
    int j = 0;
    
    while(getline(&lineptr, &n, filtr) != -1)
    {
        while(lineptr[i] != '\0')
        {
            if( ('0' <= lineptr[i]  && lineptr[i] <= '9' ) || lineptr[i] == '.' )
            {
                s[j] = lineptr[i];
                j++;
            }
            else 
            {
                if(j != 0 )
                {
                    filtrTab[y][x] = atof(s);
                    x++;
                    if(x == *filtrTabSize)
                    {
                        x = 0;
                        y++;
                    }
                    s[0] = s[1] = s[2] = s[3] = ' ';
                    j = 0;
                }
            }
            i++;
        }
        
        i = 0;

    }

    return filtrTab;
}

void *get_part_outTab(void *arguments)
{

    
    struct args *argsList = arguments;

    int **outTab = argsList -> outTab;
    int **inTab = argsList -> inTab;
    float **filtrTab = argsList -> filtrTab;
    int inTabSizeX = argsList -> inTabSizeX;
    int inTabSizeY = argsList -> inTabSizeY;
    int filtrTabSize = argsList -> filtrTabSize;
    int startX = argsList -> startX;
    int endX = argsList -> endX;

    float sum = 0;
    int x, y;
    for(int i = 0; i < inTabSizeY; i++)
    {
        for(int j = startX; j < endX; j++)
        {
            sum = 0;
            for(int k = 0; k < filtrTabSize; k++)
            {
                for(int l = 0; l < filtrTabSize; l++)
                {
                    x = (0 > (j - (int)ceil(filtrTabSize/2) + k)) ? 0: (j - (int)ceil(filtrTabSize/2) + k);
                    y = (0 > (i - (int)ceil(filtrTabSize/2) + l)) ? 0: (i - (int)ceil(filtrTabSize/2) + l);

                
                    if(x < inTabSizeX) sum += (inTab[y][x] * filtrTab[k][l]);
                    else sum += (inTab[y][inTabSizeX - 1] * filtrTab[k][l]);
                }
            }


            outTab[i][j] = (int)round(sum);
        }

    }

    pthread_exit(0);
    return NULL;

}

void st_clock(){
    st_time = times(&st_cpu);
}

void en_clock(){
    static long int clk = 0;
    clk = sysconf(_SC_CLK_TCK);
    en_time = times(&en_cpu);

    int n = 3;
    double measurements[n];
    measurements[0] = (en_time - st_time) / (double)clk;
    measurements[1] = (en_cpu.tms_utime - st_cpu.tms_utime) / (double)clk;
    measurements[2] = (en_cpu.tms_stime - st_cpu.tms_stime) / (double)clk;

    printf("Real time: %f, user time: %f, system time: %f\n", measurements[0], measurements[1], measurements[2]);

}


