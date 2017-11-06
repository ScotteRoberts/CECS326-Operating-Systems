//
//  PELLET.c
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
#include <stdbool.h>
#include <signal.h>
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


// Shared Memory Functions
void GetSharedMemory();
void AttachSharedMemory();
void DetachSharedMemory();
void RemoveSharedMemory();

//Pellet Functions
void catchKillSig();
void catchEndSig();
void OnInterruptSignal();
void OnEndSignal();
static void *child(void*);


// Pellet Variables
#define MAX_THREADS 20
#define MAX_INTERVAL 3
#define MIN_INTERVAL 1
static int TOTAL_THREADS = 30;


int main(int argc, char *argv[])
{
    printf("PID %d (pellet) started\n", getpid());

    // Setup signal intercepts from swim_mill and keyboard (^C)
    signal(SIGINT, OnInterruptSignal);
    signal(SIGTERM, OnEndSignal);
    
    // Attach process to shared memory
    GetSharedMemory();
    AttachSharedMemory();
    
    // Set the Random Number generator with the seed
    srand(time(NULL));
    
    // Create the thread array to make multiple pellets
    pthread_t threads[TOTAL_THREADS];
    
    //Create threads until the maximum is reached.
    int threadCounter = 0;
    for(int i = 0; i < TIME_LIMIT; i++)
    {
        if(threadCounter <= MAX_THREADS)
        {
            // Sleep for random interval defined by min and max intervals
            int sleepTime = rand() % MAX_INTERVAL + MIN_INTERVAL;
            sleep(sleepTime);
            
            // Spawn new pellet thread
            pthread_create(&threads[threadCounter], NULL, child, NULL);
        }
    }
    
    // Wait until all of the threads from our thread list
    // have been destroyed to continue.
    pthread_join(threads[TOTAL_THREADS-1], NULL);
    
    DetachSharedMemory();
    printf("PID %d (Pellet) exited because we ended\n", getpid());
    exit(0);
}

static void *child(void* ignored)
{
    // Generate a random position
    int pelletRow, pelletCol;
    do
    {
        pelletRow = rand()%8+1;
        pelletCol = rand()%8+1;
    }
    while((*grid)[pelletRow][pelletCol] == 'P');
    
    bool eaten = false, leftStream = false;
    
    printf("Pellet %d was dropped at [%d,%d]\n", pthread_self(), pelletCol, pelletRow);
    while(1)
    {
        (*grid)[pelletCol][pelletRow] = 'P';
        
        // Advance pellet downwards in stream
        sleep(1);
        pelletCol++;
        
        // Check if pellet was eaten or left stream
        if(pelletCol == ROWS)
            leftStream = true;
        else if((*grid)[pelletCol][pelletRow] == 'F')
            eaten = true;
        
        // Overwrite previous position with grid symbol
        if((*grid)[pelletCol-1][pelletRow] != 'F')
            (*grid)[pelletCol-1][pelletRow] = '~';
        
        if(eaten || leftStream)
            break;
    }
    
    // Print pellet results to screen
    if(eaten)
        printf("Pellet %d was eaten at column %d!\n", pthread_self(), pelletRow);
    else
        printf("Pellet %d wasn't eaten and left stream at column %d\n", pthread_self(), pelletRow);
    
    
    // Append pellet results to file
    FILE *fp;
    fp = fopen("results.txt", "a");
    if(fp == NULL)
    {
        printf("TID %d (pellet) failed to write results\n", pthread_self());
        exit(1);
    }
    else
    {
        if(eaten)
            fprintf(fp, "Pellet %d was eaten at column %d!\n", pthread_self(), pelletRow);
        else
            fprintf(fp, "Pellet %d wasn't eaten and left stream at column %d\n", pthread_self(), pelletRow);
        fclose(fp);
    }
    
    printf("Pellet %d is exiting.\n", pthread_self());
    return NULL;
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


// Handler function for receiving an Interrupt (^C).
void OnInterruptSignal()
{
    // Wait for all children to die to move on.
    wait(NULL);
    DetachSharedMemory();
    printf("PID %d (Pellet) killed because of interrupt\n", getpid());
    exit(0);
}

// Handler function for receiving the end signal.
void OnEndSignal()
{
    // Wait for all children to die to move on.
    wait(NULL);
    DetachSharedMemory();
    printf("PID %d (Pellet) killed because we have finished\n", getpid());
    exit(0);
}
