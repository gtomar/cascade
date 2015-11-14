/*	CMU Cascade Neural Network Simulator (CNNS)
	Cascade-Correlation candidate training code

	v1.0
	Matt White  (mwhite+@cmu.edu)
	December 5, 1994

	This is the code necessary for the training of candidate units using
	the Cascade-Correlation algorithm developed by Scott Fahlman and
	Christian Lebiere.

	Note:  Ideally, after each adjustment of the candidate weights, we
	would run two epochs.  The first would just determin the correlations
	between the candidate unit outputs and the residual error.  Then in a
	second pass, we would adjust each candidate's input weights so as to
	maximize the absolute value of the correlation.  We need to know the
	direction to tune the input weights.

	Since this training method doubles the number of epochs required for
	training the candidate, we cheat slightly and use the correlation 
	values computed BEFORE the most recent weight update.  This combines
	the two epochs, saving us almost a factor of two.  To bootstrap the
	process, we begin with a single epoch that computes only the
	correlation.

	Since we look only at the sign of the correlation after the first ideal
	epoch and since that sign should change very infrequently, this is
	probably ok.  But keep a lookout for pathelogical situations in which
	this might cause oscillation.
*/

#include <math.h>

#include "toolkit.h"
#include "cascade.h"

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

/*	CASCOR TRAIN CAND -  Train a pool of candidate units using the
	Cascade-Correlation algorithm.  Returns a value of TIMEOUT if 
	'cParms->candidateParm.epochs' epochs elapse while the candidates are
	still improving significantly (users should try to avoid this 
	condition).  Otherwise, returns a value of STAGNANT whenever
	'cParms->candidateParm.patience' epochs elapse without improvement in
	the candidates.
*/

status_t cascor_train_cand  ( void )
{
  float lastScore = 0.0;
  int   quitEpoch = 0,
        i,j,k;

  for  ( i = 0 ; i < Noutputs ; i++ )
    cError->sumErr[i] /= cDSet->Npts;
  cascor_correlation_epoch( );
  for  ( i = 1 ; i < cParms->candidateParm.epochs ; i++ )  {
    cascor_cand_epoch( );
    adjust_ci_weights( );

    cascor_adjust_correlations( );

    if  ( interruptPending ) handle_interrupt( cTData, cDSet->Npts );

    cNet->epochsTrained++;

    if  ( i == 1 )
      lastScore = cTData->candBestScore;
    else if  ( fabs( cTData->candBestScore - lastScore ) >
               ( lastScore * cParms->candidateParm.changeThreshold ) )  {
      quitEpoch = cNet->epochsTrained + cParms->candidateParm.patience;
      lastScore = cTData->candBestScore;
    } else if  ( cNet->epochsTrained == quitEpoch )
      return STAGNANT;
  }

  return TIMEOUT;
}


/*	CASCOR CORRELATION EPOCH -  Run a single epoch to compute correlation
	values only.  After this epoch, we will compute correlation values
	during training.  This is a slight deviation from the 'ideal' candidate
	training cycle that saves almost half the epochs.  For more
	information, see the notes at the top of this file.
*/

void cascor_correlation_epoch  ( void )
{
  int i;

  for  ( i = 0 ; i < cDSet->Npts ; i++ )  {
    if  ( cParms->useCache )  {
      cNet->values   = cTData->valCache[i];
      cError->errors = cTData->errCache[i];
    } else {
      forward_pass  ( cDSet->data[i].inputs, cDSet->data[i].reset );
      compute_error ( cDSet->data[i].outputs, FALSE, FALSE, TRUE, 
                      cParms->scoreThreshold );
    }
    cascor_compute_correlations( cDSet->data[i].reset );
  }

  cascor_adjust_correlations( );
  cNet->epochsTrained++;
}


/*	CASCOR COMPUTE CORRELATIONS -  For the current training pattern,
	compute the activation of each candidate unit.  Then begin to compute
	the correlation value for that unit.  Activation values and error from
	the rest of the network have already been computed elsewhere.
*/

void cascor_compute_correlations  ( boolean reset )
{
  float sum,
        val,
        *cWeights,
        *cCorr;
  int   i, j;

  for  ( i = 0 ; i < Ncand ; i++ )  {
    sum        = 0.0;
    cWeights   = cTData->candIn.weights[i];
    cCorr      = cTData->candCorr[i];

    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      sum += cWeights[j] * cNet->values[j];

    if  ( recurrent && !reset )
      sum += cWeights[cNet->Nunits] * cTData->candPrevValues[i];

#ifdef CONNX
    connx += cNet->Nunits+recurrent;
#endif

    val                    =  activation( cTData->candTypes[i], sum );
    cTData->candValues[i]  =  val;
    cTData->candSumVals[i] += val;

    for  ( j = 0 ; j < Noutputs ; j++ )
      cCorr[j] += val * cError->errors[j];
  }
}


/*	CASCOR CAND EPOCH -  Train the candidates for an epoch.  If the cache
	is not on, run a forward pass and compute error.  Otherwise, retrieve
	activation values from the cache.  Use these values to calculate the
	slopes and correlation values of each of the candidates.
*/

