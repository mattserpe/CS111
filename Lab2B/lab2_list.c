#include "SortedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

////////////////Globals///////////////////////
long threads;
long iterations;
long lists;

//Head pointer for circular, doubly-linked list
SortedList_t *list;
//List elements
SortedListElement_t *element;

char* yieldOpts = "none";
char* opt_sync = "none";

pthread_mutex_t* lock;
int* spinLock;

long lockWaitTime = 0;

//////////////Functions//////////////////////
void unLocking(int offset){
	//Unlock every list
	if(strcmp(opt_sync, "s") == 0)
		__sync_lock_release(spinLock + offset);
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_unlock(lock + offset) != 0){
			fprintf(stderr, "Error with mutex unlock: %s\n", strerror(errno));
			exit(1);
		}
	}
}

void locking(int offset){
	
	//Lock every list		
	if(strcmp(opt_sync, "s") == 0){
		//Setup timing
		struct timespec startTime, endTime;
		
		//Start timing lock
		if(clock_gettime(CLOCK_MONOTONIC, &startTime) != 0){
			fprintf(stderr, "Error getting start time: %s\n", strerror(errno));
			exit(1);
		}
		
		//Acquire spin lock
		while(__sync_lock_test_and_set(spinLock + offset, 1));
		
		//End timing lock
		if(clock_gettime(CLOCK_MONOTONIC, &endTime) != 0){
			fprintf(stderr, "Error getting end time: %s\n", strerror(errno));
			exit(1);
		}
		//Add to total time
		lockWaitTime = lockWaitTime + ((endTime.tv_sec - startTime.tv_sec)*1000000000L) + (endTime.tv_nsec - startTime.tv_nsec);
	}
	else if(strcmp(opt_sync, "m") == 0){
		//Setup timing
		struct timespec startTime, endTime;
		
		//Start timing lock
		if(clock_gettime(CLOCK_MONOTONIC, &startTime) != 0){
			fprintf(stderr, "Error getting start time: %s\n", strerror(errno));
			exit(1);
		}
		
		//Acqure mutex lock
		if(pthread_mutex_lock(lock + offset) != 0){
			fprintf(stderr, "Error with mutex lock: %s\n", strerror(errno));
			exit(1);
		}
		
		//End timing lock
		if(clock_gettime(CLOCK_MONOTONIC, &endTime) != 0){
			fprintf(stderr, "Error getting end time: %s\n", strerror(errno));
			exit(1);
		}
		//Add to total time
		lockWaitTime = lockWaitTime + ((endTime.tv_sec - startTime.tv_sec)*1000000000L) + (endTime.tv_nsec - startTime.tv_nsec);
	}	
}

int hashFunc(const char* kee){
	int hash = 0;
	for(unsigned int j = 0; j < strlen(kee); j++){
		hash = hash + (int)(kee[j]);
	}
	hash = hash % lists;
	return hash;
}

void* eachThread(void* in){
	//Take in pre-allocated, initialized elements
	SortedListElement_t *elemIN = in;
	
	for(long i = 0; i < iterations; i++){
		const char* kee = (elemIN + i)->key;
		int hash = hashFunc(kee);
		locking(hash);
		SortedList_insert(list + hash, &elemIN[i]);
		unLocking(hash);
	}
	
	//Locking
	long length = 0;
	for(int i = 0; i < lists; i++){
		locking(i);
	}
	//Counting
	for(int i = 0; i < lists; i++){
		length = length + SortedList_length(list + i);
	}
	//Ensure all elements inserted
	if(length < iterations){
		fprintf(stderr, "Error during element insertion. Length: %ld Iterations: %ld\n", length, iterations);
		exit(2);
	}
	//Unlocking
	for(int i = 0; i < lists; i++){
		unLocking(i);
	}
	
	
	
	//Lookup and Delete
	SortedListElement_t *lookup;
	for(long i = 0; i < iterations; i++){
		const char* kee = (elemIN + i)->key;
		int hash = hashFunc(kee);
		locking(hash);
		//Lookup
		lookup = SortedList_lookup(list + hash, (elemIN + i)->key);
		if(lookup == NULL){
			fprintf(stderr, "Error during element lookup\n");
			exit(2);
		}
		//Delete
		if(SortedList_delete(lookup) != 0){
			fprintf(stderr, "Error during element deletion\n");
			exit(2);
		}
		unLocking(hash);
	}
	return in;
}

char* randomString(char* string){
	//Define alphabet
	const char* alphabet = "abcdefghijklmnopqrstuvwxyz";
	
	//Create string one char at a time
	//choosing random chars from alphabet
	for(int i = 0; i < 255; i++){
		char temp = alphabet[rand() % 26];
		string[i] = temp;
		//strncat(string, &temp, 1);
	}
	
	//String termination character
	string[255] = '\0';
	
	return string;
}

void printCSV(struct timespec startTime, struct timespec endTime){
	long operations = threads*iterations*3;
	long runTime = ((endTime.tv_sec - startTime.tv_sec)*1000000000L) + (endTime.tv_nsec - startTime.tv_nsec);
	long avgTimePerOp = runTime/operations;
	long waitForLock = lockWaitTime/operations;
	//long throughput = 1000000000/avgTimePerOp;
	fprintf(stdout, "list-%s-%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", yieldOpts, opt_sync, threads, iterations, lists, operations, runTime, avgTimePerOp, waitForLock);
}

