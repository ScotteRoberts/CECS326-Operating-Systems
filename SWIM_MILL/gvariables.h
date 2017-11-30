#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

// Shared Memory Variables
#define TIME_LIMIT 30
const int ROWS = 10;
const int COLUMNS = 10;
const key_t key = 1000;
int shmID;
char (*grid)[ROWS][COLUMNS];

// Create the allocated memory for the shared memory.
void GetSharedMemory()
{
    if((shmID = shmget(key, sizeof(grid), IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
}

// Attach pointer to the allocated memory from shmget().
void AttachSharedMemory()
{
    if((grid = shmat(shmID, NULL, 0)) == (char *)-1)
    {
        perror("shmat");
        exit(1);
    }
}

// Remove memory pointer from the allocated memory.
void DetachSharedMemory()
{
    if (shmdt(grid) == -1)
    {
        perror("shmdt");
        exit(1);
    }
}

// Remove the allocated memory from shared memory.
void RemoveSharedMemory()
{
    if(shmctl(shmID, IPC_RMID, 0) == -1)
    {
        perror("shmctl");
        exit(1);
    }
}

// Semaphore Variables
#define SEM_LOCATION "/semaphore"
sem_t (*semaphore);

// Retrieve a semaphore from a named location.
void OpenSemaphore()
{
    if ((semaphore = sem_open(SEM_LOCATION, O_CREAT, 0644, 1)) == SEM_FAILED )
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

// Close a semaphore to deny access.
void CloseSemaphore()
{
    if (sem_close(semaphore) == -1)
    {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
}