/*	CMU Cascade Neural Network Simulator  (CNNS)
	Cascade-2 candidate training code

	v1.0
	Matt White  (mwhite+@cmu.edu)
	May 25, 1995

	This is the code necessary for the training of candidate units using
	the Cascade-2 algorithm.  This code is a re-engineered version of the
	C port, by Michael Kingsley, of the original Lisp code by Scott
	Fahlman.
*/


#include "cascade.h"
#include "toolkit.h"

/*	External Global Variable Declarations	*/

extern net_t        *cNet;
extern train_parm_t *cParms;
extern train_data_t *cTData;
extern data_set_t   *cDSet;
extern error_data_t *cError;

extern int          Noutputs,
	            Ncand;
extern boolean      recurrent,
	            interruptPending;

#ifdef CONNX
extern int          connx;
#endif


/*	C2 TRAIN CAND -  Train a new pool of candidates.  Training continues
	until either the maximum number of training epochs for a candidate pool
	has been reached (TIMEOUT), or a specific number of epochs pass without
	noticeable improvement (STAGNANT).  The results of training are
	returned as the function's return value.
*/

status_t  c2_train_cand  ( void )
{
  int   quitEpoch = 0,
        i;
  float backslide = -1.0e20,
        target    = 0.0;

  for  ( i = 0 ; i < cParms->candidateParm.epochs ; i++ )  {
    c2_cand_epoch( );      /*  Train the cands for an epoch  */

    adjust_ci_weights( );  /*  Adjust all the weights and find a favorite  */
    adjust_co_weights( );  /* unit.  */
    c2_find_best_cand( );

    cNet->epochsTrained++;

    if  ( interruptPending ) handle_interrupt( cTData, cDSet->Npts );

    /*  Check for stagnation  */
    if  ( (cTData->candBestScore > target) || 
	  (cTData->candBestScore < backslide) )  {
      target    = cTData->candBestScore * 
	          (cParms->candidateParm.changeThreshold+1);
      backslide = cTData->candBestScore *
	          (1-cParms->candidateParm.changeThreshold);
      quitEpoch = cNet->epochsTrained + cParms->candidateParm.patience;
    } else if  ( cNet->epochsTrained == quitEpoch )  {
      return STAGNANT;
    }
  }

  return TIMEOUT;
}


/*	C2 CAND EPOCH -  Train the candidates for an epoch.  If the cache is
	not on, run a forward pass and compute error.  Otherwise, retrieve
	activation values from the cache.  Use these values to calculate the
	slopes and scores of each of the candidates.
*/

void  c2_cand_epoch  ( void )
{
  int i,j;

  /*  Initialize for the epoch  */
  for  ( i = 0 ; i < Ncand ; i++ )  {
    cTData->candScores[i] = cError->sumSqDiffs;
    if  ( recurrent )  {
      cTData->candPrevValues[i] = 0.0;
      for  ( j = 0 ; j < cNet->Nunits+1 ; j++ )
	cTData->candDVdW[i][j] = 0.0;
    }
  }

  /*  Compute the epoch  */
  for  ( i = 0 ; i < cDSet->Npts ; i++ )  {
    if  ( cParms->useCache )  {
      cNet->values   = cTData->valCache[i];
      cError->errors = cTData->errCache[i];
    } else {
      forward_pass( cDSet->data[i].inputs, cDSet->data[i].reset );
      compute_error( cDSet->data[i].outputs, FALSE, FALSE, FALSE,
	             cParms->scoreThreshold );
    }
    c2_compute_slopes( cDSet->data[i].outputs, cDSet->data[i].reset );
  }
}


/*	C2 COMPUTE SLOPES -  Use the precomputed error values to compute the
	slopes of the error for each individual candidate unit.  This will
	later be used to update the input weights to each of these candidates.

	Note:  This function is extremely compute-intensive.  If you want to
	spend some time optimizing, this is a good place to start.  I think
	I've done everything reasonable, but if you find something, please
	email me at 'neural-bench@cs.cmu.edu', so that I can modify the release
	version.
*/

