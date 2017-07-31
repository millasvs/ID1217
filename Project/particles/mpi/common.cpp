#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"

double size;
int num_rows, num_bins;
extern int *globalIds;


//
//  timer
//
double read_timer( )
{
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


//
//  keep density constant
//
void set_size( int n )
{
    size = sqrt( density * n );
    /* number of columns = number of rows */
	num_rows = (int)ceil(size / cutoff);
	num_bins = num_rows * num_rows; 
}

/* inserting the particles into the appropriate bins */
void insert_into_bins(particle_t* particles, bin_t* bins, int** bins_particle_ids, int n) {
	
	/* remove particles from bins */
	for (int i = 0; i < num_bins; i++)
		bins[i].num_particles = 0;

	for (int i = 0; i < n; i++) {
		int id = globalIds[i];
		bins_particle_ids[id][bins[id].num_particles] = i;	
		bins[id].num_particles++;
	}
}

/* for each particle in the bin given as input argument: 
   go through all the particles in the current and the 
   neighboring bins and apply force between them */
void go_through_neighbors(particle_t* particles, bin_t* bins, int** bins_particle_ids, int binId) {

 int d_col[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
 int d_row[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
 
 bin_t* bin = &bins[binId];
 
 for(int i = 0; i < bin->num_particles; i++){
	int col = i % num_rows; 
	int row = (i - col) / num_rows; 
	for(int k = 0; k < 9; k++){
		int new_col = col + d_col[k]; 
		int new_row = row + d_row[k];
		if(new_col >= 0 && new_row >= 0 && new_col < num_rows && new_row < num_rows) {
			int neighbor_id = new_col + new_row * num_rows;				
			int neighbor_num_particles = bins[neighbor_id].num_particles;
			for(int j = 0; j < neighbor_num_particles; j++)
				apply_force(particles[bins_particle_ids[binId][i]], particles[bins_particle_ids[neighbor_id][j]]);
		}
	}
 }
}



/* for each particle in the bin given as input argument: 
   go through all the particles in the current and the 
   neighboring bins and apply force between them */
void go_through_neighbors(particle_t* particles, bin_t* bins, int** bins_particle_ids, int first, int last, int binId) {
 
 int d_col[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
 int d_row[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
 
 bin_t* bin = &bins[binId];
 
 for(int i = 0; i < bin->num_particles; i++){
  	if(bins_particle_ids[binId][i] >= first && bins_particle_ids[binId][i] < last){
		int col = i % num_rows; 
		int row = (i - col) / num_rows; 
		for(int k = 0; k < 9; k++){
			int new_col = col + d_col[k]; 
			int new_row = row + d_row[k];
			if(new_col >= 0 && new_row >= 0 && new_col < num_rows && new_row < num_rows) {
				int neighbor_id = new_col + new_row * num_rows;				
				int neighbor_num_particles = bins[neighbor_id].num_particles;
				for(int j = 0; j < neighbor_num_particles; j++)
					apply_force(particles[bins_particle_ids[binId][i]], particles[bins_particle_ids[neighbor_id][j]]);
			}
		}
	}
 } 
}

void move_and_update( particle_t &p, int  id){
	//
	//  slightly simplified Velocity Verlet integration
	//  conserves energy better than explicit Euler method
	//
	p.vx += p.ax * dt;
	p.vy += p.ay * dt;
	p.x  += p.vx * dt;
	p.y  += p.vy * dt;

	//
	//  bounce from walls
	//
	while( p.x < 0 || p.x > size )
	{
		p.x  = p.x < 0 ? -p.x : 2*size-p.x;
		p.vx = -p.vx;
	}
	while( p.y < 0 || p.y > size )
	{
		p.y  = p.y < 0 ? -p.y : 2*size-p.y;
		p.vy = -p.vy;
	}

	p.ax = 0; 
	p.ay = 0;

	globalIds[id] = (int)(floor(p.x / cutoff) * num_rows + floor(p.y / cutoff));
}



void move_and_update( particle_t &p, int  id, int &globalId){
	//
	//  slightly simplified Velocity Verlet integration
	//  conserves energy better than explicit Euler method
	//
	p.vx += p.ax * dt;
	p.vy += p.ay * dt;
	p.x  += p.vx * dt;
	p.y  += p.vy * dt;

	//
	//  bounce from walls
	//
	while( p.x < 0 || p.x > size )
	{
		p.x  = p.x < 0 ? -p.x : 2*size-p.x;
		p.vx = -p.vx;
	}
	while( p.y < 0 || p.y > size )
	{
		p.y  = p.y < 0 ? -p.y : 2*size-p.y;
		p.vy = -p.vy;
	}

	p.ax = 0; 
	p.ay = 0;

	globalId = (int)(floor(p.x / cutoff) * num_rows + floor(p.y / cutoff));
}

//
//  Initialize the particle positions and velocities
//
void init_particles( int n, particle_t *p )
{
    srand48( time( NULL ) );
        
    int sx = (int)ceil(sqrt((double)n));
    int sy = (n+sx-1)/sx;
    
    int *shuffle = (int*)malloc( n * sizeof(int) );
    for( int i = 0; i < n; i++ )
        shuffle[i] = i;
    
    for( int i = 0; i < n; i++ ) 
    {
        //
        //  make sure particles are not spatially sorted
        //
        int j = lrand48()%(n-i);
        int k = shuffle[j];
        shuffle[j] = shuffle[n-i-1];
        
        //
        //  distribute particles evenly to ensure proper spacing
        //
        p[i].x = size*(1.+(k%sx))/(1+sx);
        p[i].y = size*(1.+(k/sx))/(1+sy);

        //
        //  assign random velocities within a bound
        //
        p[i].vx = drand48()*2-1;
        p[i].vy = drand48()*2-1;
    }
    free( shuffle );
}

//
//  interact two particles
//
void apply_force( particle_t &particle, particle_t &neighbor ){
 	
	double dx = neighbor.x - particle.x;
	double dy = neighbor.y - particle.y;
	double r2 = dx * dx + dy * dy;
	if( r2 > cutoff*cutoff )
		return;
	

	r2 = fmax( r2, min_r*min_r );
	double r = sqrt( r2 );

	//
	//  very simple short-range repulsive force
	//
	double coef = ( 1 - cutoff / r ) / r2 / mass;
	particle.ax += coef * dx;
	particle.ay += coef * dy;
	
}

//
//  integrate the ODE
//
void move( particle_t &p ){
    //
    //  slightly simplified Velocity Verlet integration
    //  conserves energy better than explicit Euler method
    //
    p.vx += p.ax * dt;
    p.vy += p.ay * dt;
    p.x  += p.vx * dt;
    p.y  += p.vy * dt;

    //
    //  bounce from walls
    //
    while( p.x < 0 || p.x > size )
    {
        p.x  = p.x < 0 ? -p.x : 2*size-p.x;
        p.vx = -p.vx;
    }
    while( p.y < 0 || p.y > size )
    {
        p.y  = p.y < 0 ? -p.y : 2*size-p.y;
        p.vy = -p.vy;
    }
}

//
//  I/O routines
//
void save( FILE *f, int n, particle_t *p ){
    static bool first = true;
    if( first )
    {
        fprintf( f, "%d %g\n", n, size );
        first = false;
    }
    for( int i = 0; i < n; i++ )
        fprintf( f, "%g %g\n", p[i].x, p[i].y );
}

//
//  command line option processing
//
int find_option( int argc, char **argv, const char *option ){
    for( int i = 1; i < argc; i++ )
        if( strcmp( argv[i], option ) == 0 )
            return i;
    return -1;
}

int read_int( int argc, char **argv, const char *option, int default_value ){
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return atoi( argv[iplace+1] );
    return default_value;
}

char *read_string( int argc, char **argv, const char *option, char *default_value )
{
    int iplace = find_option( argc, argv, option );
    if( iplace >= 0 && iplace < argc-1 )
        return argv[iplace+1];
    return default_value;
}
