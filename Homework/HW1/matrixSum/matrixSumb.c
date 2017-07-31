/* matrix summation using pthreads

   features: the main method computes the total sum, and the 
   			 value and position of min and max element, from 
   			 partial results computed by Workers. the partial
   			 results are passed to main method via pthread_join
   			 and pthread_exit
             
   usage under Linux:
     gcc matrixSumb.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/* the parameters that each thread returns */
struct ret{int sum; int max; int min; int mini; int minj; int maxi; int maxj;};

/* timer */
double read_timer() {
  static bool initialized = false;
  static struct timeval start;
  struct timeval end;
  if( !initialized )
  {
      gettimeofday( &start, NULL );
      initialized = true;
  }
  gettimeofday( &end, NULL );
  return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix, unless its too big */
#ifdef DEBUG
  if(size <= 100){
	  for (i = 0; i < size; i++) {
		  printf("[ ");
		  for (j = 0; j < size; j++) {
			printf(" %d", matrix[i][j]);
		  }
		  printf(" ]\n");
	  }
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);

/* calculate and print final result by adding the
   results from the workers */
   int total, min, max, mini, minj, maxi, maxj;
   total = 0;
   max = 0;
   min = INT_MAX;
   for (i = 0; i < numWorkers; i++){
   	struct ret *retval;
   	pthread_join(workerid[i], (void**)&retval);
     total += retval->sum;
	 if(retval->max > max){
	 	max = retval->max; maxi = retval->maxi; maxj = retval->maxj;
	 }
	 if(retval->min < min){
	 	min = retval->min; mini = retval->mini; minj = retval->minj;
	 }
	 free(retval); 	  
   }
   
   /* get end time */
   end_time = read_timer();
   
   /* print results */
   printf("The total is %d\n", total);
   printf("Maximal element is [%d, %d] = %d\n", maxi, maxj, max);
   printf("Minimum element is [%d, %d] = %d\n", mini, minj, min);
   printf("The execution time is %g sec\n", end_time - start_time);

}

/* Each worker sums the values in one strip of the matrix
   and calculates the position and value of min and max elements
   of their strips. The result is returned to the main function 
   by calling pthread_exit */
void *Worker(void *arg) {
  long myid = (long) arg;
  int total, i, j, first, last;
  int min, max, mini, minj, maxi, maxj;
#ifdef DEBUG
  printf("worker %ld (pthread id %ld) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);
  
  struct ret *myret = malloc(sizeof(struct ret));

  /* sum values in my strip */
  total = 0;
  max = 0; 
  min = INT_MAX;
  for (i = first; i <= last; i++){
    for (j = 0; j < size; j++){
      total += matrix[i][j];
	  if(matrix[i][j] > max){
		max = matrix[i][j];
		maxi = i;
		maxj = j;
		}
	  if(matrix[i][j] < min){
	  	min = matrix[i][j];
	  	mini = i;
	  	minj = j;
	  }
  	}
  }
  myret->sum = total;
  myret->max = max;
  myret->min = min;
  myret->maxi = maxi;
  myret->maxj = maxj;
  myret->mini = mini;
  myret->minj = minj;
  
  pthread_exit((void*) myret);

}

