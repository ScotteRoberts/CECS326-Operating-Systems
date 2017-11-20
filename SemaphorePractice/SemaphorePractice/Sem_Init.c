//
//  Sem_Init.c
//  SemaphorePractice
//
//  Created by Scott Roberts on 11/19/17.
//  Copyright © 2017 Scott Roberts. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <stdbool.h>
#include <signal.h>


// Shared memory variables
struct semaphore
{
    char namedLocation[50];
    sem_t (*semaphore);
};

// Shared Memory Functions
void OpenSemaphore(sem_t *, char[]);
void CloseSemaphore(sem_t *);
void UnlinkSemaphore(char[]);

int main(int argc, char *argv[])
{
    struct semaphore x = {.namedLocation = "/semaphore"};
    int pid = fork();
    OpenSemaphore(x.semaphore, x.namedLocation);
    sem_wait(x.semaphore);
    printf("Process: %d\n", getpid());
    sem_post(x.semaphore);
    //sleep(1);
    if(pid == 0)
    {
        printf("Child dying...\n");
        exit(EXIT_SUCCESS);
    }
    wait(NULL);
    CloseSemaphore(x.semaphore);
    UnlinkSemaphore(x.namedLocation);
    printf("I made it!\n");
    exit(EXIT_SUCCESS);
}


// Create/Retrieve a semaphore from a named location.
void OpenSemaphore(sem_t (*sem), char location[])
{
    if ((sem = sem_open(location, O_CREAT, 0644, 1)) == SEM_FAILED ){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

// Close a semaphore to deny access.
void CloseSemaphore(sem_t (*sem))
{
    if (sem_close(sem) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
}

// Remove the semaphore from memory.
void UnlinkSemaphore(char location[])
{
    if (sem_unlink(location) == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
}
