/*The bear and honeybees problem: one consumer, multiple producers

Description:
one bear, n honeybees. a pot with a capacity of H portions of honey.
bear sleeps until pot is full, then eats all honey and goes back to sleep.
Each bee repeatedly gathers one portion of honey and puts it in the pot; the bee who fills the pot awakens the bear.
only semaphores are used for synchronization.

Usage in Linux:

	gcc honeybees.c -lpthread
	./a.out n
*/


#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define MAXWORKERS 10
#define SHARED 1

int H = 0, n, max = 10;
int produced[MAXWORKERS];	// an array to count the number of portions each bee has produced

sem_t sem_full;	// signaling semaphores, one binary semaphore, one counting semaphore
sem_t mutex;	// semaphore used to lock shared resource (the honeypot)

void *producer(void *arg){

 long myid = (long) arg;
 
 while(1){
 	sem_wait(&mutex);
 	if(H == max){
 	/* honeypot is full: wake the bear */
		printf("honeybee %ld. waking bear\n", myid);
 		sem_post(&sem_full);
 	} else {
 	 	printf("honeybee %ld. pot contains %d portions. %d portions produced\n", myid, ++H, ++produced[myid]);
	 	sem_post(&mutex);
	 	sleep(rand()%5);
 	}
 }
}


void *consumer(){

 while(1){
 	sem_wait(&sem_full);
 	printf("bear woken. eats all honey\n");
 	H = 0;	// eating
 	sem_post(&mutex);
 }
}

int main(int argc, char *argv[]){

/* read command line argument, if it exists */
 n = (argc > 1)? atoi(argv[1]) : MAXWORKERS;
 if(n > MAXWORKERS) n = MAXWORKERS;
 
 pthread_t bees[n];
 pthread_t bear;

/* initiate semaphores */
 sem_init(&sem_full, SHARED, 0); 		// honeypot starts out as empty
 sem_init(&mutex, SHARED, 1);			// mutex initialized to 1 to indicate critical section is free  	

/* creating threads to represent bees and the bear */
 pthread_create(&bear, NULL, consumer, NULL);

 long i;
 for(i = 0; i < n; i++)
	pthread_create(&bees[i], NULL, producer, (void*)i);

/* wait for threads to terminate (which they, incidentally, won't) */
 pthread_join(bear, NULL);
 for(i = 0; i < n; i++)
 	pthread_join(bees[i], NULL);

 return 0;
 
}
