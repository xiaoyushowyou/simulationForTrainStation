//
//  main.c
//  heapQueue
//
//  Created by Xiaoyu Yang on 2/16/18.
//  Copyright © 2018 Xiaoyu Yang. All rights reserved.
//

// Heap structure, which is used to apply priority queue
// Create by Xiaoyu Yang
// 2/15/2018

#include <stdio.h>
#include <stdlib.h>
#include "trainStation.h"
#include <time.h>

// Heap structure for an event
struct Event {
    double timestamp;
    void* AppData;
    void (*callback)(void *);
};

// Heapifydown to heap structure when add new element
void heapifyUp(int size);
void heapifyDown (int parentInd);

// Future Event List
// Use an event structure as the header for the future event list (priority queue)
// struct Event  ={-1.0, NULL, NULL, NULL};

// Function to print timestamps of events in event list
void PrintList ();

// Function to remove smallest timestamped event
struct Event *Remove (void);

// Simulation clock variable
double Now = 0.0;

// Return current simulation time
double CurrentTime (void)
{
    return (Now);
}

// Initial queue structure
int size = 0;        //Initial queue size
int capacity = 1000;  //Maximum setted queue size
struct Event** heapQueue;

void initQueue(){
    if ((heapQueue = malloc(capacity*sizeof(struct Event*))) == NULL)
    {
        fprintf(stderr, "malloc error\n");
        exit(1);
    }
}

// Schedule new event in FEL
// queue is implemented as a timestamp ordered heap
void Schedule (double ts, void *data, void (*cb)(void *)) {
    // Update queue structure if length is smaller than the content
    if (size>=capacity-1) {
        capacity = 2*capacity;
        if ((heapQueue = realloc(heapQueue, capacity*sizeof(struct Event*))) == NULL)
        {
            fprintf(stderr, "malloc error\n");
            exit(1);
        }
    }
    
    struct Event* e;
    if ((e = malloc(sizeof(struct Event))) == NULL)
    {
        fprintf(stderr, "malloc error\n");
        exit(1);
    }
    
    e->timestamp = ts;
    e->AppData=data;
    e->callback=cb;
    heapQueue[size] = e;
    heapifyUp(size);
    size++;
}

// Heapifyup when add new element to the last one
void heapifyUp(int childInd) {
    int parentInd = (childInd-1)/2;
    if (parentInd>=0 && heapQueue[parentInd]->timestamp > heapQueue[childInd]->timestamp) {
        struct Event *temp;
        temp = heapQueue[parentInd];
        heapQueue[parentInd] = heapQueue[childInd];
        heapQueue[childInd] = temp;
        heapifyUp(parentInd);
    }
}
    
    
// Remove smallest timestamped event from FEL, return pointer to this event
// return NULL if FEL is empty
struct Event *Remove (void) {
    struct Event *e;
    if (size == 0) {
        return NULL;
    }
    e = heapQueue[0];
    heapQueue[0] = heapQueue[size-1];
    heapifyDown(0);
    size--;
    return e;
}
    
// Heapify down when remove the root node. Use the last element to the root and then heapify down
void heapifyDown (int parentInd) {
    int lInd =  parentInd*2+1;
    int rInd = parentInd*2+2;
    int minInd = parentInd;
    if (lInd<size && heapQueue[parentInd]->timestamp > heapQueue[lInd]->timestamp) {
        minInd  = lInd;
    }
    
    if (rInd<size && heapQueue[minInd]->timestamp > heapQueue[rInd]->timestamp) {
        minInd = rInd;
    }
    
    if (minInd != parentInd) {
        struct Event *temp;
        temp = heapQueue[parentInd];
        heapQueue[parentInd] = heapQueue[minInd];
        heapQueue[minInd] = temp;
        heapifyDown(minInd);
    }
}

    
void PrintList ()
{
    printf ("Event List: \n");
    for (int i = 0; i < size; i++) {
        printf("%f", heapQueue[i]->timestamp);
        printf ("\n");
    }
}
        
//Main function, used to test
// int main(){
//     initQueue();
//     srand(time(NULL));
//     double r;
//     for(int i=0; i<50; i++){
//         r=rand();
//         Schedule(r, NULL, NULL);
//         printf("%f, \n", r);
//     }
    
//     PrintList();
    
//     printf("Ordered time step, %d \n", size);
//     while(size>0) {
//         struct Event* e;
//         e = Remove();
//         printf("%f \n", e->timestamp);
//     }
// }


void RunSim (void)
{
    struct Event *e;
	printf ("Initial event list:\n");
    // Main scheduler loop
    while ((e=Remove()) != NULL) {
        Now = e->timestamp;
        e->callback(e->AppData);
        free (e);    // it is up to the event handler to free memory for parameters
    }
}
        

