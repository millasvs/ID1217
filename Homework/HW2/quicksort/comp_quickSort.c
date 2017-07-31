/*
	Measure speedup of parallel implementation of quicksort
	using 2, 3 and 4 processors.
	Speedup is defined as serial execution time divided by
	parallel execution time.
	
	Usage in Linux:
	gcc -fopenmp quickSort.c 
	./a.out
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

 /* serial quicksort */
void serial_quicksort(int low, int high){

  if(low < high){
  	int pivot = partition(low, high);
	quicksort(low, pivot - 1);
	quicksort(pivot + 1, high);
  }
}

 /* fill array with random values */ 
void fill_array(){

  int i;
  for(i = 0; i < n; i++)
	  array[i] = rand()%MAXVALUE;
}

int main(int argc, char *argv[]){

 /* start time, end time and time elapsed
 	for serial quicksort */
 double s_start_time, s_end_time, s_timeElapsed;
 /* execution time for parallel quicksort, and
 	speedup for parallel quicksort */
 double timeElapsed, speedup;

 printf("#	speedup parallel quicksort\n");
 printf("#processors\tsize\tspeedup\n");
  for(numWorkers = 2; numWorkers < 5; numWorkers++){
	 for(n = 10000; n < MAXSIZE; n+=(n/4)){
	 	/* fill the array for serial quicksort */
	 	fill_array();
	 	s_start_time = omp_get_wtime();
	 	/* perform serial quicksort */
	 	serial_quicksort(0, n - 1);
	 	s_end_time = omp_get_wtime();
	 	/* check that the array is indeed sorted */
		for (int i = 0; i < n; i++)
			assert(array[i-1] <= array[i]);
	 	/* calculate execution time of serial quicksort */
	 	s_timeElapsed = s_end_time - s_start_time;
	 	
	 	/* refill the array for parallel quicksort */
	 	fill_array();
	 	start_time = omp_get_wtime();
	 	/* perform parallel quicksort */
		#pragma omp parallel shared (n) num_threads(numWorkers)
		{
		#pragma omp single 
		  quicksort(0, n-1);
		}
	 	end_time = omp_get_wtime();
	 	/* check that the array is indeed sorted */	 	
		for (int i = 0; i < n; i++)
			assert(array[i-1] <= array[i]);
	 	/* calculate execution time of parallel quicksort */	 	
	 	timeElapsed = end_time - start_time;
	 	/* calculate speedup by dividing serial execution
	 	   time with parallel execution time */
	 	speedup = s_timeElapsed / timeElapsed;
	 	/* print the results */
	 	printf("%d\t\t%d\t%g\n", numWorkers, n, speedup);
	 }
 }
}

