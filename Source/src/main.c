/*
    FIT5139 - Assignment Part B - Option 2 - Distributed Event Modelling with MPI
    Author : Siddharth Bhatt
    Email : sbha0005@student.monash.edu
 */
#include <stdio.h> // for basic io..
#include <stdlib.h> // for generating random number
#include <time.h> // for reading time, clock ticks etc..
#include "mpi.h" // because I hate my life..

// A vessel can go to maximum 1390 locations..
const int MINIMUM_ALLOWED_LOCATION = 1;
const int MAXIMUM_ALLOWED_LOCATION = 1390;

// Outer loop will run for one minute (60s)
const double ONE_MINUTE = 60.0;
    
// Sampling interval is 6ms
const double SAMPLING_INTERVAL = 0.006; // 6ms = 0.006

// Global switch for toggling debug info
const int SHOW_DEBUG_INFO = 0; // false by default

// Only root will be touching these vars
double totalStrikes = 0;
double totalSamples = 0;

int main(int argc, char ** argv) {
	
    // Basic building blocks..
    int rank, size;
    
    // We are going to run the outer loop for one minute.
    // There will be sampling intervals in between , in which the we will collect the locations from the vessels
    // And count strikes ..
    
    // struct timespec holds {seconds, nanoseconds}
    struct timespec programStartTime = {0,0}; // This will hold program start time..
    struct timespec programCurrentTime = {0,0}; // This we will use to check against start time..
    
    // Get program start time
    clock_gettime(CLOCK_MONOTONIC, &programStartTime);
    
    // Begin MPI Scope
    MPI_Init(&argc, &argv);
    
    // Get self-identity
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    // Get the size of comm
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Needed while receiving ..
    MPI_Status status;
    
    // This will be maintained at the begining of every loop iteration
    double timer = 0.0;

    // Run the loop for one minute
    while (timer < ONE_MINUTE) {

        // Get current time
        clock_gettime(CLOCK_MONOTONIC, &programCurrentTime);
        
        // Extract seconds out of programCurrentTime and update/sync timer
        // Update and sync timer value
        timer = ((double)programCurrentTime.tv_sec + programCurrentTime.tv_nsec / 1.0e9) - ((double)programStartTime.tv_sec + programStartTime.tv_nsec / 1.0e9);
        MPI_Bcast(&timer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        // Run the inner loop for SAMPLING_INTERVAL time..
        struct timespec loopStartTime = {0,0}; // This will hold loop start time..
        struct timespec loopCurrentTime = {0,0}; // This we will use to check against start time..
        
        // Get loop start time
        clock_gettime(CLOCK_MONOTONIC, &loopStartTime);
        
        // Initialize loop timer with 0.0
        double loopTimer = 0.0; // This will be maintained inside the inner loop
        
        // Run loop for interval
        while (loopTimer < SAMPLING_INTERVAL) {
            
            if (rank == 0) {
                
                // Update sample count
                totalSamples += 1;
                
                if (SHOW_DEBUG_INFO) {
                    printf("\n=======================================================\n");
                    printf("\nStarting sample # %.0f\n", totalSamples);
                    printf("\n=======================================================\n");
                    fflush(stdout);
                }
            }
            
            // Get current time
            clock_gettime(CLOCK_MONOTONIC, &loopCurrentTime);
            
            // Extract seconds out of programCurrentTime and update/sync timer
            // Update and sync timer value
            loopTimer = ((double)loopCurrentTime.tv_sec + loopCurrentTime.tv_nsec / 1.0e9) - ((double)loopStartTime.tv_sec + loopStartTime.tv_nsec / 1.0e9);
            MPI_Bcast(&loopTimer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            // Seed the random number generator
            // rank+1 to prevent srand(0)
            // (rank+1)*clock() ensures a different value in each iteration for every process..
            srand((rank+1)*clock());
            
            // Pick a location from MINIMUM_ALLOWED_LOCATION to MAXIMUM_ALLOWED_LOCATION
            int location = (rand() % MAXIMUM_ALLOWED_LOCATION ) + MINIMUM_ALLOWED_LOCATION;
            
            if (SHOW_DEBUG_INFO) {
                printf("\nProcess %d of %d picked location : %d\n", rank, size-1, location);
                fflush(stdout);
            }
            
            if (rank != 0) {
                // Send it to root
                MPI_Send(&location, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            } else {
                
                // Declare a local map for calculating strikes..
                struct mapType {
                    struct locationType {
                        int vessel[MAXIMUM_ALLOWED_LOCATION]; // vessels that want to strike. We obviously don't need MAXIMUM_ALLOWED_LOCATION as limit here.. 'size' would do it just fine.. but no comilers allow variable sized array declaration inside a struct. This is 2017 !!! Common people !
                        int lastIndexOfVessel; // To keep track of indexes.. C lang doesn't have a .append() function for arrays
                    } location[MAXIMUM_ALLOWED_LOCATION]; // location 1 will be stored in index 0.. 2 at 1.. 3 at 2.. likewise..
                } map;
                
                // TL;DR :
                // We have a map which has MAXIMUM_ALLOWED_LOCATION # of locations.
                // Each location has a list of vessels that are ready to strike there..
                
                // Explicitly set lastIndexOfVessel = 0 for all locations. For Security purposes.
                for (int locationIndex = 0; locationIndex < MAXIMUM_ALLOWED_LOCATION; locationIndex++) {
                    map.location[locationIndex].lastIndexOfVessel = 0;
                }
                
                // Update map for root
                int lastIndex = map.location[location-1].lastIndexOfVessel++;
                map.location[location-1].vessel[lastIndex] = 0;
                
                // Receive from all processes except ourself !
                for (int process = 0; process <= size-1; process++) {
                    if (process == 0) continue; // Don't try to receive from ourself. Coz we didn't send it in the first place ! LOL
                    
                    // Declare a variable to hold the incoming location
                    int incomingLocation;
                    
                    // Receive
                    MPI_Recv(&incomingLocation, 1, MPI_INT, process, 0, MPI_COMM_WORLD, &status);
                    
                    // Save it in the map
                    int lastIndex = map.location[incomingLocation-1].lastIndexOfVessel++;
                    map.location[incomingLocation-1].vessel[lastIndex] = process;
                }
                
                if (SHOW_DEBUG_INFO) {
                     // Print the map
                    for (int locationIndex = 0; locationIndex < MAXIMUM_ALLOWED_LOCATION; locationIndex++) {
                        if (map.location[locationIndex].lastIndexOfVessel > 0) { // Skip the location if nobody wants to attack here..
                            printf("\n Location %d has vessels : ", locationIndex+1);
                            for (int vesselIndex = 0; vesselIndex < (map.location[locationIndex].lastIndexOfVessel); vesselIndex++) {
                                int vessel = map.location[locationIndex].vessel[vesselIndex];
                                printf("%d ", vessel);
                            }
                        }
                    }
                }
                
                // Verify map and find out # of strikes
                for (int locationIndex = 0; locationIndex < MAXIMUM_ALLOWED_LOCATION; locationIndex++) {
                    // We can ignore this location if nobody wants to attack here..
                    if (map.location[locationIndex].lastIndexOfVessel > 0) {
                        
                        int location = locationIndex + 1;
                        int lastVesselIndex = map.location[locationIndex].lastIndexOfVessel;
                        
                        int oddCount = 0;
                        int evenCount = 0;
                        
                        for (int vesselIndex = 0; vesselIndex < lastVesselIndex; vesselIndex++) {
                            int vessel = map.location[locationIndex].vessel[vesselIndex]; // vessel 6 can be there at vesselIndex 0..
                            // Check if vessel is even or odd
                            if (vessel % 2 == 0) {
                                evenCount += 1;
                            } else {
                                oddCount += 1;
                            }
                        }
                        
                        if ( (oddCount >= 2) || (evenCount >= 2) ) {
                            // We have a strike !
                            totalStrikes += 1;
                            //printf("\nWe have a strike at location %d with %d odd vessels and %d even vessels in sample #%.0f\n", location, oddCount, evenCount, totalSamples);
                            //fflush(stdout);
                        }
                    }
                }
            }
            
        } // Inner loop ends
        
    } // Outer Loop ends

    // Print findings
    if (rank == 0) {
        printf("\n=======================================================\n");
        printf("\nLogs:\n");
        printf("\n=======================================================\n");
        printf("\nTotal Strikes generated : %.0f\n", totalStrikes);
        printf("\nTotal Sample sessions : %.0f\n", totalSamples);
        printf("\nStrike Rate : %f\n\n", totalStrikes/totalSamples);
        fflush(stdout);
    }
    
	// End MPI Scope
    MPI_Finalize();

	// Done
	return 0;
} 
