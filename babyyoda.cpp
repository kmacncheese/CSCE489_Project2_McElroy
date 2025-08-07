/*************************************************************************************
 * babyyoda - used to test your semaphore implementation and can be a starting point for
 *			     your store front implementation
 *
 *************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "Semaphore.h"

// Semaphores that each thread will have access to as they are global in shared memory
Semaphore *empty = NULL;
Semaphore *full = NULL;

pthread_mutex_t buf_mutex;

// initialize the starting vars

//buffer stuff
int *buffer = NULL; //make it an array now
int buffer_size = 0; //main param will set this, just need it to be global
int in_pos = 0; // where in the buffer array the producer will place the next item
int out_pos = 0; // where in the buffer array the consumer will take the next item

//counters to keep track of things
int produced_count = 0;
int consumed_count = 0;
int num_consumers = 0;
int max_items = 0; // main param will set this


/*************************************************************************************
 * producer_routine - this function is called when the producer thread is created.
 *
 *			Params: data - a void pointer that should point to an integer that indicates
 *							   the total number to be produced
 *
 *			Returns: always NULL
 *
 *************************************************************************************/

void *producer_routine(void* data) {
	(void) data;

	printf("Producer has opened the babyyoda shop.\n");

	//simple loop, single producer so this won't change much.
	//I just refactored this because I didn't understand the original code too much, my implementation made more sense in my head.
	while (produced_count < max_items){
		//the item to be produced
		int item = produced_count + 1;
		
		//wait to enter critical section, which will decrement S.
		empty->wait();

		//Critical section
		// lock the section
		pthread_mutex_lock(&buf_mutex);

		//place new item at the in_pos, where it should go next
		buffer[in_pos] = item;
		printf("	[+] Producer put Yoda #%d on shelf %d.\n", item, in_pos);

		//increment next item position, and wrap around the buffer if necessary.
		in_pos++;
		in_pos %= buffer_size;
		produced_count++; //increment counter

		pthread_mutex_unlock(&buf_mutex);
		// end of critical section

		//signal that the producer is available! this will increment S.
		full->signal();

		usleep((useconds_t)(rand() % 200000));
	}

	printf("\nProducer has made all the yodas and is now dipping.\n\n");

	//closing time, double check for deadlocks
	for (int i = 0; i < num_consumers; i++){
		full->signal();
	} 

	return NULL;
}


/*************************************************************************************
 * consumer_routine - this function is called when the consumer thread is created.
 *
 *       Params: data - a void pointer that should point to a boolean that indicates
 *                      the thread should exit. Doesn't work so don't worry about it
 *
 *       Returns: always NULL
 *
 *************************************************************************************/

void *consumer_routine(void *data) {
	//data should be the thread's ID, so dereference to get the value and then free.
	int t_id = *((int *) data);
	delete (int*) data;

	printf("Consumer #%d has entered the store.\n", t_id);

	while (true) {

		//so the original code here had it wait for an empty slot, but I think it should be waiting for a full one, since that means its "stocked"
		full->wait();

		// Critical section starts
		//Lock it to keep it safe
		pthread_mutex_lock(&buf_mutex);
	
		//consumer cannot buy a babyyoda if it is over the specified max_items
		if (consumed_count >= max_items) {
			// if we are at max consumption, then leave the critical section and exit.
			pthread_mutex_unlock(&buf_mutex);
			full->signal();
			break;

		}
		printf("	[-] Consumer #%d bought Yoda #%d from shelf %d. \n", t_id, buffer[out_pos], out_pos);  
		
		//increment and wrap
		consumed_count++;
		out_pos++;
		out_pos %= buffer_size;
	
		// end of critical section, unlock and signal
		pthread_mutex_unlock(&buf_mutex);

		//again a change in logic here from the original code. Since we just took a yoda, it should signal the empty array to increment.
		empty->signal();

		// Consumers wait up to one second.  should wait fater signal since its not critical. Doing otherwise would just delay the other processes.
		usleep((useconds_t) (rand() % 1000000));
	}
	
	printf("Consumer #%d goes home.\n",t_id);
	return NULL;	
}


/*************************************************************************************
 * main - Standard C main function for our storefront. 
 *
 *		Expected params: pctest <num_consumers> <max_items>
 *				max_items - how many items will be produced before the shopkeeper closes
 *
 *************************************************************************************/

int main(int argv, const char *argc[]) {

	// Get our argument parameters
	if (argv != 4) {
		printf("Invalid parameters. Format: %s <buffer_size> <num_consumers> <max_items> \nExiting...", argc[0]);
		exit(0);
	}

	// Parameters
	buffer_size = (unsigned int) strtol(argc[1], NULL, 10);
	num_consumers = (unsigned int) strtol(argc[2], NULL, 10);
	max_items= (unsigned int) strtol(argc[3], NULL, 10);

	//check params
	if (buffer_size <= 0 || num_consumers <= 0 || max_items <= 0){
		printf("Arguments cannot be negative.\nExiting...");
		exit(0);
	}

	printf("babyyoda store starting with <buffer_size>:%d, <num_consumers>:%d, and <max_items>:%d\n\n", buffer_size, num_consumers, max_items);

	// Initialize our semaphores
	buffer = new int[buffer_size];
	empty = new Semaphore(buffer_size); //empty will start like this because all slots are initially empty
	full = new Semaphore(0); //full will start like this because all slots no slots are initially full

	pthread_mutex_init(&buf_mutex, nullptr); // Initialize our buffer mutex, nullptr instead of NULL since we are dealing with an array now.

	//make the threads
	pthread_t producer;
	pthread_t *consumers = new pthread_t[num_consumers];

	// Launch our one producer thread
	pthread_create(&producer, NULL, producer_routine, NULL);

	// Launch our multiple consumer threads
	for (int i = 0; i < num_consumers; i++){
		int *t_id = new int(i + 1); // a unique thread id for each consumer thread
		//create the thread based off the the address of consumers[i], so either &consumers[i] or consumers+i
		pthread_create(consumers + i, nullptr, consumer_routine, t_id);
	}

	// Wait for our producer thread to finish up
	pthread_join(producer, NULL);
	printf("\nThe manufacturer has completed his work for the day.\n");

	printf("Waiting for consumer to buy up the rest.\n\n");

	// Now make sure they all exited
	for (int i = 0; i < num_consumers; i++){
		pthread_join(consumers[i], NULL);
	}

	// We are exiting, clean up
	delete empty;
	delete full;		
	delete[] buffer;
	delete[] consumers;
	pthread_mutex_destroy(&buf_mutex);

	printf("\nProducer/Consumer simulation complete!\n - Total items produced: %d\n - Total items consumed: %d\n", produced_count, consumed_count);
}