void  c2_compute_slopes  ( float *goal, boolean reset )
{
  float sum,          /*  The unit's sum input  */
        dsum,         /*  dVdW calculated for a point  */
        dif,          /*  Difference between the unit's value and target  */
        value,        /*  Computed activation for a unit  */
        actPrime,     /*  Computed activation prime for a unit  */
        errSum,       /*  The sum of the error prime collected over weights  */
        weight,       /*  The weight in question  */
        *cIWeights,   /*  Current In Weights  */
        *cISlopes,    /*  Current In Slopes  */
        *cOWeights,   /*  Current Out Weights  */
        *cOSlopes,    /*  Current Out Slopes  */
        goalDir,      /*  The direction the goal lies in  */
        difDir;       /*  The direction our difference with the goal lies in */
  int   i, j;

  for  ( i = 0 ; i < Ncand ; i++ )  {
    sum       = 0.0;                        /*  Initialize local variables  */
    errSum    = 0.0;
    cOWeights = cTData->candOut.weights[i];
    cOSlopes  = cTData->candOut.slopes[i];
    cIWeights = cTData->candIn.weights[i];
    cISlopes  = cTData->candIn.slopes[i];

    /*  Compute the value of the unit  */
    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      sum += cNet->values[j] * cIWeights[j];
    if  ( recurrent && !reset )
      sum += cTData->candPrevValues[i] * cIWeights[cNet->Nunits];
#ifdef CONNX
    connx += cNet->Nunits+recurrent;
#endif
    value    = activation( cTData->candTypes[i], sum );
    actPrime = activation_prime( cTData->candTypes[i], value, sum);

    /*  Compute the slopes for the outgoing weights  */
    for  ( j = 0 ; j < Noutputs ; j++ )  {
      weight  = cOWeights[j];
      dif     = ( weight * value ) - cError->errors[j];
      goalDir = ( goal[j] < 0.0 ) ? -1.0 : 1.0;
      difDir  = ( dif > 0.0 ) ? -1.0 : 1.0;

      if  ( !( cParms->overshootOK && (goalDir == difDir) ) )  {
	cTData->candScores[i] -= dif * dif;
	cOSlopes[j]           += dif * value;
	errSum                += dif * weight;
      }
    }
    errSum *= actPrime;

    /*  First approximation of the slopes coming into the unit  */
    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      cISlopes[j] += errSum * cNet->values[j];

    /*  Compute the influences of the recurrent connection  */
    if  ( recurrent )  {
      for ( j = 0 ; j < cNet->Nunits ; j++ )  {
	if  ( reset )
	  cTData->candDVdW[i][j] = 0.0;
	dsum = actPrime * (cNet->values[j] + 
			   (cIWeights[cNet->Nunits] * cTData->candDVdW[i][j]));
	cISlopes[j] += errSum * dsum;
	cTData->candDVdW[i][j] = dsum;
      }

      if  ( !reset )  {
	dsum = actPrime * (cTData->candPrevValues[i] + 
			   (cIWeights[cNet->Nunits] * 
			    cTData->candDVdW[i][cNet->Nunits]));
	cISlopes[cNet->Nunits] += errSum * dsum;
	cTData->candDVdW[i][cNet->Nunits] = dsum;
      }
      
      cTData->candPrevValues[i] = value;
    }
  }
}


/*	C2 FIND BEST CAND -  Search through the candidate scores to find the
	candidate with the best score.
*/

void  c2_find_best_cand  ( void )
{
  int i;

  cTData->candBestScore = cTData->candScores[0];
  cTData->candBest      = 0;

  for  ( i = 1 ; i < Ncand ; i++ )
    if  ( cTData->candScores[i] > cTData->candBestScore )  {
      cTData->candBestScore = cTData->candScores[i];
      cTData->candBest      = i;
    }
}
