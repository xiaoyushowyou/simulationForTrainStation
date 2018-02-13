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
#define arrivalHigh 60
#define arrivalLow 30
#define waitHigh 15
#define waitLow 8

// Flag set to 1 to print debugging statements (event trace), 0 otherwise
#define DB	1

// number of arrivals for high speed and low speed trains to be simulated (used to determine length of simulation run)
#define	NARRIVALSL	10
#define NARRIVALSH  10

// State Variables of Simulation
int freeSharePlat = 1; // boolearn: 1 if shared platform is free, 0 otherwise
int freeSpecialPlat = 1; // boolearn: 1 if shared platform is free, 0 otherwise



int	InTheAir=0;	    // # aircraft using runway or waiting to use it
int	OnTheGround=0;	// # aircraft parked at store
int	RunwayFree=1;	// boolean: 1 if runway is free, 0 otherwise
int	ArrivalCount=0;	// number of arrivals simulated; used for termination

// State variables used for statistics
double	TotalWaitingTime = 0.0;	// total time waiting to land
double	LastEventTime = 0.0;	// time of last event processed; used to compute TotalWaitingTime

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Data structures for events
//
/////////////////////////////////////////////////////////////////////////////////////////////

// types of events
typedef enum {ARRIVAL, DEPARTURE, LANDED} KindsOfEvents;

// Event parameters
// No event parameters really needed in this simple simulation
struct EventData {
	KindsOfEvents EventType;
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
/////////////////////////////////////////////////////////////////////////////////////////////

// prototypes for event handlers
void Arrival (struct EventData *e);		// aircraft arrival event
void Departure (struct EventData *e);	// aircraft departure event
void Landed (struct EventData *e);		// landed event

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
void Arrival (struct EventData *e)
{
	struct EventData *d;
	double ts;

	if (e->EventType != ARRIVAL) {fprintf (stderr, "Unexpected event type\n"); exit(1);}
	if (DB) printf ("Arrival Event: time=%f\n", CurrentTime());

	// update waiting time statistics
	if (InTheAir > 1) {	// if there are waiting aircraft, update total waiting time
		TotalWaitingTime += ((InTheAir-1) * (CurrentTime()-LastEventTime));
		}

	// update other statistics
	NEvents++;		    // event count

	InTheAir++;

	// schedule next arrival event if this is not the last arrival
	ArrivalCount++;
	if (ArrivalCount < NARRIVALS) {
		if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = ARRIVAL;
		ts = CurrentTime() + RandExp(A);
		Schedule (ts, d, (void *) Arrival);
	}

	if (RunwayFree) {
		RunwayFree = 0;				// runway no longer free
		// schedule landed event
		if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = LANDED;
		ts = CurrentTime() + R;
		Schedule (ts, d, (void *) Landed);
	}

	LastEventTime = CurrentTime();		// time of last event processed
	free (e);				// free storage for event parameters
}

// event handler for landed events
void Landed (struct EventData *e)
{
	struct EventData *d;
	double ts;

	if (e->EventType != LANDED) {fprintf (stderr, "Unexpected event type\n"); exit(1);}
	if (DB) printf ("Landed Event: time=%f\n", CurrentTime());

	// update waiting time statistics
	if (InTheAir > 1) {	// if there are waiting aircraft, update total waiting time
		TotalWaitingTime += ((InTheAir-1) * (CurrentTime()-LastEventTime));
		}

	NEvents++;		// event count
	InTheAir--;
	OnTheGround++;
	// schedule departure event
	if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
	d->EventType = DEPARTURE;
	ts = CurrentTime() + G;
	Schedule (ts, d, (void *) Departure);

	if (InTheAir>0) {
		//  schedule landed event for next aircraft
		if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = LANDED;
		ts = CurrentTime() + R;
		Schedule (ts, d, (void *) Landed);
	}
	else {
		// runway is now free
		RunwayFree = 1;
	}

	LastEventTime = CurrentTime();		// time of last event processed
	free (e);				// event parameters
}


// event handler for departure events
void Departure (struct EventData *e)
{
	if (e->EventType != DEPARTURE) {fprintf (stderr, "Unexpected event type\n"); exit(1);}
	if (DB) printf ("Departure Event: time=%f\n", CurrentTime());

	// update statistics
	if (InTheAir > 1) {	// if there are waiting aircraft, update total waiting time
		TotalWaitingTime += ((InTheAir-1) * (CurrentTime()-LastEventTime));
		}
	NEvents++;		// event count

	OnTheGround--;

	LastEventTime = CurrentTime();		// time of last event processed
	free (e);				// event parameters
}

///////////////////////////////////////////////////////////////////////////////////////
//////////// MAIN PROGRAM
///////////////////////////////////////////////////////////////////////////////////////

int main (void)
{
	struct EventData *d;
	double ts;
	double Duration;

	// initialize event list with first arrival
	if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
	d->EventType = ARRIVAL;
	ts = RandExp(A);
	Schedule (ts, d, (void *) Arrival);

	printf ("Welcome to the Airport Simulation\n");
	StartTime = clock();
	RunSim();
	EndTime = clock();

	// print final statistics
	printf ("Number of aircraft = %d\n", NARRIVALS);
	printf ("Total waiting time = %f\n", TotalWaitingTime);
	printf ("Average waiting time = %f\n", TotalWaitingTime / (double) NARRIVALS);

	Duration = (double) (EndTime-StartTime) / (double) CLOCKS_PER_SEC;
	printf ("%d events executed in %f seconds (%f events per second)\n", NEvents, Duration, (double)NEvents/Duration);
}
