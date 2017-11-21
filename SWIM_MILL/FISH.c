//
//  FISH.c
//  SWIM_MILL_NEW
//
//  Created by Scott Roberts on 9/29/17.
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

// Shared Memory Variables
const int ROWS = 10;
const int COLUMNS = 10;
const key_t key = 1000;
int shmID;
char (*grid)[ROWS][COLUMNS];

// Shared Memory Functions
void GetSharedMemory();
void AttachSharedMemory();
void DetachSharedMemory();
void RemoveSharedMemory();

// Semaphore Variables
#define SEM_LOCATION "/semaphore"
sem_t (*semaphore);

// Semaphore Functions
void OpenSemaphore();
void CloseSemaphore();

// Fish Variables
int fishColumn = ROWS/2;

// Fish Functions
void OnInterruptSignal();
void OnEndSignal();
void MoveFish(int);


int main(int argc, char *argv[])
{
    printf("PID %d (fish) started\n", getpid());
    
    // Setup catch for kill signal from swim_mill
    signal(SIGINT, OnInterruptSignal);
    signal(SIGTERM, OnEndSignal);
    
    // Attach process to shared memory
    GetSharedMemory();
    AttachSharedMemory();
    
    while(1)
    {
        // Start from bottom and go to top, checking all rows for pellets
        for(int row = (ROWS-2); row >= 0; row--)
        {
            // Locate any pellets on the current row
            int currentRowPellets[COLUMNS] = {0};
            for (int col = 0; col < COLUMNS; col++)
            {
                if((*grid)[row][col] == 'P')
                {
                    currentRowPellets[col] = 1;
                }
            }
            
            // If the row just above you has a pellet that is directly
            // in front of you, then don't move for a turn.
            if(row == (ROWS-2) && currentRowPellets[fishColumn] == 1)
            {
                sleep(1);
                break;
            }
            // Else, there isn't a pellet directly above the fish.
            else
            {
                // Zero out the closest pellet to assign later.
                int closestPellet = -1;
                
                /* Find the closest pellet in the row if the fish can
                 even reach that space by the time the pellet falls out*/
                /*
                 int hDistance = fishColumn - col;
                 if (hDistance < 0) hDistance = hDistance * -1;
                 int vDistance = fishRow - row;
                 int slope = vDistance / hDistance;
                 */
                
                for(int col = 0; col < COLUMNS; col++)
                {
                    if(currentRowPellets[col] == 1)
                    {
                        closestPellet = col;
                        break;
                    }
                }
                
                // If there is a pellet in the row that the fish can reach
                if(closestPellet != -1)
                {
                    // Move left if the Closest pellet is in a column to your left.
                    if(closestPellet > fishColumn)
                    {
                        sem_wait(semaphore);
                        MoveFish(1);
                        sem_post(semaphore);
                    }
                    
                    // Move right if the Closest pellet is in a column to your right.
                    else
                    {
                        sem_wait(semaphore);
                        MoveFish(-1);
                        sem_post(semaphore);
                    }
                    
                    // Wait for the next turn to look again
                    sleep(1);
                    break;
                }
            }
        }
    }
}

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

// Retrieve a semaphore from a named location.
void OpenSemaphore()
{
    if ((semaphore = sem_open(SEM_LOCATION, 0)) == SEM_FAILED )
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

// The Fish will move left if the direction is negative and
// right if the direcion is positive.
void MoveFish(int direction)
{
    // Previous position needs updating similar to a linked list.
    (*grid)[ROWS-1][fishColumn] = '~';
    fishColumn += direction;
    (*grid)[ROWS-1][fishColumn] = 'F';
}

// Handler function for receiving an Interrupt (^C).
void OnInterruptSignal()
{
    CloseSemaphore();
    DetachSharedMemory();
    printf("PID %d (Fish) killed because of interrupt\n", getpid());
    exit(0);
}

// Handler function for receiving the end signal.
void OnEndSignal()
{
    CloseSemaphore();
    DetachSharedMemory();
    printf("PID %d (Fish) killed because we have finished\n", getpid());
    exit(0);
}