void listInit(){
	//Allocate memmory for head pointers
	list = malloc(sizeof(SortedList_t)*lists);
	if(list == NULL){
		fprintf(stderr, "Error allocating memmory for list heads: %s\n", strerror(errno));
		exit(1);
	}
	
	//Allocate memmory for list elements
	element = malloc(threads*iterations*sizeof(SortedListElement_t));
	if(element == NULL){
		fprintf(stderr, "Error allocating memmory for list elements: %s\n", strerror(errno));
		exit(1);
	}
	
	//Initialize head pointers
	for(int i = 0; i < lists; i++){
		list[i].key = NULL;
		list[i].prev = &list[i];
		list[i].next = &list[i];
	}
	
	//Seed random number generator
	srand(time(NULL));
	
	char* string;
	
	//Assign keys, initialize list elements
	for(int i = 0; i < threads*iterations; i++){
		//Allocate 256 bit string
		string = malloc(256 * sizeof(char));
		
		//Create random string and assign to key
		element[i].key = randomString(string);
		element[i].prev = NULL;
		element[i].next = NULL;
	}
	
	free(string);
}

void lockInit(){
	if(strcmp(opt_sync, "s") == 0){		//Spin lock
		//Allocate
		spinLock = malloc(lists*sizeof(int));
		if(spinLock == NULL){
			fprintf(stderr, "Error allocating memmory for spin locks: %s\n", strerror(errno));
			exit(1);
		}
		//Initialize
		for(int i = 0; i < lists; i++)
			spinLock[i] = 0;
	}
	else if(strcmp(opt_sync, "m") == 0){ 	//Mutex lock
		//Allocate
		lock = malloc(lists*sizeof(pthread_mutex_t));
		if(lock == NULL){
			fprintf(stderr, "Error allocating memmory for mutex locks: %s\n", strerror(errno));
			exit(1);
		}
		//Initialize
		for(int i = 0; i < lists; i++){
			if(pthread_mutex_init(&lock[i], NULL) != 0){
				fprintf(stderr, "Error initializing mutex locks: %s\n", strerror(errno));
				exit(1);
			}
		}
	}
}

void segHandler(){
	fprintf(stderr, "Segmentation Fault!\n");
	exit(2);
}

//////////////////Main//////////////////////
int main(int argc, char* argv[]){
	int option_index = 0;
	opt_yield = 0;
	
	//Segmentation Fault Handler
	signal(SIGSEGV, segHandler);
	
	//Option Struct
	static struct option long_options[] = {
		{"threads", required_argument, NULL, 't'},
		{"iterations", required_argument, NULL, 'i'},
		{"lists", required_argument, NULL, 'l'},
		{"sync", required_argument, NULL, 'z'},
		{"yield", required_argument, NULL, 'y'},
		{0, 0, 0, 0}
	};
	
	//Set defaults to 1
	threads = 1;
	iterations = 1;
	lists = 1;

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
			//Lists option
			case 'l':
				lists = atoi(optarg);
				break;
			//Yield option
			case 'y':
				yieldOpts = optarg;
				switch(optarg[0]){
					case 'i':
						opt_yield = INSERT_YIELD;
						if(strcmp(optarg, "id") == 0)
							opt_yield = INSERT_YIELD | DELETE_YIELD;
						else if(strcmp(optarg, "il") == 0)
							opt_yield = INSERT_YIELD | LOOKUP_YIELD;
						else if(strcmp(optarg, "idl") == 0)
							opt_yield = INSERT_YIELD | DELETE_YIELD | LOOKUP_YIELD; 
						break;
					case 'd':
						opt_yield = DELETE_YIELD;
						if(strcmp(optarg, "dl") == 0)
							opt_yield = DELETE_YIELD | LOOKUP_YIELD;
						break;
					case 'l':
						opt_yield = LOOKUP_YIELD;
						break;
					default:
						opt_yield = 0;
				}
				break;
			//Sync option
			case 'z':
				opt_sync = optarg;
				break;
			//Undefined Option
			default:
				fprintf(stderr, "Unrecognized Option. Accepted options: --threads=# --lists=# --iterations=# --yield=[idl] --sync=[ms] \n");
				exit(1);
				break;
		}
	}
	
	//Allocate and initialize locks if necessary
	lockInit();
	
	//Allocate and intialize lists
	listInit();
	
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
		if(pthread_create(&ID[i], NULL, eachThread, (void *) (element + (i*iterations))) != 0){
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
	
	//Check length of lists to confirm they're all 0
	long totList = 0;
	for(int i = 0; i < lists; i++){
		totList = totList + SortedList_length(list + i);
	}
	
	if(totList != 0){
		fprintf(stderr, "Non-Zero list length\n");
		exit(2);
	}
	
	//Print CSV to stdout
	printCSV(startTime, endTime);
	
	//Destroy locks if necessary
	if(strcmp(opt_sync, "s") == 0)
		free(spinLock);
	else if(strcmp(opt_sync, "m") == 0)
		for(int i = 0; i < lists; i++){
			pthread_mutex_destroy(lock + i);
		}
	
	//Free and exit
	free(list);
	free(element);
	free(ID);
	exit(0);
	
}