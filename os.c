// A Multithreaded Program that implements the banker's algorithm.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
int nResources, nProcesses;
int *resources;
int **allocated;
int **maxRequired;
int **need;
int *safe_sequence;
int nProcessRan = 0;

pthread_mutex_t lockResources;
pthread_cond_t condition;

// get safe sequence is there is one else return false
bool get_safe_sequence();

// final output after decision
void* display_result(void*);

int main(int argc, char** argv) {
	// iterators
	int i, j;
	
	// Get number of processes
	do {
		printf("\nEnter the number of processes: ");
		scanf("%d", &nProcesses);
		// can't have 0 or negative processes
		// More than 5 becomes too many
		if (nProcesses < 1 || nProcesses > 5) {
			printf("Range for number of processes: 1 - 5.\n");
		}
	} while (nProcesses < 1 || nProcesses > 5); // stay realistic
	
	
	// Get number of resources
	do {
		printf("\nEnter the number of resources: ");
		scanf("%d", &nResources);
		// can't have 0 or negative resources
		// More than 5 becomes too many
		if (nResources < 1 || nResources > 5) {
			printf("Range for number of resources: 1 - 5.\n");
		}
	} while (nResources < 1 || nResources > 5); // stay realistic
	
	
	// The next bits of code deal with memory assignment
	
	resources = (int*)malloc(nResources * sizeof(*resources));
	if (!resources) {
		printf("Failed to allocate memory to resources, exiting program...\n");
		exit(-1);
	}
	printf("\nEnter currently Available resources: ");
	for(i = 0; i < nResources; ++i) {
		scanf("%d", &resources[i]);
	}
	allocated = (int**)malloc(nProcesses * sizeof(*allocated));
	if (!allocated) {
		printf("Failed to allocate memory to allocated, exiting program...\n");
		exit(-1);
	}
	for(i = 0; i < nProcesses; ++i) {
		allocated[i] = (int*)malloc(nResources * sizeof(**allocated));
		if (!allocated[i]) {
		printf("Failed to allocate memory to allocated[%d], exiting program...\n", i);
		exit(-1);
		}
	}
	maxRequired = (int**)malloc(nProcesses * sizeof(*maxRequired));
	if (!maxRequired) {
		printf("Failed to allocate memory to maxRequired, exiting program...\n");
		exit(-1);
	}
	for(i = 0; i < nProcesses; ++i) {
		maxRequired[i] = (int*)malloc(nResources * sizeof(**maxRequired));
		if (!maxRequired[i]) {
			printf("Failed to allocate memory to maxRequired[%d], exiting program...\n", i);
		exit(-1);
		}
	}
	
	// allocated
	
	printf("\n");
	for(i = 0; i < nProcesses; ++i) {
		printf("\nHow much resource has to be allocated to process %d: ", i + 1);
		for(j = 0; j < nResources; ++j) {
			scanf("%d", &allocated[i][j]);
		}
	}
	printf("\n");
	
	// maximum required resources
	
	for(i = 0; i < nProcesses; ++i) {
		printf("\nWhat is the maximum resource required by process %d: ", i + 1);
			for(j = 0; j < nResources; ++j) {
				scanf("%d", &maxRequired[i][j]);
			}
	}
	printf("\n");
	
	// calculate need matrix
	
	need = (int**)malloc(nProcesses * sizeof(*need));
	if (!need) {
		printf("Failed to allocate memory to need, exiting program...\n");
		exit(-1);
	}
	for(i = 0; i < nProcesses; ++i) {
		need[i] = (int*)malloc(nResources * sizeof(**need));
		if (!need[i]) {
			printf("Failed to allocate memory to need[%d], exiting program...\n",i);
			exit(-1);
		}
	}
	for(i = 0; i < nProcesses; ++i) {
		for(j = 0; j < nResources; ++j) {
			need[i][j] = maxRequired[i][j] - allocated[i][j];
		}
	}
	safe_sequence = (int*)malloc(nProcesses * sizeof(*safe_sequence));
	if (!safe_sequence) {
		printf("Failed to allocate memory to safe_sequence, exiting program...\n");
		exit(-1);
	}
	
	// set default values
	
	for(i = 0; i < nProcesses; ++i) {
		safe_sequence[i] = -1;
	}
	
	// attempt to get a safe sequence
	
	if(!get_safe_sequence()) {
		printf("\nUnsafe! The processes leads the system to a unsafe state.\n\n");
		exit(-1);
	}
	printf("\n\nSafe Sequence Found: ");
	for(i = 0; i < nProcesses; ++i) {
		printf("%-3d", safe_sequence[i] + 1);
	}
	printf("\nExecuting Processes...\n\n");
	sleep(1);
	
	// run threads
	
	pthread_t processes[nProcesses];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int processNumber[nProcesses];
	for(i = 0; i < nProcesses; ++i) {
		processNumber[i] = i;
	}
	for(i = 0; i < nProcesses; ++i) {
		pthread_create(&processes[i], &attr, display_result,(void*)(&processNumber[i]));
	}
	for(i = 0; i < nProcesses; ++i) {
		pthread_join(processes[i], NULL);
	}
	printf("\nAll processes have finished executing.\n");// free resources
	printf("\nFreeing assigned memory.\n");
	free(resources);
	for(i = 0; i < nProcesses; ++i) {
		free(allocated[i]);
		free(maxRequired[i]);
		free(need[i]);
	}
	free(allocated);
	free(maxRequired);
	free(need);
	free(safe_sequence);
	// end
}
bool get_safe_sequence() {
	
	// iterators
	
	int i, j, k;
	
	// get safe sequence
	
	int tempRes[nResources];
	for(i = 0; i < nResources; ++i) {
		tempRes[i] = resources[i];
	}
	bool finished[nProcesses];
	for(i = 0; i < nProcesses; ++i) {
		finished[i] = false;
	}
	int nfinished = 0;
	while(nfinished < nProcesses) {
		bool safe = false;
		for(i = 0; i < nProcesses; ++i) {
			if(!finished[i]) {
				bool possible = true;
				for(j = 0; j < nResources; ++j) {
					
					// Required resources not available
						
						if(need[i][j] > tempRes[j]) {
							possible = false;
							break;
						}
				}
				if(possible) {
					for(j = 0; j < nResources; ++j) {
					tempRes[j] += allocated[i][j];
				}
				safe_sequence[nfinished] = i;
				finished[i] = true;
				++nfinished;
				
				// All must be safe to get a safe sequence
				
				safe = true;
				}
			}
		}
		if(!safe) {
			for(k= 0; k < nProcesses; ++k) {
				safe_sequence[k] = -1;
			}
			
			// no safe sequence found
			
			return false;
		}
	}
	
	// safe sequence found
	
	return true;
}

