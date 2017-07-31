/*
Finds palindromic words in input file and
writes them to output file. Both files
are given as input arguments.

The search for the palindromic words are done 
in parallel using openMP. 


usage in Linux:

	gcc -fopenmp palindrome.c
	./a.out [input file] [output file] numWorkers

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <omp.h>

#define SIZE 100000	  	// size of dictionary
#define WORDSIZE 30	  	// assume a word is at most 30 characters long
#define MAXWORKERS 10 	// max number of threads
#define LIMIT 30		// size of array where linear search is started

char dict[SIZE][WORDSIZE];		//the dictionary
char *palindromic[SIZE];		// the palindromic words from the dictionary
double start_time, end_time;	// start and end time of parallel execution

void swap(int i, int j, char a[]){

 char temp = a[i];
 a[i] = a[j];
 a[j] = temp;

}

char* reverseString(char a[]){
 int i;
 char * b = (char*) malloc(strlen(a));
 strcpy(b, a);
 
 for(i = 0; i < strlen(b); i++){
 	int j = strlen(b) - 1 - i;
 	if(j == i || j < i) break;
	swap(i, j, b);
 }
 
 return b;
}


bool linearSearch(char word[], int low, int high){

for(int i = low; i <= high; i++){
	if((strcmp(word, dict[i]) == 0)) return true;
}
return false;

}


/* searching for a word in dict
   using binary search */
bool search(char word[], int low, int high){

 if(low < high){
 /* if the array is small enough, start linear search */
	if((high - low) < LIMIT){
		return linearSearch(word, low, high);
	} else {
	/* otherwise, continue binary search */
		int mid = low + ((high - low) / 2);
		if((strcmp(word, dict[mid]) < 0)){
			search(word, low, mid);
		} else {
		if((strcmp(word, dict[mid]) == 0)) return true;
		search(word, mid, high);
		}
	}
 } else {
	return false;
 }
 
}


int main(int argc, char *argv[]){

if(argc < 4){
	printf("usage: <file name> <file name> <numThreads>\n");
	return -1;
}

/* read command line arguments */
char *name = argv[1];
char *output = argv[2];
int numWorkers = atoi(argv[3]);
if(numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

/* open input file */
FILE * fp = fopen(name, "r");
assert(fp!=NULL);

int n = 1, index = 0;

/* read from input file and put all words in an array */
while(n > 0){
	n = fscanf(fp, "%s", dict[index]);
	index++;
}

fclose(fp);

index--;
printf("index = %d\n", index);

/* sort dict */
int x, y;

char temp[WORDSIZE];

for (x = 0; x < index; x++) {
      for (y = 0; y < index - 1; y++) {
         if (strcmp(dict[y], dict[y + 1]) > 0) {
            strcpy(temp, dict[y]);
            strcpy(dict[y], dict[y + 1]);
            strcpy(dict[y + 1], temp);
         }
      }
   }
   
for(x = 0; x < index; x++)
	assert(strcmp(dict[x], dict[x + 1]) < 0);   
   

/* reverse the words and search for them in the array using binary search */
int i, j = 0;
/* get start time */
start_time = omp_get_wtime();
#pragma omp parallel for num_threads(numWorkers)
for(i = 0; i < index; i++){
	char *word = dict[i];
	bool pal = false;
	/* word is a palindrome */
	if(strcmp(word, reverseString(word)) == 0){
		pal = true;
	} else {
	/* word is not a palindrome */
		if(search(reverseString(word), 0, index)) pal = true;
	}

	/* if the word is palindromic, put it in
	   the palindromic list. this is put in 
	   a critical section because the list
	   and index j are shared */
	if(pal){
	#pragma omp critical
	{
		palindromic[j] = word;
		j++;
	}
	}
}

/* get end time */
end_time = omp_get_wtime();

printf("j = %d\n", j);

/* writing the palindromic words to a results file */
FILE * fp_out = fopen(output, "w");
assert(fp_out != NULL);

for(i = 1; i < j; i++)
	fprintf(fp_out, "%s\n", palindromic[i]);

printf("Execution time = %g s\n", end_time - start_time);


fclose(fp_out);

}
