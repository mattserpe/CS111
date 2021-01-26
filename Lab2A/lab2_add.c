#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>

////////////////Globals///////////////////////
long threads;
long iterations;
long long counter;

char* testName = "add-none";

int opt_yield = 0;
char* opt_sync = NULL;

pthread_mutex_t lock;
int spinLock = 0;

//////////////Functions//////////////////////
void add(long long *pointer, long long value){
	long long sum = *pointer + value;
	if(opt_yield){
		testName = "add-yield-none";
		sched_yield();
	}
	*pointer = sum;
}

void addM(long long *pointer, long long value){
	//Lock
	if(pthread_mutex_lock(&lock) != 0){
		fprintf(stderr, "Error with mutex lock: %s\n", strerror(errno));
		exit(1);
	}
	long long sum = *pointer + value;
	if(opt_yield){
		testName = "add-yield-m";
		sched_yield();
	}
	*pointer = sum;
	//Unlock
	if(pthread_mutex_unlock(&lock) != 0){
		fprintf(stderr, "Error with mutex unlock: %s\n", strerror(errno));
		exit(1);
	}
}

void addS(long long *pointer, long long value){
	while(__sync_lock_test_and_set(&spinLock,1));	//Spin till I acquire lock
	long long sum = *pointer + value;
	if(opt_yield){
		testName = "add-yield-s";
		sched_yield();
	}
	*pointer = sum;
	__sync_lock_release(&spinLock);		//Release lock after addition
}

void addC(long long *pointer, long long value){
	//__sync_val_compare_and_swap (*ptr, oldval, newval)
	long long oldVal;
	long long newVal;
	do{
		oldVal = counter;					//Indicate value I think is currently at *pointer
		newVal = oldVal + value;			//Indicate value I want to set *pointer to if true
		
		//Yield between new sum computation, compare and swap
		if(opt_yield){
			testName = "add-yield-c";
			sched_yield();
		}
	} while(__sync_val_compare_and_swap (pointer, oldVal, newVal) != oldVal);	//Try the addition until it occurs
}

void* addSub(void* in){
	//No sync
	if(opt_sync == NULL){
		testName = "add-none";
		for(long i = 0; i < iterations; i++){
			add(&counter, 1);
			add(&counter, -1);
		}
	}
	//Sync-m
	else if(strcmp(opt_sync, "m") == 0){
		testName = "add-m";
		for(long i = 0; i < iterations; i++){
			addM(&counter, 1);
			addM(&counter, -1);
		}
	}
	//Sync-s
	else if(strcmp(opt_sync, "s") == 0){
		testName = "add-s";
		for(long i = 0; i < iterations; i++){
			addS(&counter, 1);
			addS(&counter, -1);
		}
	}
	//Sync-c
	else if(strcmp(opt_sync, "c") == 0){
		testName = "add-c";
		for(long i = 0; i < iterations; i++){
			addC(&counter, 1);
			addC(&counter, -1);
		}
	}
	
	return in;
}

void printCSV(struct timespec startTime, struct timespec endTime){
	long operations = threads*iterations*2;
	long runTime = ((endTime.tv_sec - startTime.tv_sec)*1000000000L) + (endTime.tv_nsec - startTime.tv_nsec);
	long avgTimePerOp = runTime/operations;
	fprintf(stdout, "%s,%ld,%ld,%ld,%ld,%ld,%lld\n", testName, threads, iterations, operations, runTime, avgTimePerOp, counter);
}

//////////////////Main//////////////////////
int main(int argc, char* argv[]){
	int option_index = 0;

	//Option Struct
	static struct option long_options[] = {
		{"threads", required_argument, NULL, 't'},
		{"iterations", required_argument, NULL, 'i'},
		{"sync", required_argument, NULL, 'z'},
		{"yield", no_argument, NULL, 'y'},
		{0, 0, 0, 0}
	};
	
	//Set defaults to 1
	threads = 1;
	iterations = 1;

	int option;
	while ((option = getopt_long(argc, argv, "", long_options, &option_index)) >= 0){
		switch (option){
			//Threads option
			case 't':
				threads = atoi(optarg);
				break;
			//Iterations option
			case 'i':
				iterations = atoi(optarg);
				break;
			//Yield option
			case 'y':
				opt_yield = 1;
				break;
			//Sync option
			case 'z':
				opt_sync = optarg;
				break;
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options: --threads=# --iterations=# --yield --sync=[msc] \n");
				exit(1);
				break;
		}
	}
	
	//Initialize counter
	counter = 0;
	
	//Create array to hold thread IDs
	pthread_t *ID;
	ID = (pthread_t*)malloc(sizeof(pthread_t)*threads);
	if(ID == NULL){
		fprintf(stderr, "Error allocating memmory for thread IDs: %s\n", strerror(errno));
		exit(1);
	}
	
	//Get start time
	struct timespec startTime;
	if(clock_gettime(CLOCK_MONOTONIC, &startTime) != 0){
		fprintf(stderr, "Error getting start time: %s\n", strerror(errno));
		exit(1);
	}
	
	//Create threads
	for(int i = 0; i < threads; i++){
		if(pthread_create(&ID[i], NULL, &addSub, NULL) != 0){
			fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
			exit(1);
		}
	}
	
	//Wait for threads to finish
	for(int i = 0; i < threads; i++){
		if(pthread_join(ID[i], NULL) != 0){
			fprintf(stderr, "Error while waiting for thread: %s\n", strerror(errno));
			exit(1);
		}
	}
	
	//Get end time
	struct timespec endTime;
	if(clock_gettime(CLOCK_MONOTONIC, &endTime) != 0){
		fprintf(stderr, "Error getting end time: %s\n", strerror(errno));
		exit(1);
	}
	
	//Print CSV to stdout
	printCSV(startTime, endTime);
	
	//Free and exit
	free(ID);
	exit(0);
}