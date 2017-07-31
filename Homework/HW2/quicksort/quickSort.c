/*
	A parallell implementation of the quicksort algorithm
	using openMP.
	
	Usage in Linux:
	gcc -fopenmp quickSort.c 
	./a.out n numWorkers 
*/

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<omp.h>

#define MAXSIZE 10000000	/* maximum array size */
#define MAXVALUE 1000		/* upper threshold for value of array elements */
#define MAXWORKERS 10   	/* maximum number of workers */

int n;						/* size of array */
int array[MAXSIZE];			/* array to be sorted */
int numWorkers;           	/* number of workers */ 

double start_time, end_time; /* start and end times */

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

  int a = (int) (low + (high - low + 1)*(1.0 * rand() / RAND_MAX));
  int pivot = array[a];
  swap(a, high);
  int i = low;
  int j;
  for(j = low; j < high; j++){
  	if(array[j] < pivot){
  		swap(i, j);
  		i++;
  	}
   }
  swap(i, high);
  return i;	
}

 /* parallel quicksort */
void quicksort(int low, int high){

  if(low < high){
  	int pivot = partition(low, high);
#pragma omp task	
	quicksort(low, pivot - 1);
#pragma omp task
	quicksort(pivot + 1, high);
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

#ifdef DEBUG
 /* print initial array, unless its too big */
  if(n <= 100){
	  printf("Initial array:\n");
	  print_list();
  }
#endif
  
  /* get start time */
  start_time = omp_get_wtime();

  /* start parallel quicksort */
#pragma omp parallel shared (n) num_threads(numWorkers)
{
#pragma omp single 
  quicksort(0, n-1);
}
  /* get end time */
  end_time = omp_get_wtime();

#ifdef DEBUG  
  /* print sorted array, unless its too big */
  if(n <= 100){
	  printf("Sorted array:\n"); 
	  print_list();
  }
#endif
  
  /* assert that the list is indeed sorted */
  for (int i = 0; i < n; i++)
  	assert(array[i-1] <= array[i]);
  
  /* print results */
  printf("The execution time is %g sec\n", end_time - start_time);


} 
