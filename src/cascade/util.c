/*	CMU Cascade Neural Network Simulator (CNNS)
	Network utilities

	v1.0
	Matt White  (mwhite+@cmu.edu)
	December 6, 1994

	This file contains some general functions to compute and train a
	cascade network.  Things like activation functions, error computation,
	etc.
*/

#include <math.h>
#include <string.h>

#include "cascade.h"
#include "toolkit.h"

/*	External Global Variable Declarations	*/

extern net_t        *cNet,
                    *nets;
extern df_t         *dFiles;
extern train_parm_t *cParms;
extern train_data_t *cTData;
extern error_data_t *cError;

extern int	    Ninputs,
		    Noutputs;
#ifdef CONNX
extern int          connx;
#endif
extern float        sigMax,
                    sigMin;

/*  FORWARD PASS -  Feed forward through the current network with inputs
    specified.  Call 'compute_outputs' to compute the outputs of the network.
*/

void forward_pass  ( float *inputs, boolean reset )
{
  int   i,j;
  float sum,
        *weights;

  cNet->values[0] = BIAS;
  
  for  ( i = 1 ; i <= Ninputs ; i++ )
    cNet->values[i] = inputs[i-1];

  for  ( i = Ninputs+1 ; i < cNet->Nunits ; i++ )  {
    sum     = 0.0;
    weights = cNet->weights[i];

    for  ( j = 0 ; j < i ; j++ )
      sum += cNet->values[j] * weights[j];
    if  ( cNet->recurrent && !reset )
      sum += cNet->values[i] * weights[i];

    cNet->values[i] = activation( cNet->unitTypes[i], sum );

#ifdef CONNX
    connx += i + (cNet->recurrent);
#endif
  }

  compute_outputs( );
}


/*  COMPUTE OUTPUTS -  Compute the output values of the network.
*/

void compute_outputs  ( void )
{
  int   i, j;
  float sum,
        *weights;

  for  ( i = 0 ; i < Noutputs ; i++ )  {
    sum     = 0.0;
    weights = cNet->outWeights[i];

    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      sum += cNet->values[j] * weights[j];
    cNet->outValues[i] = activation( cNet->outputTypes[i], sum );
  }

#ifdef CONNX
  connx += Noutputs * (cNet->Nunits+cNet->recurrent);
#endif
}


/*  COMPUTE ERROR -  Compute the error of the network, given a specific goal
    vector.  If alterStats is set, then the error statistics will be modified
    to reflect the current goal vector.  If alterSlopes is set, then the output
    slopes will be modified appropriately as well.
*/
    
void compute_error  ( float *goal, boolean alterStats, boolean alterSlopes,
		      boolean useEPrime, float threshold )
{
  float dif,
        error,
        val;
  int   i, j;

  for  ( i = 0 ; i < Noutputs ; i++ )  {
    val   = cNet->outValues[i];
    dif   = val - goal [i];
    error = (useEPrime) ? (dif*output_prime(cNet->outputTypes[i], val)) : dif;

    cError->errors[i] = error;
    
    if  ( alterStats )  {
      if  ( fabs( dif ) > threshold )
	cError->bits++;
      cError->sumSqDiffs += dif * dif;
      cError->sumSqError += error * error;
      cError->sumErr[i]  += error;
    }

    if  ( alterSlopes )
      for  ( j = 0 ; j < cNet->Nunits ; j++ )
	cTData->output.slopes[i][j] += error * cNet->values[j];
  }
}

void compute_normal_error  ( float *goal, int *error_count )
{
  float val;
  int   i;

  int max_val_idx, max_goal_idx;
  float max_val, max_goal;

  max_val_idx = 0;
  max_goal_idx = 0;

  max_val = cNet->outValues[0];
  max_goal = goal[0];

  for  ( i = 0 ; i < Noutputs ; i++ )  {
    val   = cNet->outValues[i];
    if( val > max_val) {
    	max_val = val;
    	max_val_idx = i;
    }

    if( goal[i] > max_goal) {
    	max_goal = goal[i];
    	max_goal_idx = i;
    }
  }

  if(max_val_idx != max_goal_idx){
	  (*error_count)++;
  }
}

/*  QUICKPROP -  Perform a quickprop update on a weight.  Pointers to the
    weight, delta, slope, and previous slope values should be passed to the
    function as should the epsilon, decay, mu and shrink factor values to use.
*/