void cascor_cand_epoch  ( void )
{
  int i;

  for  ( i = 0 ; i < cDSet->Npts ; i++ )  {
    if  ( cParms->useCache )  {
      cNet->values   = cTData->valCache[i];
      cError->errors = cTData->errCache[i];
    } else {
      forward_pass( cDSet->data[i].inputs, cDSet->data[i].reset );
      compute_error( cDSet->data[i].outputs, FALSE, FALSE, TRUE,
	             cParms->scoreThreshold );
    }
    cascor_compute_slopes( cDSet->data[i].reset );
  }
}


/*	CASCOR COMPUTE SLOPES -  Use the precomputed correlation values to
	compute the slopes of the error for each individual candidate unit.
	This will later be used to update the input weights to each of these
	candidates.

	Note:  This function is extremely compute-intensive.  If you want to
	some time optimizing, this is a good place to start.  I think that I've
	done everything reasonable, but if your find something, please email me
	at 'neural-bench@cs.cmu.edu', so that I can modify the release version.
*/

void cascor_compute_slopes ( boolean reset )
{
  float sum,
        change,
        value,
        actPrime,
        error,
        direction,
        *cWeights,
        *cCorr,
        *cPCorr,
        *cSlopes;
  int   i,j;

  for  ( i = 0 ; i < Ncand ; i++ )  {
    sum      = 0.0;
    change   = 0.0;
    cWeights = cTData->candIn.weights[i];
    cSlopes  = cTData->candIn.slopes[i];
    cCorr    = cTData->candCorr[i];
    cPCorr   = cTData->candPrevCorr[i];

    /*  Comput the unit's activation value  */
    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      sum += cNet->values[j] * cWeights[j];
    if  ( recurrent && !reset )
      sum += cWeights[cNet->Nunits] * cTData->candPrevValues[i];
#ifdef CONNX
    connx += cNet->Nunits+recurrent;
#endif
    value           = activation( cTData->candTypes[i], sum );
    actPrime        = activation_prime( cTData->candTypes[i], value, sum );
    cTData->candSumVals[i] += value;

    if ( !recurrent )
      actPrime        /= cError->sumSqError;

    /*  Compute correlations  */
    for  ( j = 0 ; j < Noutputs ; j++ )  {
      error          = cError->errors[j];
      direction      = ( cPCorr[j] < 0.0 ) ? -1.0 : 1.0;
      change         -= direction * 
	((recurrent) ? ((error-cError->sumErr[j])/cError->sumSqError) :
		       actPrime * (error - cError->sumErr[j]));
      cCorr[j]       += error * value;
    }

    /*  Compute slopes for recurrent networks  */
    if ( recurrent )  {
      for  ( j = 0 ; j < cNet->Nunits ; j++ )  {
	if  ( reset )  cTData->candDVdW[i][j] = 0.0;
	sum          =  actPrime * (cNet->values[j] + 
				    (cTData->candIn.weights[i][cNet->Nunits] * 
				     cTData->candDVdW[i][j]));
	cSlopes[j]   += change * sum;
	cTData->candDVdW[i][j] =  sum;
      }
      
      if  ( !reset )  {
	sum          =  actPrime * (cTData->candPrevValues[i] + 
			           (cTData->candIn.weights[i][cNet->Nunits] * 
				     cTData->candDVdW[i][cNet->Nunits]));
	cSlopes[cNet->Nunits]   += change * sum;
	cTData->candDVdW[i][cNet->Nunits] =  sum;
      }
      cTData->candPrevValues[i] = value;
    }  else  
      /*  Compute slopes for non-recurrent networks  */
      for  ( j = 0 ; j < cNet->Nunits ; j++ )
	cSlopes[j] += change * cNet->values[j];
  }
}


/*	CASCOR ADJUST CORRELATIONS -  Normalize each candidate's correlation
	score and then stuff that value into the previous correlation 
	structure.  Zero out the correlation score to prepare for the next
	round.  Note which unit has the best total score to date.
*/

void cascor_adjust_correlations  ( void )
{
  float aveValue,
        score,
        correlation,
        *cCorr,
        *cPCorr;
  int   i,j;

  cTData->candBest      = 0;
  cTData->candBestScore = 0.0;

  for  ( i = 0 ; i < Ncand ; i++ )  {
    aveValue = cTData->candSumVals[i] / cDSet->Npts;
    score    = 0.0;
    cCorr    = cTData->candCorr[i];
    cPCorr   = cTData->candPrevCorr[i];

    /*  Adjust correlations  */
    for  ( j = 0 ; j < Noutputs ; j++ )  {
      correlation = ( cCorr[j] - aveValue * cError->sumErr[j] ) /
	            cError->sumSqError;
      cPCorr[j]   = correlation;
      cCorr[j]    = 0.0;
      score       += fabs( correlation );
    }

    /*  Find the best unit of the bunch  */
    cTData->candSumVals[i]  = 0.0;
    cTData->candScores[i] = score;
    if  ( score > cTData->candBestScore )  {
      cTData->candBest      = i;
      cTData->candBestScore = score;
    }
  }
}
