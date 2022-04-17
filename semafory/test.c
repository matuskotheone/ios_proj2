#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main()
{
    sem_t sem;
    sem_init(&sem, 0, 2);
    sem_wait(&sem);
    puts("hovno");
    sem_wait(&sem);
    puts("hovno");
    sem_destroy(&sem);

    return 0;
}