void quickprop ( float *w, float *d, float *s, float *p, float epsilon,
		 float decay, float mu, float shrinkFactor )
{
  float nextStep = 0.0;

  *s += decay * *w;

  if  ( *d < 0.0 )  {
    if  ( *s > 0.0 )
      nextStep -= epsilon * *s;
    if  ( *s >= (shrinkFactor * *p) )
      nextStep += mu * *d;
    else
      nextStep += *d * *s / (*p - *s);
  } else if  ( *d > 0.0 )  {
    if  ( *s < 0.0 )
      nextStep -= epsilon * *s;
    if  ( *s <= (shrinkFactor * *p) )
      nextStep += mu * *d;
    else
      nextStep += *d * *s / (*p - *s);
  } else
    nextStep -= epsilon * *s;

  *w += nextStep;
  *d =  nextStep;
  *p =  *s;
  *s =  0.0;
}


/*  ACTIVATION -  Compute the activation level of a unit based on its type and
    the sum of its inputs.
*/

float activation  ( node_t unitType, float sum )
{
  float temp;

  switch ( unitType )  {
    case SIGMOID:    if ( sum < -15.0 )
                       return -0.5;
                     if ( sum > 15.0 )
		       return 0.5;
                     return 1.0 / (1.0 + exp( -sum )) - 0.5;
    case ASIGMOID:   if ( sum < -15.0 )
                       return 0.0;
                     if ( sum > 15.0 )
		       return 1.0;
                     return 1.0 / (1.0 + exp( -sum ));
    case LINEAR:     return sum;
    case VARSIGMOID: if ( sum < -15.0 )
                       return sigMin;
                     if ( sum > 15.0 )
		       return sigMax;
                     return( (sigMax - sigMin) /
			     (1.0 + exp( -sum )) + sigMin );
    case GAUSSIAN:   temp = -0.5 * sum * sum;
                     if  ( temp < -75.0 )
		       return 0.0;
                     else
		       return exp( temp );
    }
}


/*	ACTIVATION PRIME -  Compute the activation prime value of a unit.  This
	version of the function does NOT use the sigmoid prime offset to
	increase learning speed because the offset confuses cascor's 
	correlation machinery.
*/

float activation_prime  ( node_t unitType, float value, float sum )
{
  switch  ( unitType )  {
    case SIGMOID:    return( 0.25 - value * value );
    case ASIGMOID:   return( value * (1.0 - value) );
    case LINEAR:     return 1.0;
    case VARSIGMOID: return( (value - sigMin) *
			     ( 1.0 - (value - sigMin) ) /
			     ( sigMax - sigMin ) );
    case GAUSSIAN:   return( sum * (-value) );
    }
}


/*  OUTPUT PRIME -  Compute the activation prime of a unit, based on its type
    and value.  Use this function only on output units, since it uses the
    sigmoid prime offset to eliminate flat spot and this tends to confuse
    hidden units.
*/

float output_prime  ( node_t outType, float value )
{
  switch  ( outType )  {
    case SIGMOID:  return( cParms->outPrimeOffset + 0.25 - value * value );
    case ASIGMOID: return( cParms->outPrimeOffset + value * (1.0 - value ) );
    case LINEAR:   return 1.0;
    case VARSIGMOID: return( cParms->outPrimeOffset +
			     (value - sigMin) *
			     ( 1.0 - (value - sigMin) ) /
			     ( sigMax - sigMin ) );    }
}


/*  RANDOM WEIGHT -  Return a random floating point number with a value of
    +/- x.
*/

float random_weight ( float x )
{
  return( x * ((float)(random() % 1000) / 500.0) - x );
}


/*	SYNC -  Synchronize a network to a data file so that its outputs are
	of the correct types (and whatever other changes need to be made before
	training).
*/