// process code
void* display_result(void *arg) {
	
	// iterator
	int i;
	int p = *((int*)arg);
	
	// lock resources
	// we should not allow access to prevent mismatch
	
	pthread_mutex_lock(&lockResources);
	
	// condition check
	
	while(p != safe_sequence[nProcessRan])
		pthread_cond_wait(&condition, &lockResources);
		
		// process
	
	printf("\nNow running Process %d....", p + 1);
	printf("\n\tAllocated: ");
	for(i = 0; i < nResources; ++i){
		printf("%3d", allocated[p][i]);
	}
	printf("\n\tNeeded: ");
	for(i = 0; i < nResources; ++i) {
		printf("%3d", need[p][i]);
	}
	printf("\n\tAvailable: ");
	for(i = 0; i < nResources; ++i) {
		printf("%3d", resources[i]);
	}
	
	// Sleep for aesthetics
	
	printf("\n");
	sleep(1);
	printf("\tResource Allocated!");
	printf("\n"); sleep(1);
	printf("\tProcess Code Running...");
	printf("\n"); sleep(rand()%3 + 2); // process code
	printf("\tProcess Code Completed...");
	printf("\n"); sleep(1);
	printf("\tProcess Releasing Resource...");
	printf("\n"); sleep(1);
	printf("\tResource Released!");
	for(i = 0; i < nResources; ++i) {
		resources[i] += allocated[p][i];
	}
	printf("\n\tNow Available: ");
	for(i = 0; i < nResources; ++i) {
		printf("%3d", resources[i]);
	}
	printf("\n\n");
	
	// Sleep for aesthetics
	
	sleep(1);
	nProcessRan++;
	pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&lockResources);
	pthread_exit(NULL);
}