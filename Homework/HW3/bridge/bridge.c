/* One lane bridge problem.

Cars coming from the north and the south arrive at a one-lane bridge. 
Cars heading in the same direction can cross the bridge at the same time, 
but cars heading in opposite direction cannot. Only semaphores are used for synchronization,
in a "passing the baton" fashion.
The program is fair in the sense that both northbound and southbound cars 
give preference to cars going in the same direction (a new southbound car is delayed
when a northbound car is waiting and vice versa)

Global invariant: (ns == 0 || nn == 0) && (ns != nn) && (ns >= 0 && nn >= 0). This should always be true.

Usage in Linux:
	gcc bridge.c -lpthread
	./a.out n
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#define MAXWORKERS 100
#define SHARED 1

/* one semaphore to allow northbound cars, one to allow southbound cars, 
and one to gain access to critical section (bridge) */
sem_t sem_north, sem_south, mutex;
/* number of cars and upper boundary on the number of cars on the bridge */
int n, boundary = 30;	 
/* number of cars going north/south on the bridge, 
number of delayed cars headed north/south */
int nn = 0, ns = 0, dn = 0, ds = 0; 
/* arguments that each car passes to car function */
struct args{long myid; int initialDirection; int trips;} args;

void *car(void *args){
/* unwrapping arguments */
 struct args *myargs = args;
 long myid = myargs->myid;
 int initialDirection = myargs->initialDirection;
 int trips = myargs->trips;
 int direction = 1 - initialDirection;
 while(trips > 0){
 	 sleep(2 + rand()%3);		// time taken between crossing the bridge again
	 direction = 1 - direction;	// switch direction
	 if(direction){
	 /**** southbound ****/
	 /* < await (nn == 0) ns = ns + 1 >*/
	 	sem_wait(&mutex);
	 	if(nn > 0) { ds++; sem_post(&mutex); sem_wait(&sem_south);}
	 	ns = ns + 1;
	 	printf("\ncar %ld going south. %d trips left\n", myid, --trips);
	 	printf("nn = %d ns = %d ds = %d dn = %d\n", nn, ns, ds, dn);
	 	assert((nn == 0) || (ns == 0));
	 	assert(nn != ns); 
	 /* if there are any delayed southbound cars, let them pass too */	
	 	if(ds > 0) { ds = ds - 1; sem_post(&sem_south);}
 		else sem_post(&mutex);
	 	sleep(rand()%2);			// time taken to cross the bridge
	 /* < ns = ns - 1 > */
	 	sem_wait(&mutex);
	 	ns = ns - 1;
	 /* passing the baton to a delayed northbound car, if no southbound car is on the bridge 
	 	or the number of southbound cars is bigger than the boundary */
	 	if((ns == 0 || ns > boundary) && dn > 0) { dn = dn - 1; sem_post(&sem_north);}
	 	else sem_post(&mutex);
	 } else {
	 /**** northbound ****/
 	 /* < await (ns == 0) nn = nn + 1 >*/
	 	sem_wait(&mutex);
	 	if(ns > 0) { dn++; sem_post(&mutex); sem_wait(&sem_north);}
	 	nn = nn + 1;
	 	printf("\ncar %ld going north. %d trips left\n", myid, --trips);
	 	printf("nn = %d ns = %d ds = %d dn = %d\n", nn, ns, ds, dn);
	 	assert((nn == 0) || (ns == 0));
	 	assert(nn != ns); 
 	 /* if there are any delayed northbound cars, let them pass too */	
	 	if(dn > 0) { dn = dn - 1; sem_post(&sem_north);}
	 	else sem_post(&mutex);
	 	sleep(rand()%2);			// time taken to cross the bridge
	 /* < nn = nn - 1 > */
	 	sem_wait(&mutex);
	 	nn = nn - 1;
	 /* passing the baton to a delayed southbound car, if no northbound car is on the bridge*/
	 	if((nn == 0 || nn > boundary) && ds > 0) { ds = ds - 1; sem_post(&sem_south);}
	 	else sem_post(&mutex);
	 }
 } 
}


int main(int argc, char *argv[]){

/* read command line argument, if it exists */
 n = (argc > 1)? atoi(argv[1]) : MAXWORKERS;
 if(n > MAXWORKERS) n = MAXWORKERS;
 
 pthread_t cars[n];
 struct args args[n];	// arguments that cars pass
 
/* initiate semaphores */
 sem_init(&sem_north, SHARED, 0);  // semaphores initialized to 0 to indicate ns == 0 and nn == 0
 sem_init(&sem_south, SHARED, 0);	
 sem_init(&mutex, SHARED, 1);		// mutex initialized to 1 to indicate critical section (bridge) is free  	

/* creating threads to represent cars */
 long i; int trips = 5;
 for(i = 0; i < n; i++){
 	int initialDirection;
 	/* cars with even id-number starts going north and vice versa */
 	if(i%2 == 0) initialDirection = 0; 
 	else initialDirection = 1; 
 	args[i].myid = i;
 	args[i].initialDirection = initialDirection;
 	args[i].trips = trips;
	pthread_create(&cars[i], NULL, car, &args[i]);
 }
/* wait for threads to terminate */ 
 for(i = 0; i < n; i++)
 	pthread_join(cars[i], NULL);

 return 0;

}



