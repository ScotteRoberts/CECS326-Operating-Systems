//
//  FISH.c
//  SWIM_MILL_NEW
//
//  Created by Scott Roberts on 9/29/17.
//  Copyright © 2017 Scott Roberts. All rights reserved.
//
#include "gvariables.h"

// Fish Variables
int fishColumn = ROWS/2;

// Fish Functions
int * LocatePellets(int);
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
    OpenSemaphore();
    
    while(1)
    {
        // Start from bottom and go to top, checking all rows for pellets
        for(int row = (ROWS-2); row >= 0; row--)
        { 
            // Find the pellets for the given row.
            int* pelletLocations = LocatePellets(row);

            // If the row just above you has a pellet that is directly
            // in front of you, then don't move for a turn.
            if(row == (ROWS-2) && pelletLocations[fishColumn] == 1)
            {
                sleep(1);
                break;
            }
            // Else, there isn't a pellet directly above the fish.
            else
            {
                // Zero out the closest pellet to assign later.
                int closestPellet = -1;
                
                for(int col = 0; col < COLUMNS; col++)
                {
                    if(pelletLocations[col] == 1)
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
                        MoveFish(1);
                    }
                    // Move right if the Closest pellet is in a column to your right.
                    else
                    {
                        MoveFish(-1);
                    }
                        
                    
                    // Wait for the next turn to look again
                    sleep(1);
                    break;
                }
            }
        }
    }
}

int* LocatePellets(int currentRow) {
    int pelletLocations[10] = {0};
    int range, start, end;
    
    /* For a given row, the fish can only reach +/-
     ((totalRows - 1) - currentRow) spaces away from it */
    range = ((ROWS - 1) - currentRow);
    
    /* If the range to the left of the fish is out
     out of bounds, just set the min left range to 0 */
    start = currentRow - range;
    if(start < 0) start = 0;
    
    /* If the range to the right of the fish is out
     out of bounds, just set the min left range to last col */
    end = currentRow + range;
    if(end >= COLUMNS) end = (COLUMNS - 1);
    
    // Start from min reachable range (left) from fish
    for(int i = start; i < fishColumn; i++) {
        if((*grid)[currentRow][i] == 'P')
            pelletLocations[i] = 1;
    }
    
    // Start from fish position to max range (right)
    for(int i = fishColumn; i <= end; i++) {
        if((*grid)[currentRow][i] == 'P')
            pelletLocations[i] = 1;
    }
    
    /* Array will be all 0's, except where a pellet exists
     and the fish can reach it, then that index will be a 1 */
    return pelletLocations;
}

// The Fish will move left if the direction is negative and
// right if the direcion is positive.
void MoveFish(int direction)
{
    // Previous position needs updating similar to a linked list.
    sem_wait(semaphore);
    (*grid)[ROWS-1][fishColumn] = '~';
    fishColumn += direction;
    (*grid)[ROWS-1][fishColumn] = 'F';
    sem_post(semaphore);
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
