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
#define A_IN_MOL 3

typedef struct Shared {
    //input 
    int no;
    int nh;
    int ti;
    int tb;

    //semaphores
    sem_t hyd;
    sem_t oxy;
    sem_t output; 
    sem_t mol; 

    sem_t bar; 
    sem_t bar2; 

    sem_t mutex;
    sem_t mutex2;

    sem_t end; 

    //else 
    int count;
    int hydrogen;
    int oxygen;
    int curr_h;
    int curr_o;
    int next;       // if 1 no more
    size_t num_rows;
    int molNo;
    FILE* output_f ;
    
       
} shared_t;

// controlling arguments
int arg_process(int argc, char** argv, shared_t* in)
{

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
//          5 no enough o or h
//          6 not enough h
void my_print(shared_t* shared, char atom, int id, int action)
{
    sem_wait(&shared->output);
    if (action == 1)
    {
        fprintf(shared->output_f,"%ld: %c %d: started\n", ++shared->num_rows, atom, id);
    }
    else if (action == 2)
    {
        fprintf(shared->output_f,"%ld: %c %d: going to queue\n", ++shared->num_rows, atom, id);
    }
    else if (action == 3)
    {
        fprintf(shared->output_f,"%ld: %c %d: creating molecule %d\n", ++shared->num_rows, atom, id, shared->molNo/3);
    }
    else if (action == 4)
    {
        fprintf(shared->output_f,"%ld: %c %d: molecule %d created\n", ++shared->num_rows, atom, id, shared->molNo++/3);
    }
    else if (action == 5)
    {
        fprintf(shared->output_f,"%ld: %c %d: not enough O or H\n", ++shared->num_rows, atom, id);
    }
    else if (action == 6)
    {
        fprintf(shared->output_f,"%ld: %c %d: not enough H\n", ++shared->num_rows, atom, id);
    }
    fflush(shared->output_f);
    sem_post(&shared->output);
}

void rand_wait(int max_time)
{
    usleep((rand()%(max_time+1))*TIMECONVERT);
}

// if enough atoms for molecule lets them go through 
void creating_mol(shared_t *shared)
{
    sem_post(&(shared->hyd));
    sem_post(&(shared->hyd));
    shared->hydrogen -= 2;
    shared->curr_h -= 2;
    sem_post(&(shared->oxy));
    shared->oxygen -= 1;
    shared->curr_o -= 2;
}

// controlls if can be created more molecules of not lets atoms go through semaphores 
void poss_next(shared_t *shared)
{
    if (shared->curr_h < 2 || shared->curr_o < 1)
    {
        shared->next = 1;
        for(int i = 0; i < shared->curr_o; i++)
        {
            sem_post(&(shared->oxy));
            sem_post(&(shared->mutex));
        }

        for(int i = 0; i < shared->curr_h; i++)
        {
            sem_post(&(shared->hyd));
            sem_post(&(shared->mutex));
        }
    }
}

void barrier(shared_t* shared, int n)
{
    sem_wait(&(shared->mutex2));
    shared->count += 1;
    if (shared->count == n)
    {
        for (int i = 0; i < n; i++) 
            sem_post(&(shared->bar));
    }

    sem_post(&(shared->mutex2));
    sem_wait(&(shared->bar));

    sem_wait(&(shared->mutex2));

    shared->count -= 1;
    
    if (shared->count == 0)
    {
        for (int i = 0; i < n; i++) 
            sem_post(&(shared->bar2));
    }
    sem_post(&(shared->mutex2));
    sem_wait(&(shared->bar2));
}






int hydrogen(shared_t* shared, int id)
{
    int value;
    //starting and going to q
    my_print(shared, 'H', id, 1);
    rand_wait(shared->ti);
    my_print(shared, 'H', id, 2);


    //mutex
    sem_wait(&(shared->mutex));

    if (shared->next)
    {
        puts("pomutexe hydrogen");
        my_print(shared, 'H', id, 5);
        shared->curr_h--;
        sem_post(&(shared->mutex));
        sem_post(&(shared->hyd));
        sem_post(&(shared->oxy));
        exit(EXIT_FAILURE);
    }
    /*
    if (shared->curr_h < 2 || shared->curr_o < 1)
    {
        my_print(shared, 'H', id, 5);
        shared->curr_h--;
        sem_post(&(shared->mutex));
        exit(EXIT_SUCCESS);
    }
    */

    shared->hydrogen++;

    if (shared->hydrogen >= 2 && shared->oxygen >= 1)
    {
        creating_mol(shared);
    }
    else
    {
        sem_post(&(shared->mutex));
    }

    sem_wait(&(shared->hyd));

    if (shared->next)
    {
        puts("neskorsejc hydrogen ");
        my_print(shared, 'H', id, 5);
        shared->curr_h--;
        sem_post(&(shared->mutex));
        sem_post(&(shared->hyd));
        sem_post(&(shared->oxy));
        exit(EXIT_FAILURE);
    }
    // po tadialto som dobre////////////////////////////////  
   
    my_print(shared, 'H', id, 3);

    barrier(shared, A_IN_MOL);

    my_print(shared, 'H', id, 4);

    sem_post(&(shared->end));

    return 0;
}


int oxygen(shared_t* shared, int id)
{
    int value;
    //starting and going to q
    my_print(shared, 'O', id, 1);
    rand_wait(shared->ti);
    my_print(shared, 'O', id, 2);

    //mutex
    sem_wait(&(shared->mutex));

    if (shared->next)
    {
        puts("pomutexe oxy ");
        my_print(shared, 'O', id, 6);
        shared->curr_o--;
        sem_post(&(shared->mutex));
        sem_post(&(shared->hyd));
        sem_post(&(shared->oxy));
        exit(EXIT_FAILURE);
    }

    /*
    if (shared->curr_h < 2)
    {
        my_print(shared, 'O', id, 6);
        shared->curr_o--;
        sem_post(&(shared->mutex));
        exit(EXIT_FAILURE);
    }
    */

    shared->oxygen++;

    if (shared->hydrogen >= 2)
    {
        creating_mol(shared);
    }
    else 
    {
        sem_post(&(shared->mutex));
    }


    sem_wait(&(shared->oxy));
    
    if (shared->next)
    {
        puts("neskorsejc oxy ");
        my_print(shared, 'O', id, 6);
        shared->curr_o--;
        sem_post(&(shared->mutex));
        sem_post(&(shared->hyd));
        sem_post(&(shared->oxy));
        exit(EXIT_FAILURE);
    }

    // po tadialto som dobre////////////////////////////////  

    my_print(shared, 'O', id, 3);

    rand_wait(shared->tb);    

    barrier(shared, A_IN_MOL);
    
    sem_wait(&(shared->end));
    sem_wait(&(shared->end));

    my_print(shared, 'O', id, 4);

    poss_next(shared);
    sem_post(&(shared->mutex));

    return 0;
}

void destroy_all(shared_t *shared)
{
    sem_destroy(&(shared->hyd));
    sem_destroy(&(shared->oxy));

    sem_destroy(&(shared->mol));


    sem_destroy(&(shared->mutex));
    sem_destroy(&(shared->mutex2));
    sem_destroy(&(shared->bar));
    sem_destroy(&(shared->bar2));
    sem_destroy(&(shared->end));

    sem_destroy(&(shared->output));

    fclose(shared->output_f);


}


void init_sem(shared_t *shared, sem_t *sem_to_init, int pshared, unsigned int value)
{
    // intialization
    if (sem_init(sem_to_init, pshared, value) == -1)
    {
        fprintf(stderr, "ERROR: Error unable to initializie semaphore.\n");
        destroy_all(shared);
        exit(EXIT_FAILURE);
    }
}


int init(shared_t* shared)
{
    init_sem(shared, &(shared->oxy), 1, 0);

    init_sem(shared, &(shared->hyd), 1, 0);


    init_sem(shared, &(shared->mol), 1, 0);

    init_sem(shared, &(shared->mutex), 1, 1);
    init_sem(shared, &(shared->mutex2), 1, 1);

    init_sem(shared, &(shared->output), 1, 1);

    init_sem(shared, &(shared->bar), 1, 0);
    init_sem(shared, &(shared->bar2), 1, 0);

    init_sem(shared, &(shared->end), 1, 0);

    shared->output_f = fopen("proj2.out", "w");

    shared->num_rows = 0;
    shared->hydrogen = 0;
    shared->oxygen = 0;
    shared->molNo = 3;
    shared->count = 0;
    shared->next = 0;

    shared->curr_h = shared->nh;
    shared->curr_o = shared->no;

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
            hydrogen(shared, i+1 );
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
            oxygen(shared, i+1);
            exit(EXIT_SUCCESS);
        }
    }

    while(wait(NULL)>0);

    destroy_all(shared);

    if (munmap(shared, sizeof(shared_t)) == -1)
        fprintf(stderr, "ERROR: unable to clean shared memory.\n");


    return 0;
}

