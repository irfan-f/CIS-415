/*=============================================================================
 * Program Name: CIS 415: Lab 9
 * Author: Jared Hall, Irfan Filipovic
 * Date: 05/27/2020
 * Description:
 *     A simple program that implements a thread-safe queue of meal tickets
 *
 * Notes:
 *===========================================================================*/

//========================== Preprocessor Directives ==========================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//=============================================================================

//================================= Constants =================================
#define MAXNAME 15
#define MAXQUEUES 4
#define MAXTICKETS 3
#define MAXDISH 20
#define MAXPUBS 3
#define MAXSUBS 4
//=============================================================================

//============================ Structs and Macros =============================

typedef struct mealTicket{
	int ticketNum;
	char *dish;
} mealTicket;

typedef struct MTQ {
	// Queue structure
	char name[MAXNAME];
	int head;
	int tail;
	int length;
	int ticket;
	mealTicket *buffer;
	// Declare a mutex for each queue
	pthread_mutex_t mutex1;
} MTQ;

typedef struct publish {
	mealTicket *tickets;
	char *names[MAXTICKETS][MAXQUEUES];
	int threadid;
	int state;
} publish;

MTQ registry[MAXQUEUES];

void init(int pos, char *MTQ_ID, int len) {
	strcpy(registry[pos].name, MTQ_ID);
	registry[pos].buffer = (mealTicket *)malloc(sizeof(mealTicket)*(len+1));
	registry[pos].head = 0;
	registry[pos].tail = 0;
	registry[pos].length = len;
	registry[pos].ticket = 0;

	//add null ticket or use head-1 to split empty/full case. Up to you.
	registry[pos].buffer[len].ticketNum = -1;
	registry[pos].buffer[registry[pos].tail].ticketNum = 0;
	// Init mutex
	 pthread_mutex_init(&registry[pos].mutex1, NULL);
}

void freeMTQ(int pos, char *MTQ_ID) {
	free(registry[pos].buffer);
}

//=============================================================================

//================================= Functions =================================
int enqueue(char *MTQ_ID, mealTicket *MT) {
	int ret = 0;
	int i, flag = 0;
	//Step-1: Find registry
	for(i=0;i<MAXQUEUES;i++) {
		if(strcmp(MTQ_ID, registry[i].name) == 0) { flag = 1; break; }
	}
	//STEP-2: Enqueue the ticket
	if(flag) {
		int tail = registry[i].tail;
		if(registry[i].buffer[tail].ticketNum != -1) {
			registry[i].buffer[tail].ticketNum = registry[i].ticket;
			registry[i].buffer[tail].dish =  MT->dish;
			registry[i].ticket++;
			if(tail == registry[i].length) { registry[i].tail = 0; }
			else { registry[i].tail++; }
			ret = 1;
		}
	}
	return ret;
}

int dequeue(char *MTQ_ID, int ticketNum, mealTicket *MT) {
	int ret = 0;
	int i, flag = 0;

	//Step-1: Find registry
	for(i=0;i<MAXQUEUES;i++) {
		if(strcmp(MTQ_ID, registry[i].name) == 0) { flag = 1; break; }
	}

	//Step-2: Dequeue the ticket
	if(flag) {
		int head = registry[i].head;
		int tail = registry[i].tail;

		if(head != tail) {
			//copy the ticket
			MT->ticketNum = registry[i].buffer[head].ticketNum;
			MT->dish = registry[i].buffer[head].dish;

			//change the null ticket to empty
			if(head == 0) {
				registry[i].buffer[registry[i].length].ticketNum = 0;
			} else {
			registry[i].buffer[head-1].ticketNum = 0;
			}

			//change the current ticket to null
			registry[i].buffer[head].ticketNum = -1;

			//increment the head
			if(head == registry[i].length+1) { registry[i].head = 0; }
			else { registry[i].head++; }
			ret = 1;
		}
	}
	return ret;
}

void *publisher(void *args) {
	/* TODO: The publisher will receive the following in the struct args:
	*        1. An array of mealTickets to push to the queue.
	*        2. For each meal ticket, an MTQ_ID. (Note: a publisher can push to multiple queues)
	*        3. The thread ID
	*        4. The threads state: alive=1|dead=0
	* The publisher will then print its type and thread ID on startup.
	*/

	// Code to print current thread, and find which queues needed
	int j, i;
	struct publish *data;
	data = (struct publish *) args;
	fprintf(stdout, "Publisher Thread ID: %d, %lds\n", data->threadid, pthread_self());
	for(i = 0; i < MAXTICKETS; i++) {
		if(data->tickets[i].dish != NULL) {
			char **ids = data->names[i];
			int count = 0;
			int multiple[MAXQUEUES];
			int max = 1;
			for(j = 0; j < MAXQUEUES; j++) {
				//fprintf(stdout, "%s\n", ids[j]);
				while(ids[count] != NULL) {
					if(strcmp(ids[count], registry[j].name) == 0) {
						multiple[count] = j;
						count++;
					} else {
						max = count+1;
						break;
					}
				}
			}

			// data->ticket[i] contains mealTicket, multiple[] contains which queues.
			// Iterate each queue needed.
			for(int z = 0; z < max; z++) {
				pthread_mutex_lock(&(registry[multiple[z]].mutex1));
				if(enqueue(registry[multiple[z]].name, &data->tickets[i]) == 0) {
					fprintf(stdout, "publisher %d, %ld: buffer %s is full\n", data->threadid, pthread_self(), registry[multiple[z]].name);
					pthread_mutex_unlock(&(registry[multiple[z]].mutex1));
					sleep(1);
				} else {
					pthread_mutex_unlock(&(registry[multiple[z]].mutex1));
					sleep(1);
				}
			}
		}
	}
}

