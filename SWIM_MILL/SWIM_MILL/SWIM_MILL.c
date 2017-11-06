//
//  Swim_Mill_Main.c
//  Swim_Mill
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
#include <stdbool.h>
#include <signal.h>

// Path = "/Users/ScottRoberts/desktop/CSULB/CECS\ 326\ Operating\ Systems/Lab\ assignments/SWIM_MILL_NEW/SWIM_MILL_NEW/"

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

// Swim Mill Variables
#define TIME_LIMIT 30
pid_t fishPID;
pid_t pelletPID;

// Swim Mill Function Prototypes
void KillInterrupt();
void KillClean();
void DisplayGrid();
void CreateGrid();


int main(int argc, char *argv[])
{
    printf("PID %d (Swim Mill) started\n", getpid());
    
    // Setup signal for the Interrupt (^C)
    signal(SIGINT, KillInterrupt);
    
    
    // Get a New File
    FILE *file;
    file = fopen("results.txt", "w+");
    if(file == NULL)
    {
        printf("PID %d (Swim Mill) failed to reset file\n", getpid());
        exit(1);
    }
    else
    {
        fclose(fopen("file.txt", "w"));
    }
    
    // Create grid in shared memory
    GetSharedMemory();
    AttachSharedMemory();
    CreateGrid();
    
    // Start child Fish process
    if((fishPID = fork()) == 0)
    {
        execv("FISH", argv);
        exit(0);
    }
    else if (fishPID > 0)
    {
        // Start child Pellet process
        if((pelletPID = fork()) == 0)
        {
            execv("PELLET", argv);
            exit(0);
        }
        else
        {
            // Run fish and pellet processes for the rest of the program.
            for(int seconds = TIME_LIMIT; seconds >= 0; seconds--)
            {
                printf("%d seconds remaining\n", seconds);
                sleep(1);
                DisplayGrid();
            }
            KillClean();
        }
    }
    return 0;
}

// Create the allocated memory for the shared memory.
void GetSharedMemory()
{
    
    if((shmID = shmget(key, sizeof(grid), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    
}

// Attach pointer to the allocated memory from shmget().
void AttachSharedMemory()
{
    
    if((grid = shmat(shmID, NULL, 0)) == (char *)-1) {
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

// Display the Swim Mill grid to console.
void DisplayGrid()
{
    for(int i = 0; i < ROWS; i++)
    {
        for(int j = 0; j < COLUMNS; j++)
        {
            printf("%c",(*grid)[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Initialize the Grid.
void CreateGrid()
{
    
    // Place "grid" into memory
    for(int i = 0; i < ROWS; i++)
    {
        for(int j = 0; j < COLUMNS; j++)
        {
            (*grid)[i][j] = '~';
        }
    }
    
    // Place fish in middle of last row
    (*grid)[ROWS-1][COLUMNS/2] = 'F';
}

// End everything nicely... seriously... nicely.
void KillClean()
{
    // Kill the Fish.
    int killReturn = kill(fishPID, SIGTERM);
    if (killReturn == -1)
        printf("There was a problem killing (Fish) PID: %d, check that out.\n", fishPID);
    
    // Kill the Pellet.
    killReturn = kill(pelletPID, SIGTERM);
    if (killReturn == -1)
        printf("There was a problem killing (Pellet) PID: %d, check that out.\n", pelletPID);
    
    // Wait for all children to die to move on.
    wait(NULL);
        //printf("Child dead\n");
    
    // Detach and Remove shared memory.
    DetachSharedMemory();
    RemoveSharedMemory();
    
    printf("PID %d (Swim Mill) exited because we have finished.\n", getpid());
    exit(0);
}

// KILL EVERYTHING!!!
void KillInterrupt()
{
    // Kill the Fish.
    int killReturn = kill(fishPID, SIGINT);
    if (killReturn == -1)
        printf("There was a problem killing (Fish) PID: %d, check that out.\n", fishPID);
    
    // Kill the Pellet.
    killReturn = kill(pelletPID, SIGINT);
    if (killReturn == -1)
        printf("There was a problem killing (Pellet) PID: %d, check that out.\n", pelletPID);
    
    // Wait for all children to die to move on.
    wait(NULL);
        //printf("Child dead\n");
    
    // Detach and Remove shared memory.
    DetachSharedMemory();
    RemoveSharedMemory();
    
    printf("PID %d (Swim Mill) exited because of the interrupt.\n", getpid());
    exit(0);
}







