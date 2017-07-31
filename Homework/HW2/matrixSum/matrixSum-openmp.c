/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/

#include <omp.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAXSIZE 20000		/* maximum matrix size */
#define MAXVALUE 1000		/* upper threshold for value of array elements */
#define MAXWORKERS 10   	/* maximum number of workers */

double start_time, end_time; 	/* start and end times */
int numWorkers;           		/* number of workers */ 
int size; 						/**/
int matrix[MAXSIZE][MAXSIZE];	/* matrix to be calculated on */

/* check correctness of parallel implementation
   by comparing with result of serial computation */
void check_results(long total, int min, int max){
unsigned long sum = 0;
int amin = INT_MAX;
int amax = 0;
for(int i = 0; i < size; i++){
	for(int j = 0; j < size; j++){
		sum += matrix[i][j];
		if(amin > matrix[i][j]) amin = matrix[i][j];
		if(amax < matrix[i][j]) amax = matrix[i][j];		
	}
}

assert(total == sum);
assert(min == amin);
assert(max == amax);

}


/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  unsigned long total = 0;

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

int min = INT_MAX, max = 0, min_i, min_j, max_i, max_j;
  start_time = omp_get_wtime();
#pragma omp parallel
{
int my_min = INT_MAX, my_max = 0, mmin_i, mmin_j, mmax_i, mmax_j; 
#pragma omp for reduction (+:total) private(j)
  for (i = 0; i < size; i++){
    for (j = 0; j < size; j++){
    //printf("i = %d j = %d myid = %d\n", i, j, omp_get_thread_num());
      total += matrix[i][j];
      if(matrix[i][j] > my_max){ 
      	my_max = matrix[i][j];
      	mmax_i = i;
      	mmax_j = j;
      }
      if(matrix[i][j] < my_min){
      	my_min = matrix[i][j];
      	mmin_i = i;
      	mmin_j = j;
      }
    }
   }
// implicit barrier

 if(my_min < min){
 #pragma omp critical
 {
	min = my_min;
	min_i = mmin_i;
	min_j = mmin_j;
 }
 }
 if(my_max > max){
 #pragma omp critical
 {
	max = my_max;
	max_i = mmax_i;
	max_j = mmax_j;
 }
 }
}

  end_time = omp_get_wtime();
  check_results(total, min, max);
  
  /* print results */
  printf("The total is %lu\n", total);
  printf("Min element is [%d, %d] = %d\n", min_i, min_j, min);
  printf("Max element is [%d, %d] = %d\n", max_i, max_j, max);
  printf("The execution time is %g sec\n", end_time - start_time);
}