typedef struct subscribe {
		char *MTQS[MAXQUEUES];
		int threadid;
		struct mealTicket *ticket;
		int state;
} subscribe;

void *subscriber(void *args) {
	/* TODO:The subscriber will take the following:
	* 1. The MTQ_ID's it will pull from. (Note: A subscriber can pull from multiple queues.)
	* 2. The thread ID
	* 3. A temp meal ticket struct.
	* 4. The threads state: alive=1|dead=0

	* The subscriber will print its type and thread ID on startup. Then it will pull a ticket from its queue
	* and print it. If the queue is empty then it will print an empty message along with its
	* thread ID and wait for 1 second. If the thread is not empty then it will pop a ticket and
	* print it along with the thread id.
	*/
	struct subscribe *data;
	data = (struct subscribe *) args;
	int i = 0;
	for(int i = 0; i < 3; i++) {
		int j;
		fprintf(stdout, "%s\n", data->MTQS[i]);
		for(j = 0; j < MAXQUEUES; j++) {
			if(strcmp(data->MTQS[i], registry[j].name) == 0) { break; }
		}
		pthread_mutex_lock(&(registry[j].mutex1));
		if(dequeue(data->MTQS[i], 0, data->ticket) == 0) {
			fprintf(stdout, "subscriber %d, %ld: buffer %s is empty\n", data->threadid, pthread_self(), registry[j].name);
			pthread_mutex_unlock(&(registry[j]).mutex1);
			sleep(1);
		} else {
			pthread_mutex_unlock(&(registry[j]).mutex1);
			fprintf(stdout, "subscriber: %d Ticket: %d ### Dish: %s\n", data->threadid,data->ticket->ticketNum, data->ticket->dish);
		}
	}
}
//=============================================================================

//=============================== Program Main ================================
int main(int argc, char argv[]) {
	//Variables Declarations
	char *qNames[] = {"Breakfast", "Lunch", "Dinner", "Bar"};
	char *bFood[] = {"Eggs", "Bacon", "Steak"};
	char *lFood[] = {"Burger", "Fries", "Coke"};
	char *dFood[] = {"Steak", "Greens", "Pie"};
	char *brFood[] = {"Whiskey", "Sake", "Wine"};
	int i, j, t = 1;
	int test[4];
	char dsh[] = "Empty";
	mealTicket bfast[3] = {[0].dish = bFood[0], [1].dish = bFood[1], [2].dish = bFood[2]};
	mealTicket lnch[3] = {[0].dish = lFood[0], [1].dish = lFood[1], [2].dish = lFood[2]};
	mealTicket dnr[3] = {[0].dish = dFood[0], [1].dish = dFood[1], [2].dish = dFood[2]};
	mealTicket br[3] = {[0].dish = brFood[0], [1].dish = brFood[1], [2].dish = brFood[2]};
	mealTicket ticket = {.ticketNum=0, .dish=dsh};
	//STEP-1: Initialize the registry
	for(int i=0; i<4; i++) {
		init(i, qNames[i], 3);
	}

	//STEP-2: Create the publisher thread-pool
	struct publish pub = {.tickets = bfast, .threadid = 0, .state = 1};
	pub.names[0][0] = "Breakfast";
	for(int i = 1; i < MAXQUEUES; i++) {
		pub.names[0][i] = NULL;
	}
	pub.names[1][0] = "Breakfast";
	for(int i = 1; i < MAXQUEUES; i++) {
		pub.names[1][i] = NULL;
	}
	pub.names[2][0] = "Breakfast";
	pub.names[2][1] = "Dinner";
	for(int i = 2; i < MAXQUEUES; i++) {
		pub.names[2][i] = NULL;
	}
	// create pool
	pthread_t pub_pool[MAXPUBS];
	for(int i = 0; i < MAXPUBS; i++) {
		pub.threadid = i;
		pthread_create(&pub_pool[i], NULL, &publisher, (void *) &pub);
	}
	//STEP-3: Create the subscriber thread-pool
	for(int i = 0; i < MAXPUBS; i++) {
		if(i < MAXPUBS) {pthread_join(pub_pool[i], NULL); };
	}
	struct mealTicket meal;
	dequeue("Dinner", 0, &meal);
	printf("woooooooooooooooooooooooooooo %s\n", meal.dish);
	struct subscribe sub = {.ticket = &meal, .threadid = 0, .state = 1};
	for(int i = 0; i < 5; i++) {
		sub.MTQS[i] = qNames[i];
	}
	// create pool
	pthread_t sub_pool[MAXSUBS];
	for(int i = 0; i < MAXSUBS; i++) {
		sub.threadid = i;
		pthread_create(&sub_pool[i], NULL, &subscriber, &sub);
	}
	//STEP-4: Join the thread-pools

	for(int i = 0; i < MAXSUBS; i++) {
		if(i < MAXSUBS) {pthread_join(sub_pool[i], NULL); };
	}
	//STEP-5: Free the registry
	for(int i = 0; i < MAXQUEUES; i++) {
		free(registry[i].buffer);
	}
	return EXIT_SUCCESS;
}
//=============================================================================
