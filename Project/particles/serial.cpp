/*Particle simulator.

	A sequential program that runs in time O(n), where n
	is the number of particles.
	Instead of comparing every particle with every other particle
	to see if they are in range, only a subset of the particles are 
	gone through when applying the repulsive force between the particles.
	
To run in Linux:
make -f Makefile_p serial
./serial

	
*/


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "common.h"

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
        printf( "-n <int> to set the number of particles\n" );
        printf( "-o <filename> to specify the output file name\n" );
        return 0;
    }
    
    int n = read_int( argc, argv, "-n", 1000 );

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
  	insert_into_bins(particles, bins, n);
    
    //
    //  simulate a number of time steps
    //
    double simulation_time = read_timer( );
    for( int step = 0; step < NSTEPS; step++ )
    {
        //
        //  compute forces
        //
        for( int i = 0; i < n; i++ )
            particles[i].ax = particles[i].ay = 0;        
        
        for (int i = 0; i < num_bins; i++)
			go_through_neighbors(particles, bins, i);

		for (int i = 0; i < n; i++) 
            move_and_update( particles[i], i, globalIds[i] );		
		
		insert_into_bins(particles, bins, n);

#ifdef DEBUG
		/* checking that the number of particles doesnt change */
        int numparticles = 0;
        for(int i = 0; i < num_bins; i++)
        	numparticles += bins[i].num_particles;
        assert(numparticles==n);
#endif        
        //
        //  save if necessary
        //
        if( fsave && (step%SAVEFREQ) == 0 )
            save( fsave, n, particles );
    }
    simulation_time = read_timer( ) - simulation_time;
    
    printf( "n = %d, simulation time = %g seconds\n", n, simulation_time );
    
    free( particles );
    if( fsave )
        fclose( fsave );
    
    return 0;
}
