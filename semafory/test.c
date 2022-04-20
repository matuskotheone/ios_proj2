#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

sem_t* sem = NULL;
FILE* pfile;

int init()
{
    pfile = fopen("proj2.out", "w");
    if ((sem = sem_open("/xgazdi04.ios.proj2.semafor", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return 1;
    return 0;
}

void cleanup()
{
    sem_close(sem);
    sem_unlink("/xgazdi04.ios.proj2.semafor");

    if (pfile != NULL)
    {
        fclose(pfile);
    }
}


int main()
{
    pid_t pid = fork();

    init();

    if (pid == 0)
    {
        puts("som tu");
        //usleep(3);
        sem_wait(sem);
        puts("toto pojde prve");
        exit(0);
    }
    pid = fork();
    if (pid == 0)
    {
        puts("som tu2");
        //usleep(2);
        sem_wait(sem);
        puts("toto pojde druhe");
        exit(0);
    }

    //usleep(4);
    sem_post(sem);
    sem_post(sem);

    usleep(2);
    cleanup();

    return 0;
}
