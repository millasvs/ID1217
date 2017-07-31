/*

To run in Linux:
make -f Makefile_p openmp
./openmp

*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <omp.h>
#include "common.h"

int n_threads;
extern int num_bins, num_rows; 
int *globalIds; 
//
//  benchmarking program
//
int main( int argc, char **argv )
{    
    if( find_option( argc, argv, "-h" ) >= 0 )
    {
        printf( "Options:\n" );
        printf( "-h to see this help\n" );
        printf( "-n <int> to set number of particles\n" );
        printf( "-p <int> to set the number of threads\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }

    int n = read_int( argc, argv, "-n", 1000 );
    n_threads = read_int( argc, argv, "-p", 2 );
    char *savename = read_string( argc, argv, "-o", NULL );

    FILE *fsave = savename ? fopen( savename, "w" ) : NULL;

    particle_t *particles = (particle_t*) malloc( n * sizeof(particle_t) );
    set_size( n );
	globalIds =  (int*) malloc(n * sizeof(int));
    init_particles( n, particles );
    
    bin_t *bins = (bin_t*) malloc( num_bins * sizeof(bin_t) );
	
	/* make room for up to n particles per bin */
	for(int i = 0; i < num_bins; i++)
		bins[i].particle_ids = (int*) malloc(n*sizeof(int));

	/* initialise the bins */
    init_bins(bins);

	for(int i = 0; i < n; i++)
		globalIds[i] = (int)(floor(particles[i].x / cutoff) * num_rows + floor(particles[i].y / cutoff));

	/* insert particles into the bins */
  	insert_into_bins( particles, bins, n );

    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );

	#pragma omp parallel
    for( int step = 0; step < 1000; step++ )
    {
        //
        //  compute all forces
        //
        #pragma omp for
        for( int i = 0; i < n; i++ )
            particles[i].ax = particles[i].ay = 0;

		#pragma omp for
        for (int i = 0; i < num_bins; i++)
			go_through_neighbors(particles, bins, i);
        
        //
        //  move particles
        //
		#pragma omp for
		for (int i = 0; i < n; i++) 
            move_and_update( particles[i], i, globalIds[i]);		
		
		#pragma omp master
		insert_into_bins(particles, bins, n);
		
		#pragma omp barrier
		

#ifdef DEBUG
		#pragma omp master
		{
		/* checking that the number of particles doesnt change */
        int numparticles = 0;
        for(int i = 0; i < num_bins; i++)
        	numparticles += bins[i].num_particles;
        assert(numparticles==n);
        }
#endif 
        
        //
        //  save if necessary
        //
        #pragma omp master
        if( fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, n_threads = %d, simulation time = %g seconds\n", n, n_threads, simulation_time );
    
    free( particles );
    free( globalIds );
    free( bins );
    if( fsave )
        fclose( fsave );
    
    return 0;
}
