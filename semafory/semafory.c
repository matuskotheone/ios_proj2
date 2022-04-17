#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>


typedef struct Input {
    int NO;
    int NH;
    int TI;
    int TB;
} input_t;


int arg_process(int argc, char** argv, input_t* in)
{
    if(argc != 5)
    {
        fprintf(stderr,"ERROR: incorrect number of arguments\n");
        return 1;
    }
    char* end;

    in->NO = (int)strtol(argv[1], &end, 0);
    if (*end != '\0' || in->NO < 0)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        return 1;
    }

    in->NH = (int)strtol(argv[2], &end, 0);
    if (*end != '\0' || in->NO < 0)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        return 1;
    }

    in->TI = (int)strtol(argv[3], &end, 0);
    if (*end != '\0' || in->TI < 0 || in->TI > 1000)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        return 1;
    }

    in->TB = (int)strtol(argv[4], &end, 0);
    if (*end != '\0' || in->TB < 0 || in->TB > 1000)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        return 1;
    }
    return 0;
}


int innit()
{
    //TODO
    return 0; 
}

int cleanup()
{
    //TODO
    return 0; 
}




int main(int argc, char** argv)
{
    input_t in;
    if (arg_process(argc, argv, &in) == 1)
    {
        return 1;
    }

    return 0;
}

