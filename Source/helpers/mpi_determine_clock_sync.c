#include <stdio.h>
#include "mpi.h"

int main(int argc, char **argv) {

	int rank, size;

	int clocksAreSynced = 0; // default we assume they are not synced
	int gotResult = 0;	

	// MPI Scope begins
	MPI_Init(&argc, &argv);
	
	// read basic info
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// self declaration 
	printf("\nI am process %d of %d\n", rank, size);
	fflush(stdout); // works fine on my machine without this... but may need this on Monash machine..

	// Check if clocks are synced
	MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_WTIME_IS_GLOBAL, &clocksAreSynced, &gotResult);
	if (gotResult) {
		if (clocksAreSynced) {
			printf("\nClocks are synced. Message being printed from process rank %d\n", rank);
			fflush(stdout);
		} else {
			printf("\nClocks are not synced. Message being printed from process rank %d\n", rank);
			fflush(stdout);
		}
	} else {
		printf("\nCould not get result for MPI_WTIME_IS_GLOBAL\n");
		fflush(stdout);
	}
	
	// I am sooo done with MPI..
	MPI_Finalize();

	// and this program..
	return 0;
}
