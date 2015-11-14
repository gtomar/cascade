/*	CMU Cascade Neural Network Simulator (CNNS)
	Caching Utilities
	
	v1.0
	Matt White  (mwhite+@cmu.edu)
	May 25, 1995

	This file contains functions for the allocation and computation of the
	caches used in Cascade-Correlation and Cascade-2 algorithms.
*/

#include <stdio.h>
#include "toolkit.h"
#include "cascade.h"

#ifdef CONNX
extern int connx;
#endif

/*	BUILD CACHE -  Allocate memory for the cache.  If not enough memory is
	available, deallocate the partial cache and return gracefully.
*/

boolean build_cache  ( int maxUnits, int Noutputs, int Npts,
		       float ***valCache, float ***errCache )
{
  int i;
  
  if  ( (((*valCache) = (float **)malloc(Npts*sizeof( float * ))) == NULL) ||
        (((*errCache) = (float **)malloc(Npts*sizeof( float * ))) == NULL) )  {
    free_cache( valCache, errCache, Npts );
    printf  ("ERROR: Insufficient memory for cache, shutting cache down.\n");
    return FALSE;
  }

  for  ( i = 0 ; i < Npts ; i++ )
    if  ((((*valCache)[i] =(float *)malloc(maxUnits*sizeof(float))) == NULL) ||
	 (((*errCache)[i] =(float *)malloc(Noutputs*sizeof(float))) == NULL)) {
      free_cache( valCache, errCache, Npts );
      printf  ("ERROR: Insufficient memory for cache, shutting cache down.\n");
      return FALSE;
    }

  return TRUE;
}


/*	FREE CACHE -  Deallocate the memory associated with a cache.
*/

void free_cache  ( float ***valCache, float ***errCache, int Npts )
{
  int i;

  for  ( i = 0 ; i < Npts ; i++ )  {
    if  ( (*valCache)[i] != NULL )
      free( (*valCache)[i] );
    if  ( (*errCache)[i] != NULL )
      free( (*errCache)[i] );
  }

  if  ( *valCache != NULL )
    free( *valCache );
  if  ( *errCache != NULL )
    free( *errCache );

  *valCache = NULL;
  *errCache = NULL;
}


/*	COMPUTE CACHE -  Compute the initial values of the cache.  This
	function should be called once before the run has started.
*/

void compute_cache  ( int Ninputs, data_set_t *dSet, float **valCache )
{
  int i,j;

  for  ( i = 0 ; i < dSet->Npts ; i++ )  {
    valCache[i][0] = BIAS;
    for  ( j = 1 ; j <= Ninputs ; j++ )
      valCache[i][j] = dSet->data[i].inputs[j-1];
  }
}


/*	RECOMPUTE CACHE -  Recompute the values of a cache for a new unit.
	This function should be called every time a unit is added to the
	network.
*/

void recompute_cache  ( int unitNum, net_t *net, data_set_t *dSet, 
		        float **valCache )
{
  float sum;
  int   i,j;

  for  ( i = 0 ; i < dSet->Npts ; i++ )  {
    sum = 0.0;

    for  ( j = 0 ; j < unitNum ; j++ )
      sum += valCache[i][j] * net->weights[unitNum][j];
    if   ( (net->recurrent) && !(dSet->data[i].reset) )
      sum += ((i>0)?valCache[i-1][unitNum]:0.0) * 
	      net->weights[unitNum][unitNum];

    valCache[i][unitNum] = activation( net->unitTypes[unitNum], sum );

#ifdef CONNX
    connx += unitNum + (net->recurrent);
#endif
  }
}

