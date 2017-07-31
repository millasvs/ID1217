/*
	A parallell implementation of the quicksort algorithm
	using the Lomuto partition.
	
	Usage in Linux:
	gcc quickSort.c -lpthread
	./a.out n numWorkers 
*/

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<pthread.h>
#include <sys/time.h>
#include <assert.h>

#define MAXSIZE 1000000		/* maximum array size */
#define MAXVALUE 1000		/* upper threshold for value of array elements */
#define MAXWORKERS 10   	/* maximum number of workers */

int n;						/* size of array */
int array[MAXSIZE];			/* array to be sorted */
int numWorkers;           	/* number of workers */ 

double start_time, end_time; /* start and end times */

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

 /* prints the list */
void print_list(){
  int j;
  printf("[ ");
  for (j = 0; j < n; j++) {
    printf(" %d", array[j]);
  }
  printf(" ]\n");
}

 /* simple swap function */
void swap(int i, int j){

  int temp = array[i];
  array[i] = array[j];
  array[j] = temp;
}

 /* swaps elements and returns new pivot */
int partition(int low, int high){

  int pivot = array[high];
  int i = low;
  int j;
  for(j = low; j < high; j++){
  	if(array[j] <= pivot){
  		swap(i, j);
  		i++;
  	}
   }
  swap(i, high);
  return i;	
}

 /* serial quicksort */
void quicksort(int low, int high){

  if(low < high){
  	int pivot = partition(low, high);
	quicksort(low, pivot - 1);
	quicksort(pivot + 1, high);
  
  }
}

/* arguments to pass into parallel quicksort */
struct args{int low; int high; int id;};
void q_sort(int low, int high, int id);

/* start parallel quicksort with right number of arguments */
void *initiate_thread(void *arg){

  q_sort(((struct args*)arg)->low, ((struct args*)arg)->high, ((struct args*)arg)->id);
  return 0; 
}

 /* parallel quicksort */
void q_sort(int low, int high, int id){
  
  int rc;
  /* make sure the list has more than one element */
  if(low < high){
  	  /* start threads until maximum number of threads has been reached */
	  if((id++ < numWorkers)){
	  	
	  	int pivot = partition(low, high);
	  	
	  	pthread_t thread;	
  		struct args myargs = {low, pivot - 1, id};
		rc = pthread_create(&thread, NULL, initiate_thread, &myargs);
		if(rc != 0)	printf("thread %d failed\n", id);
			  
		q_sort(pivot + 1, high, id);
		/* wait for thread created above to terminate */
	  	pthread_join(thread, NULL);
	  }
	  /* if maximum number of threads has been reached, 
	  do serial quicksort instead */
	  else{
		quicksort(low, high);
	  }
  }
}


int main(int argc, char *argv[]){

 /* read command line args if any */
  n = (argc > 1)? atoi(argv[1]) : MAXSIZE; 
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (n > MAXSIZE) n = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
 /* fill array with random values */
  int i;
  for(i = 0; i < n; i++)
	  array[i] = rand()%MAXVALUE;

 /* print initial array, unless its too big */
  if(n <= 100){
  printf("Initial array:\n");
  print_list();
  }
  
  /* get start time */
  start_time = read_timer();
  
  /* call parallel quicksort */
  q_sort(0, n-1, 0); 

  /* get end time */
  end_time = read_timer();
  
  /* print results */
  printf("The execution time is %g sec\n", end_time - start_time);
  
  /* print sorted array, unless its too big */
  if(n <= 100){
  printf("Sorted array:\n"); 
  print_list();
  }
  
  /* assert that the list is indeed sorted */
    for (int i = 0; i < n; i++)
        assert(array[i-1] <= array[i]);

} 
