#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>

void nic()
{
    for ( int i = 0; i < 5; i++)
    {
        printf("%d\n", rand()%50);
    }

}

int main()
{
    time_t t;
    srand((unsigned) time(&t));
    nic();

}
