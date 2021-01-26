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

//Head pointer for circular, doubly-linked list
SortedList_t *list;
//List elements
SortedListElement_t *element;
//char* keys;

char* yieldOpts = "none";
char* opt_sync = "none";

pthread_mutex_t lock;
int spinLock = 0;

//////////////Functions//////////////////////
void* eachThread(void* in){
	//Take in pre-allocated, initialized elements
	SortedListElement_t *elemIN = in;
	
	//Do any required locking
	if(strcmp(opt_sync, "s") == 0)
		while(__sync_lock_test_and_set(&spinLock,1));
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_lock(&lock) != 0){
			fprintf(stderr, "Error with mutex lock: %s\n", strerror(errno));
			exit(1);
		}
	}

	//Perform insertions
	for(long i = 0; i < iterations; i++){
		SortedList_insert(list, &elemIN[i]);
	}
	
	//Do any required unlocking
	/*if(strcmp(opt_sync, "s") == 0)
		__sync_lock_release(&spinLock);
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_unlock(&lock) != 0){
			fprintf(stderr, "Error with mutex unlock: %s\n", strerror(errno));
			exit(1);
		}
	}*/
	
	//Get list length
	long length = SortedList_length(list);
	
	//Do any required locking
	/*if(strcmp(opt_sync, "s") == 0)
		while(__sync_lock_test_and_set(&spinLock,1));
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_lock(&lock) != 0){
			fprintf(stderr, "Error with mutex lock: %s\n", strerror(errno));
			exit(1);
		}
	}*/
	
	//Ensure all elements inserted
	if(length != iterations){
		fprintf(stderr, "Error during element insertion. Length: %ld Iterations: %ld\n", length, iterations);
		exit(2);
	}
	
	//Do any required unlocking
	/*if(strcmp(opt_sync, "s") == 0)
		__sync_lock_release(&spinLock);
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_unlock(&lock) != 0){
			fprintf(stderr, "Error with mutex unlock: %s\n", strerror(errno));
			exit(1);
		}
	}*/
	
	//Do any required locking
	/*if(strcmp(opt_sync, "s") == 0)
		while(__sync_lock_test_and_set(&spinLock,1));
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_lock(&lock) != 0){
			fprintf(stderr, "Error with mutex lock: %s\n", strerror(errno));
			exit(1);
		}
	}*/
	
	//Lookup and Delete
	SortedListElement_t *lookup;
	for(long i = 0; i < iterations; i++){
		//Lookup
		lookup = SortedList_lookup(list, (elemIN + i)->key);
		if(lookup == NULL){
			fprintf(stderr, "Error during element lookup\n");
			exit(2);
		}
		
		//Delete
		if(SortedList_delete(lookup) != 0){
			fprintf(stderr, "Error during element deletion\n");
			exit(2);
		}
	}
	
	//Do any required unlocking
	if(strcmp(opt_sync, "s") == 0)
		__sync_lock_release(&spinLock);
	else if(strcmp(opt_sync, "m") == 0){
		if(pthread_mutex_unlock(&lock) != 0){
			fprintf(stderr, "Error with mutex unlock: %s\n", strerror(errno));
			exit(1);
		}
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
	fprintf(stdout, "list-%s-%s,%ld,%ld,1,%ld,%ld,%ld\n", yieldOpts, opt_sync, threads, iterations, operations, runTime, avgTimePerOp);
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
		{"sync", required_argument, NULL, 'z'},
		{"yield", required_argument, NULL, 'y'},
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
				fprintf(stderr, "Unrecognized Option. Accepted options: --threads=# --iterations=# --yield=[idl] --sync=[ms] \n");
				exit(1);
				break;
		}
	}
	
	//Initialize locking if necessary
	if(strcmp(opt_sync, "s") == 0)
		spinLock = 0;
	else if(strcmp(opt_sync, "m") == 0)
		pthread_mutex_init(&lock, NULL);
	
	//Allocate memmory for head pointer
	list = malloc(sizeof(SortedList_t));
	
	//Allocate memmory for list elements
	element = malloc(threads*iterations*sizeof(SortedListElement_t));
	if(element == NULL){
		fprintf(stderr, "Error allocating memmory for list elements: %s\n", strerror(errno));
			exit(1);
	}
	
	//Initialize head pointer
	list->key = NULL;
	list->prev = list;
	list->next = list;
	
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
	
	//Check length of list to confirm it's 0
	if(SortedList_length(list) != 0){
		fprintf(stderr, "Non-Zero list length\n");
		exit(2);
	}
	
	//Print CSV to stdout
	printCSV(startTime, endTime);
	
	//Free and exit
	free(list);
	free(element);
	free(string);
	free(ID);
	exit(0);
	
}