void sync  ( net_t *net, data_file_t *dFile )
{
  int    i,j,k;
  char   *fn = "Sync Net";
  cvrt_t *map;

  /*  Match the output types  */
  for  ( i = 0 ; i < net->Noutputs ; i++ )
    if  ( dFile->outputType[i] == CONT )
      net->outputTypes[i] = LINEAR;
    else  {
      if  ( (dFile->binPos == 0.5) && (dFile->binNeg == -0.5) )
	net->outputTypes[i] = SIGMOID;
      else if  ( (dFile->binPos == 1.0) && (dFile->binNeg == 0.0) )
	net->outputTypes[i] = ASIGMOID;
      else {
	net->sigmoidMax = dFile->binPos;
	net->sigmoidMin = dFile->binNeg;
	net->outputTypes[i] = VARSIGMOID;
      }
    }

  /*  Free up the network's old conversions maps  */
  if  ( net->inputMap != NULL )  {
    for  ( i = 0 ; i < net->Ninputs ; i++ )  {
      map = &(net->inputMap[i]);
      for  ( j = 0 ; j < map->Nenums ; j++ )  {
	free( map->equivs[j] );
	free( map->enums[j] );
      }
      free( map->equivs );
      free( map->enums );
      free( map->unknown );
    }
    free( net->inputMap );
  }

  if ( net->outputMap != NULL )  {
    for  ( i = 0 ; i < net->Noutputs ; i++ )  {
      map = &(net->outputMap[i]);
      for  ( j = 0 ; j < map->Nenums ; j++ )  {
	free( map->equivs[j] );
	free( map->enums[j] );
      }
      free( map->equivs );
      free( map->enums );
      free( map->unknown );
    }
    free( net->outputMap );
  }

  /*  Copy the conversion map from the data file to the network  */
  net->inputMap = (cvrt_t *) alloc_mem( net->Ninputs, sizeof( cvrt_t ), fn );
  for  ( i = 0 ; i < net->Ninputs ; i++ )  {
    map = &(net->inputMap[i]);
    map->Nenums = dFile->inputMap[i].Nenums;
    map->Nunits = dFile->inputMap[i].Nunits;
    map->unknown = (float *)alloc_mem( map->Nunits, sizeof( float ), fn );

    if  ( map->Nenums > 0 )  {
      map->enums  = (char **)alloc_mem( map->Nenums, sizeof( char * ), fn );
      map->equivs = (float **)alloc_mem( map->Nenums, sizeof( float * ), fn );

      for  ( j = 0 ; j < map->Nenums ; j++ )  {
	map->enums[j]  = strdup( dFile->inputMap[i].enums[j] );
	map->equivs[j] = (float *)alloc_mem(map->Nunits, sizeof( float ), fn);
	for  ( k = 0 ; k < map->Nunits ; k++ )
	  map->equivs[j][k] = dFile->inputMap[i].equivs[j][k];
      }
    }
    for  ( j = 0 ; j < map->Nunits ; j++ )
      map->unknown[j] = dFile->inputMap[i].unknown[j];
  }

  net->outputMap = (cvrt_t *) alloc_mem( net->Noutputs, sizeof( cvrt_t ), fn );
  for  ( i = 0 ; i < net->Noutputs ; i++ )  {
    map = &(net->outputMap[i]);
    map->Nenums = dFile->outputMap[i].Nenums;
    map->Nunits = dFile->outputMap[i].Nunits;
    map->unknown = (float *)alloc_mem( map->Nunits, sizeof( float ), fn );

    if  ( map->Nenums > 0 )  {
      map->enums  = (char **)alloc_mem( map->Nenums, sizeof( char * ), fn );
      map->equivs = (float **)alloc_mem( map->Nenums, sizeof( float * ), fn );

      for  ( j = 0 ; j < map->Nenums ; j++ )  {
	map->enums[j]  = strdup( dFile->outputMap[i].enums[j] );
	map->equivs[j] = (float *)alloc_mem(map->Nunits, sizeof( float ), fn);
	for  ( k = 0 ; k < map->Nunits ; k++ ) 
	  map->equivs[j][k] = dFile->outputMap[i].equivs[j][k];
      }
    }
    for  ( j = 0 ; j < map->Nunits ; j++ )
      map->unknown[j] = dFile->outputMap[i].unknown[j];
  }
}


/*  SELECT NET -  Locate the indicated network in memory.  If the network is
    not found, return a NULL.
*/

net_t *select_net ( char *name )
{
  net_t *index;

  index = nets;
  while ( index != NULL )  {
    if  ( !strcasecmp( index->name, name ) )
      return index;
    index = index->next;
  }

  return NULL;
}


/*  ADD NET -  Adds a network to the linked list of those stored in memory.
*/

void add_net ( net_t *newNet )
{
  newNet->next = nets;
  nets = newNet;
}


