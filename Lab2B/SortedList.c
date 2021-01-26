#include "SortedList.h"
#include <string.h>
#include <sched.h>

int opt_yield = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	//Ensure list, element valid
	if(element == NULL || list == NULL){
		return;
	}

	//Circular doubly-linked list so inserting anywhere is the same
	
	SortedListElement_t *iterator = list->next;
	
	//Keep iterating until element key is less than than current node key or reach end of list
	while(iterator != list && strcmp(iterator->key, element->key) <= 0){
		iterator = iterator->next;
	}

	//Critical section
	if(opt_yield & INSERT_YIELD)
		sched_yield();

	//Then insert before iterator
	element->prev = iterator->prev;
	element->next = iterator;
	iterator->prev->next = element;
	iterator->prev = element;

}

int SortedList_delete( SortedListElement_t *element){
	//Ensure element is valid and not the head
	if(element == NULL || element->key == NULL){
		return 1;
	}
	
//Reassign neighbors' pointers to remove element
	//Ensure next->prev amd prev-next point to this node
	if(element->next->prev != element || element->prev->next != element)
		return 1;
	else{
		//Critical section
		element->next->prev = element->prev;
		
		if(opt_yield & DELETE_YIELD)
			sched_yield();
		
		element->prev->next = element->next;
	}
	
	return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
	//No matching element if list invalid
	if(list == NULL)
		return NULL;
	
	//Don't check head pointer (head key is NULL)
	SortedListElement_t *iterator = list->next;
	
	//Critical section in loop
	//Iterate through elements and compare key until found or end
	while(iterator != list){
		if(opt_yield & LOOKUP_YIELD)
			sched_yield();
		
		if(strcmp(iterator->key, key) == 0)
			return iterator;
		
		iterator = iterator->next;
	}
	
	//No matches found
	return NULL;
}

int SortedList_length(SortedList_t *list){
	//-1 if list corrupted
	if(list == NULL)
		return -1;
	
	//Count elements by iterating through list until end
	int count = 0;
	
	//Exclude head pointer
	SortedListElement_t *iterator = list->next;
	
	//Critical section in loop
	while(iterator != list){
		if(opt_yield & LOOKUP_YIELD)
			sched_yield();
		
		count++;
		iterator = iterator->next;	
	}
	return count;
}