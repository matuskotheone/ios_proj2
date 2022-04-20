#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TIMECONVERT 1000 // what number to multiply us with to get ms

typedef struct Shared {
    //input 
    int no;
    int nh;
    int ti;
    int tb;

    //semaphores
    sem_t hyd;
    sem_t hyd2;
    sem_t oxy;
    sem_t oxy2;
    sem_t output; 

    //else 
    size_t num_rows;
    int molNo;
} shared_t;


// controlling arguments
int arg_process(int argc, char** argv, shared_t* in)
{
    //setting row counter to 0
    in->num_rows = 0;

    if(argc != 5)
    {
        fprintf(stderr,"ERROR: incorrect number of arguments\n");
        exit(EXIT_FAILURE);
    }
    char* end;

    in->no = (int)strtol(argv[1], &end, 0);
    if (*end != '\0' || in->no < 0)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        exit(EXIT_FAILURE);
    }

    in->nh = (int)strtol(argv[2], &end, 0);
    if (*end != '\0' || in->no < 0)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        exit(EXIT_FAILURE);
    }

    in->ti = (int)strtol(argv[3], &end, 0);
    if (*end != '\0' || in->ti < 0 || in->ti > 1000)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        exit(EXIT_FAILURE);
    }

    in->tb = (int)strtol(argv[4], &end, 0);
    if (*end != '\0' || in->tb < 0 || in->tb > 1000)
    {
        fprintf(stderr,"ERROR: incorrect arguments\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

// action:  1 started
//          2 going to queue
//          3 creating molecule 
//          4 created 
void my_print(shared_t* shared, char atom, int id, int action)
{
    sem_wait(&shared->output);
    if (action == 1)
    {
        printf("%ld: %c %d: started\n", ++shared->num_rows, atom, id);
    }
    else if (action == 2)
    {
        printf("%ld: %c %d: going to queue\n", ++shared->num_rows, atom, id);
    }
    else if (action == 3)
    {
        printf("%ld: %c %d: creating molecule %d\n", ++shared->num_rows, atom, id, shared->molNo);
    }
    else if (action == 4)
    {
        printf("%ld: %c %d: molecule %d created\n", ++shared->num_rows, atom, id, shared->molNo);
    }
    sem_post(&shared->output);
}

void rand_wait(int max_time)
{
    usleep((rand()%(max_time+1))*TIMECONVERT);
}

int hydrogen(shared_t* shared, int id)
{
    sem_wait(&(shared->hyd));
    my_print(shared, 'H', id, 1);
    sem_post(&(shared->hyd));
    rand_wait(shared->ti);
    my_print(shared, 'H', id, 2);

    return 0;
}

int oxygen(shared_t* shared, int id)
{
    sem_wait(&(shared->oxy));
    my_print(shared, 'O', id, 1);
    sem_post(&(shared->oxy));
    rand_wait(shared->ti);
    my_print(shared, 'O', id, 2);
    return 0;
}

void destroy_all_sem(shared_t *shared)
{
    sem_destroy(&(shared->hyd));
    sem_destroy(&(shared->hyd2));
    sem_destroy(&(shared->oxy));
    sem_destroy(&(shared->oxy2));

    sem_destroy(&(shared->output));
}


void init_sem(shared_t *shared, sem_t *sem_to_init, int pshared, unsigned int value)
{
    // intialization
    if (sem_init(sem_to_init, pshared, value) == -1)
    {
        fprintf(stderr, "ERROR: Error unable to initializie semaphore.\n");
        destroy_all_sem(shared);
        exit(EXIT_FAILURE);
    }
}


int init(shared_t* shared)
{
    init_sem(shared, &(shared->oxy), 1, 1);
    init_sem(shared, &(shared->oxy2), 1, 1);

    init_sem(shared, &(shared->hyd), 1, 1);
    init_sem(shared, &(shared->hyd2), 1, 0);

    init_sem(shared, &(shared->output), 1, 1);
    return 0; 
}




int main(int argc, char** argv)
{
    // creation of shared memory 
    shared_t *shared = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    if(shared == MAP_FAILED)
    {
        fprintf(stderr,"ERROR: unable to create shared memory.");
        exit(EXIT_FAILURE);
    }

    arg_process(argc, argv, shared);

    init(shared);
    pid_t pid;
    // forking hydrogen processes
    for (int i = 0; i < shared->nh; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            fprintf(stderr, "ERROR: Chyba pri forku");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            hydrogen(shared, i);
            exit(EXIT_SUCCESS);
        }
    }

    // forking oxygen processes
    for (int i = 0; i < shared->no; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            fprintf(stderr, "ERROR: Chyba pri forku");
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            oxygen(shared, i);
            exit(EXIT_SUCCESS);
        }
    }

    while(wait(NULL)>0);
    destroy_all_sem(shared);

    if (munmap(shared, sizeof(shared_t)) == -1)
        fprintf(stderr, "ERROR: unable to clean shared memory.\n");


    return 0;
}

