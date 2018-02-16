// Simulation Engine Interface
// Start the priority queue structure
void initQueue();

// Schedule an event with timestamp ts, event parameters *data, and callback function cb
void Schedule (double ts, void *data, void (*cb)(void *));

// Call this procedure to run the simulation
void RunSim (void);

// This function returns the current simulation time
double CurrentTime (void);
