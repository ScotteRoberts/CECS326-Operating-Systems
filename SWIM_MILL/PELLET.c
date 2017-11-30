//
//  PELLET.c
//  SWIM_MILL_NEW
//
//  Created by Scott Roberts on 9/29/17.
//  Copyright © 2017 Scott Roberts. All rights reserved.
//
#include "gvariables.h"

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

    OpenSemaphore();
    
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
    
    sem_wait(semaphore);
    printf("Pellet %d was dropped at [%d,%d]\n", pthread_self(), pelletCol, pelletRow);
    sem_post(semaphore);
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
        {
            //sem_wait(semaphore);
            (*grid)[pelletCol-1][pelletRow] = '~';
            //sem_post(semaphore);

        }
        
        if(eaten || leftStream)
            break;
    }
    
    // Print pellet results to screen
    sem_wait(semaphore);
    if(eaten)
        printf("Pellet %d was eaten at column %d!\n", pthread_self(), pelletRow);
    else
        printf("Pellet %d wasn't eaten and left stream at column %d\n", pthread_self(), pelletRow);
    sem_post(semaphore);
    
    
    // Append pellet results to file
    //sem_wait(semaphore);
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
    //sem_post(semaphore);
    return NULL;
}

// Handler function for receiving an Interrupt (^C).
void OnInterruptSignal()
{
    // Wait for all children to die to move on.
    wait(NULL);
    CloseSemaphore();
    DetachSharedMemory();
    printf("PID %d (Pellet) killed because of interrupt\n", getpid());
    exit(0);
}

// Handler function for receiving the end signal.
void OnEndSignal()
{
    // Wait for all children to die to move on.
    wait(NULL);
    CloseSemaphore();
    DetachSharedMemory();
    printf("PID %d (Pellet) killed because we have finished\n", getpid());
    exit(0);
}
