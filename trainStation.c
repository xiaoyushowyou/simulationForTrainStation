#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "trainStation.h"

// Execution time statistics (for measuring runtime of program); unrelated to sim model
clock_t StartTime, EndTime;	// start and end time of simulation run
int NEvents=0;			    // number of events executed

/////////////////////////////////////////////////////////////////////////////////////////////
//
// State variables  and other global information
//
/////////////////////////////////////////////////////////////////////////////////////////////

// Simulation constants; all times in minutes
// arrivalHigh = mean interarrival time for high speed trains
// arrivalLow = mean ineterarrival time for low speed trains
// waitHigh = time of high speed train when using platform
// waitLow = time of low speed train when using platform
#define arrivalHigh 20
#define arrivalLow 50
#define waitHigh 8
#define waitLow 30

// Flag set to 1 to print debugging statements (event trace), 0 otherwise
#define DB	1

// number of arrivals for high speed and low speed trains to be simulated (used to determine length of simulation run)
#define	NARRIVALSL	16
#define NARRIVALSH  12

// State Variables of Simulation
int freeSharePlat = 1; // boolearn: 1 if shared platform is free, 0 otherwise
int freeSpecialPlat = 1; // boolearn: 1 if shared platform is free, 0 otherwise
int numWaitHigh = 0; // number of high speed train using platform or waiting to use it
int numWaitLow = 0; // number of low speed train using platfor or waiting to use it
int numTotalHigh = 0; // number of total high speed train simulated, used for termination
int numTotalLow = 0; // number of total low speed train simulated, used for termination

// State variables used for statistics
double waitTimeH = 0.0;  //total waiting time for high speed trians
double waitTimeL = 0.0;  //total waiting time for low speed trains
double	TotalWaitingTime = 0.0;	// total time waiting to land
double	LastEventTime = 0.0;	// time of last event processed; used to compute TotalWaitingTime


/////////////////////////////////////////////////////////////////////////////////////////////
//
// Data structures for events
//
/////////////////////////////////////////////////////////////////////////////////////////////

// types of events
typedef enum {ARRIVAL, DEPARTURE} KindsOfEvents;
typedef enum {HIGH, LOW} KindsOfTrains;
typedef enum {SHARE, SPEC} KindsOfPlat;

