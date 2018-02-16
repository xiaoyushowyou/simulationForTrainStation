// Heap structure, which is used to apply priority queue
// Create by Xiaoyu Yang
// 2/15/2018

#include <stdio.h>
#include <stdlib.h>
#include "trainStation.h"

// Heap structure for an event
struct Event {
    double timestamp;
    void* AppData;
    void (*callback)(void *);
}

// Future Event List
// Use an event structure as the header for the future event list (priority queue)
struct Event FEL ={-1.0, NULL, NULL, NULL};

// Function to print timestamps of events in event list
void PrintList (struct Event *List);

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
size = 0;        //Initial queue size
capacity = 100;  //Maximum setted queue size 

void initQueue {
    Event** heapQueue = (Event**)malloc(capacity*sizeof(Event*));        
}

// Schedule new event in FEL
// queue is implemented as a timestamp ordered heap
void Schedule (double ts, void *data, void (*cb)(void *)) {
// Update queue structure if length is smaller than the content
    if (size>=capacity) {
        capacity = 2*capacity;
        heapQueue = realloc(capacity*sizeof(Event*));
    }
    
    struct Event *e;
    e->timestamp = ts;
    e->AppData = data;
    e->callback = cb;

    arr[size] = e;
    heapifyUp(size);
    size++;
}

// Heapifyup when add new element to the last one
void heapifyUp(int childInd) {
    int parentInd = childInd/2;
    if (lastInd>0 && parentInd->timestamp > childInd->timestamp) {
        struct Event *temp;
        temp = heapQueue[parentInd];
        heapQueue[parentInd] = heapQueue[childInd];
        heapQueue[parentInd] = temp;
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
    heapifyDown(size);
    size--;
    return e;
}

// Heapify down wehn remove the root node. Use the last element to the root and then heapify down
void heapifyDown (void) {
    heapQueue[0] = heapQueue 
}



struct Event *Remove (void)
{
	struct Event *e;

	if (FEL.Next==NULL) return (NULL);
	e = FEL.Next;		// remove first event in list
	FEL.Next = e->Next;
	return (e);
}


void PrintList (struct Event *List)
{
	struct Event *p;

	printf ("Event List: ");
	for (p=List; p!=NULL; p=p->Next) {
		printf ("%f ", p->timestamp);
	}
	printf ("\n");
}


void RunSim (void) 
{
	struct Event *e;

	printf ("Initial event list:\n");
	PrintList (FEL.Next);

	// Main scheduler loop
	while ((e=Remove()) != NULL) {
		Now = e->timestamp;
		e->callback(e->AppData);
		free (e);	// it is up to the event handler to free memory for parameters
	}
}