/*  DEL NET -  Removes a network from memory.
*/
boolean del_net ( char *netName )
{
  net_t *index,
        *pindex;

  index  = nets;
  pindex = NULL;
  while ( index != NULL )
    if  ( !strcmp( index->name, netName ) )  {
      if ( pindex == NULL )
	nets = index->next;
      else
	pindex->next = index->next;
      break;
    }  else  {
      pindex = index;
      index  = index->next;
    }
  
  if ( index == NULL )
    return FALSE;

  free_net( &index );
  return TRUE;
}


/*  SELECT DATA -  Similar to SELECT NET, except that we locate a parsed data
    file instead of a network (of course).
*/

data_file_t *select_data ( char *filename )
{
  df_t *index;

  index = dFiles;
  while ( index != NULL )  {
    if  ( !strcmp( index->data->filename, filename ) )
      return index->data;
    index = index->next;
  }

  return NULL;
}


/*  ADD DATA FILE -  Add a data file to the linked list of data files stored
    in memory.
*/

void add_data_file ( data_file_t *dFile )
{
  df_t *temp;

  temp = (df_t *)alloc_mem (1, sizeof( df_t ), "Add Data File");
  temp->next = dFiles;
  temp->data = dFile;
  dFiles     = temp;
}


/*  DEL DATA FILE -  Remove a data file from memory altogether.
*/

boolean del_data_file ( char *filename )
{
  df_t *index,
       *pindex;

  index  = dFiles;
  pindex = NULL;
  while ( index != NULL )
    if  ( !strcmp( index->data->filename, filename ) )  {
      if ( pindex == NULL )
	dFiles = index->next;
      else
	pindex->next = index->next;
      break;
    }  else  {
      pindex = index;
      index  = index->next;
    }
  
  if ( index == NULL )
    return FALSE;

  free_data( &(index->data) );
  return TRUE;
}


/*	NTOA - Return a character string equivilant of the node type passed 
	to it.
*/

char *ntoa  ( node_t value )
{
  switch ( value )  {
    case UNDEFINED:  return "Undefined";
    case VARIED:     return "Varied";
    case SIGMOID:    return "Sigmoid";
    case ASIGMOID:   return "ASigmoid";
    case LINEAR:     return "Linear";
    case GAUSSIAN:   return "Gaussian";
    default:         return "(illegal)";
    }
}


/*	ALTOA -  Return the character string equivelant of the algorithm type
	passed to the function.
*/

char *altoa  ( algo_t value )
{
  switch ( value )  {
    case CASCOR:   return "Cascade Correlation";
    case CASCADE2: return "Cascade-2";
    default:       return "(illegal)";
    }
}


/*	ETOA -  Return the character string associated with the error type
	passed.
*/

char *etoa  ( error_t value )
{
  switch ( value )  {
    case INDEX:  return "Index";
    case BITS:   return "Bits";
    default:     return "(illegal)";
    }
}


/*	STOA -  Converts a status type to a character string.
*/

char *stoa  ( status_t status )
{
  switch  ( status )  {
    case WIN:      return "Win";
    case TRAINING: return "Training";
    case STAGNANT: return "Stagnant";
    case TIMEOUT:  return "Timeout";
    case LOSS: 	   return "Loss";
    default:       return "(invalid)";
    }
}

/*	ATOAL -  Extract an algorithm type from a character string.
*/

algo_t atoal  ( char *value )
{
  if  ( !strcasecmp( value, "cascade2" ) || !strcasecmp( value, "cascade-2" ) )
    return CASCADE2;
  return CASCOR;
}


/*	ATOE -  Extract an error measure from the character string passed.
*/

error_t atoe  ( char *value )
{
  if  ( !strcasecmp( value, "index" ) )
    return INDEX;
  return BITS;
}


/*	ATON -  Extract a node type from the character string.
*/

node_t aton  ( char *value )
{
  if  ( !strcasecmp( value, "sigmoid" ) )
    return SIGMOID;
  if  ( !strcasecmp( value, "asigmoid" ) )
    return ASIGMOID;
  if  ( !strcasecmp( value, "gaussian" ) )
    return GAUSSIAN;
  if  ( !strcasecmp( value, "linear" ) )
    return LINEAR;
  if  ( !strcasecmp( value, "varied" ) )
    return VARIED;
  return UNDEFINED;
}