// Event parameters
struct EventData {
	KindsOfEvents EventType;
	KindsOfTrains TrainType;
	KindsOfPlat PlatFormType;
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
/////////////////////////////////////////////////////////////////////////////////////////////

// prototypes for event handlers
void Arrival (struct EventData *e);		// train arrival event
void Departure (struct EventData *e);	// train departure event

// prototypes for other procedures
double RandExp(double M);			// random variable, exponential distribution

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Random Number generator
//
/////////////////////////////////////////////////////////////////////////////////////////////

// Compute exponenitally distributed random number with mean M
double RandExp(double M)
{
	double urand;	// uniformly distributed random number [0,1)
	urand = ((double) rand ()) / (((double) RAND_MAX)+1.0);	// avoid value 1.0
	return (-M * log(1.0-urand));
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Event Handlers
// Parameter is a pointer to the data portion of the event
//
/////////////////////////////////////////////////////////////////////////////////////////////

// event handler for arrival events
void Arrival (struct EventData *e) {
	struct EventData *d;
	double ts;
	if (e->EventType != ARRIVAL) {fprintf(stderr, "unexpected event type\n"); exit(1);}
    char* myTrain = malloc(sizeof(char*));
    
    if (e->TrainType==HIGH) {
        myTrain = "High Speed Train";
    }
    else {
        myTrain = "Low Speed Train";
    }
    
    if (DB) printf ("Arrival Event for %s: time=%f\n", myTrain ,CurrentTime());
    
	//update waiting time statistics
	if (numWaitHigh>0) {
		waitTimeH += (numWaitHigh*(CurrentTime()-LastEventTime));
	}
    
	if (numWaitLow>0) 
	{
		waitTimeL += (numWaitLow*(CurrentTime()-LastEventTime));
	}
	TotalWaitingTime = waitTimeH+waitTimeL;

	// update other statistics
	NEvents++;		    // event count

	// schedule next arrival event if this is not the last arrival
	if (e->TrainType==HIGH && numTotalHigh <= NARRIVALSH) {
		numWaitHigh++;
        numTotalHigh++;
		if ((d = malloc(sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = ARRIVAL;
		d->TrainType = HIGH;
		d->PlatFormType = SHARE;
		ts = CurrentTime() + RandExp(arrivalHigh);
		Schedule(ts, d, (void*) Arrival);
	}

	if (e->TrainType==LOW && numTotalLow <= NARRIVALSL) {
		numWaitLow++;
        numTotalLow++;
		if ((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = ARRIVAL;
		d->TrainType = LOW;
		d->PlatFormType = SPEC;
		ts = CurrentTime() + RandExp(arrivalLow);
		Schedule(ts, d, (void*) Arrival);
	}
    
    
	// schedule depature event if plantform is available. The train will first load/unload passenger, then departure. 
	if (freeSharePlat && numWaitHigh!=0) {
		if ((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
        freeSharePlat = 0;
        d->EventType = DEPARTURE;
		d->TrainType = HIGH;
		d->PlatFormType = SHARE;
		ts = CurrentTime() + waitHigh;
		Schedule(ts, d, (void*) Departure);
	}

	if (freeSpecialPlat && numWaitLow!=0) {
		if ((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
        freeSpecialPlat = 0;
        d->EventType = DEPARTURE;
		d->TrainType = LOW;
		d->PlatFormType = SPEC;
		ts = CurrentTime() + waitLow;
		Schedule(ts, d, (void*) Departure);
	}
//    else if (!freeSpecialPlat && numWaitLow!=0 && freeSharePlat) {
//        if ((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
//        freeSharePlat = 0;
//        d->EventType = DEPARTURE;
//        d->TrainType = LOW;
//        d->PlatFormType = SHARE;
//        ts = CurrentTime() + waitLow;
//        Schedule(ts, d, (void*) Departure);
//    }

	LastEventTime = CurrentTime();
	free(e);
}



// event handler for depature events, which considers the time spend at platform to load and unload passengers
void Departure (struct EventData *e) {
	struct EventData *d;
	double ts;

	if (e->EventType != DEPARTURE) {fprintf (stderr, "Unexpected event type\n"); exit(1);}
    
    char* myTrain = malloc(sizeof(char*));
    if (e->TrainType==HIGH) {
        myTrain = "High Speed Train";
    }
    else {
        myTrain = "Low Speed Train";
    }
    
	if (DB) printf ("Departure Event for %s: time=%f\n", myTrain, CurrentTime());

	// update waiting time statistics
	if (numWaitHigh>0) {
		waitTimeH += (numWaitHigh*(CurrentTime()-LastEventTime));
	}
    
	if (numWaitLow>0) 
	{
		waitTimeL += (numWaitLow*(CurrentTime()-LastEventTime));
	}
    
	TotalWaitingTime = waitTimeH+waitTimeL;

	// update other statistics
	NEvents++;		    // event count
	if (e->TrainType == HIGH) {
		numWaitHigh--;
	}
	if (e->TrainType == LOW) {
		numWaitLow--;
	}

	// schedule departure event
	if (e->PlatFormType == SHARE) {
		if (numWaitHigh!=0) {
			if ((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
			d->EventType = DEPARTURE;
			d->TrainType = HIGH;
			d->PlatFormType = SHARE;
			ts = CurrentTime() + waitHigh;
			Schedule(ts, d, (void*) Departure);
		}
//        else if (!freeSpecialPlat && numWaitLow!=0) {
//            if ((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
//            d->EventType = DEPARTURE;
//            d->TrainType = LOW;
//            d->PlatFormType = SHARE;
//            ts = CurrentTime() + waitLow;
//            Schedule(ts, d, (void*) Departure);
//        }
		else {
		freeSharePlat = 1;
		}
	}
	

	if (e->PlatFormType == SPEC) {
		if (numWaitLow!=0) {
            if ((d = malloc(sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
			d->EventType = DEPARTURE;
			d->TrainType = LOW;
			d->PlatFormType = SPEC;
			ts = CurrentTime() + waitLow;
			Schedule(ts, d, (void*) Departure);
		}
		else {
			freeSpecialPlat = 1;
		}
	}

	LastEventTime = CurrentTime();
	free(e);
}


///////////////////////////////////////////////////////////////////////////////////////
//////////// MAIN PROGRAM
///////////////////////////////////////////////////////////////////////////////////////
int main (void)
{
    struct EventData *d;
    double ts;
    double Duration;

    // Initialize heap priority queue structure
    initQueue();
    // initialize event list with first arrival
    if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    d->EventType = ARRIVAL;
    d->TrainType = HIGH;
    d->PlatFormType = SHARE;
    ts = RandExp(arrivalHigh);
    Schedule (ts, d, (void *) Arrival);

    if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    d->EventType = ARRIVAL;
    d->TrainType = LOW;
    d->PlatFormType = SPEC;
    ts = RandExp(arrivalLow);
    Schedule (ts, d, (void *) Arrival);

    printf ("Welcome to the Train Station Simulation\n");

    StartTime = clock();
    RunSim();
    EndTime = clock();

    // print final statistics
    printf ("Number of high speed trains = %d\n", NARRIVALSH);
    printf ("Number of low speed trains = %d\n", NARRIVALSL);
    printf ("Total waiting time of high speed trains = %f\n", waitTimeH);
    printf ("Average waiting time of high speed trains = %f\n", (double)waitTimeH/NARRIVALSH);
    printf ("Total waiting time of low speed trains = %f\n", waitTimeL);
    printf ("Average waiting time of low speed trains = %f\n", (double)waitTimeL/NARRIVALSL);
    printf ("Total waiting time = %f\n", TotalWaitingTime);
    printf ("Average waiting time = %f\n", TotalWaitingTime / (double) (NARRIVALSH+NARRIVALSL));

    Duration = (double) (EndTime-StartTime) / (double) CLOCKS_PER_SEC;
    printf ("%d events executed in %f seconds (%f events per second)\n", NEvents, Duration, (double)NEvents/Duration);
}

