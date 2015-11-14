/*       CMU Cascade Neural Network Simulator (CNNS)

	 v1.0
	 Matt White  (mwhite+@cmu.edu)
	 May 25, 1995

	 This file contains the core of the Cascade Neural Network Simulator.
	 Program setup takes place here and then control is dispatched to the
	 Command Line Interface in file 'interface.c'.  Also contained within
	 this file are training routines common to both Cascade Correlation and
	 Cascade-2.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>

#include "toolkit.h"
#include "parse.h"
#include "cascade.h"


/*  Global Data  */

net_t        *cNet,         /*  Current network being trained  */
             *nets;
df_t         *dFiles;
train_parm_t *cParms;       /*  Current network training parameters  */
train_data_t *cTData;       /*  Training data on current network  */
data_file_t  *cDFile;       /*  Current data file being used  */
data_set_t   *cDSet;        /*  Current data set being used for training  */
error_data_t *cError;       /*  Error structure for the network  */

int          Ninputs,       /*  Number of inputs in the network  */
             Noutputs,      /*  Number of outputs  */
             Ncand,         /*  Number of candidate units  */
             NtrainOutVals; /*  Number of outputs * Number of training pts  */
#ifdef CONNX
int          connx;         /*  The number of connection crossings made  */
#endif
float        sigMax,        /*  Maximum value for a VARSIGMOID unit  */
             sigMin;        /*  Minimum value for a VARSIGMOID unit  */
boolean      recurrent,     /*  Whether the current network is recurrent  */
             interruptPending,  /*  Is the user waiting for attention  */
             interact;      /*  If TRUE, interact with user, else don't  */
jmp_buf      abort_trap;    /*  This jump point lets us kill a run in  */
                            /* progress  */ 


void main ( int argc, char *argv[] )
{
  train_parm_t *parms;
  time_t       t;
  int          i;

  display_banner( );             /*  Welcome user and then do some  */
                                 /* initializations.                */
  interruptPending = FALSE;
  interact         = TRUE;
  signal( SIGINT, trap_ctrl_c ); /*  Trap C-c so we can break out of a run  */
  time( &t );
  srandom( t );

  parms = build_parm( );         /*  Build and initialize a parm table  */
  set_parmtable( parms );
  cParms = parms;


  /*  Process command line arguments  */

  for  ( i = 1 ; i < argc ; i++ )  
    load_script( argv[i], NULL );

  /*  Invoke command interpreter  */

  cli( FALSE );
}


/*	TRAIN NET -  Train the network passed (net) on the data file 
	specified (dFile).  Use the parameters specified in the parm table, 
	'parms'.  The parameter, 'trialNum', is used to report the trial
	number to the user.

	Make sure that the network is built and that the data is loaded before
	calling this function or bad things may happen.
*/

trial_result_t  train_net  ( net_t *net, train_parm_t *parms, 
			     data_file_t *dFile, int trialNum )
{
  train_data_t   *tData;	/*  Training data (slope, deltas, etc. )  */
  error_data_t   *error;	/*  Error information on net's performance  */
  trial_result_t result,	/*  The results from this trial  */
                 testRes;	/*  Results from the net's test epoch  */
  status_t       status = TRAINING,	/*  Training status  */
                 valStatus = TRAINING;	/*  Cross-Validation status  */
  int            startEpochs,	/*  The age of the network at start  */
                 outVals,	/*  The number of output values in either  */
	                        /* the training or the test set  */
                 valCLeft,	/*  Validation cycles remaining until  */
                                /* stagnation  */
                 valBUnits,	/*  Number of units at peak performance  */
                 i,j;	        /*  Loop indices  */
  time_t         startTime,	/*  Time training began  */
                 endTime;	/*  Time training ended  */
  float          valBScore,	/*  Score at peak validation performance  */
                 **valBWeights; /*  Output weights at peak performance  */
  boolean        init = TRUE;   /*  Initialize the validation function?  */

  /*  Initialize for training  */

  tData = build_train_data ( net, parms, dFile->train->Npts );
  error = build_error_data ( net );
  set_globals ( net, parms, tData, dFile, error );
  startEpochs = net->epochsTrained;
#ifdef CONNX
  connx       = 0;
#endif
  time( &startTime );

  display_begin_trial  ( trialNum, startTime );

  if  ( parms->useCache )
    compute_cache( Ninputs, dFile->train, tData->valCache );


  /*  Setjmp is to mark our position in case the user aborts the run  */
  if  ( setjmp( abort_trap ) == 0 )  {
    while ( net->maxNewUnits > 0 )  {

      /*  Train outputs  */
      status = train_outputs( );
      display_trainout_results  ( net, error, status, parms->errorMeasure );

      /*  Validate and check status  */
      if  ( status == WIN )
	break;
      if  ( parms->validate )  {
	valStatus = validation_epoch( &valBScore, &valBWeights, &valCLeft,
				     &valBUnits, init );
	init = FALSE;
	if  ( valStatus != TRAINING )
	  break;
      }
      

      /*  Initialize the candidates and train them either with cascor or  */
      /* cascade-2, as specified by the user                              */
      init_cand( tData, Ncand, Noutputs, net->Nunits, recurrent,
		parms->weightRange, parms->candType );
      if  (cParms->algorithm == CASCOR)
	status = cascor_train_cand( );
      else
	status = c2_train_cand( );
      install_cand( tData->candBest, (cParms->algorithm == CASCADE2) );
      
      display_traincand_results  ( net, tData, status );
    }

    /*  If we ran out of new units, train the outputs from the last unit  */
    /* added.  Otherwise these output weights will perform badly  */
    if ( (status != WIN) && (valStatus == TRAINING) )
      status = train_outputs( );
  }
  time( &endTime );

  /*  Compute performance statistics and report  */
  result.Nepochs    = net->epochsTrained - startEpochs;
  result.time       = (int)(endTime - startTime);
  result.Nunits     = net->Nunits;
#ifdef CONNX
  result.connx      = connx;
#endif
  if  ( cParms->test )  {
    if  ( dFile->test == NULL )
      dFile->test = dFile->train;
    testRes = test_net( net, dFile->test );
    result.bits       = testRes.bits;
    result.index      = testRes.index;
    result.sumSqDiffs = testRes.sumSqDiffs;
    result.sumSqError = testRes.sumSqError;
    outVals           = (dFile->test->Npts) * (net->Noutputs);
  } else {
    result.bits       = error->bits;
    result.index      = error->index;
    result.sumSqDiffs = error->sumSqDiffs;
    result.sumSqError = error->sumSqError;
    outVals           = (dFile->train->Npts) * (net->Noutputs);
  }
  result.perCorrect = (((float)(outVals-result.bits))/outVals)*100.0;
  if  (((parms->errorMeasure == BITS) && (result.bits == 0)) ||
       ((parms->errorMeasure == INDEX) && 
	(result.index < parms->indexThreshold)) )  {
    result.endStatus = WIN;
    result.Nvictories = 1;
  } else {
    result.endStatus = LOSS;
    result.Nvictories = 0;
  }
  display_trial_results  ( result, trialNum, parms->test, parms->errorMeasure,
			   net->Ninputs, endTime );

  /*  Free memory allocated for training  */
  if  ( parms->validate )  {
    for  ( i = 0 ; i < net->Noutputs ; i++ )
      free( valBWeights[i] );
    free( valBWeights );
  }
  free_train_data( &tData, net, parms );
  free_error_data( &error );

  return result;
}


/*	TEST NET -  Test the network on the data set provided.  The results of
	the test are returned in a trial result structure.  This function has
	some reliance on the cParms structure.
*/

trial_result_t  test_net    ( net_t *net, data_set_t *dSet )
{
  error_data_t   *err,		/*  Temporary error information  */
                 *temp;		/*  Pointer to old error information  */
  net_t          *tempNet;	/*  Pointer to old network  */
  trial_result_t result;	/*  Results of testing  */
  int            i;		/*  Indexing variable  */

  /*  Save pointers to old information  */
  err     = build_error_data( net );
  temp    = cError;
  cError  = err;
  tempNet = cNet;
  cNet    = net;

  /*  Compute an epoch on the test data  */
  init_error( cError, net->Noutputs );
  for  ( i = 0 ; i < dSet->Npts ; i++ )  {
    forward_pass( dSet->data[i].inputs, cDSet->data[i].reset );
    compute_error( dSet->data[i].outputs, TRUE, FALSE, TRUE, 0.4999 );
  }

  /*  Store results and return global pointers to their old values  */
  result.bits       = cError->bits;
  result.index      = ERROR_INDEX( cError->sumSqDiffs, dSet->stdDev, 
				   (dSet->Npts)*Noutputs );
  result.sumSqDiffs = cError->sumSqDiffs;
  result.sumSqError = cError->sumSqError;
  cError            = temp;
  free_error_data( &err );
  cNet              = tempNet;

  return result;
}


/*  SET GLOBALS -  Sets global variables to represent the network being
    trained.  Global variables are used for the inner training loops because
    parameter passing is too time consuming.
*/

void set_globals  ( net_t *net, train_parm_t *parms, train_data_t *tData,
		    data_file_t *dFile, error_data_t *error )
{
  cNet   = net;
  cParms = parms;
  cTData = tData;
  cDFile = dFile;
  cDSet  = dFile->train;
  cError = error;

  Ninputs       = net->Ninputs;
  Noutputs      = net->Noutputs;
  Ncand         = parms->Ncand;
  NtrainOutVals = net->Noutputs * dFile->train->Npts;
  recurrent     = net->recurrent;

  sigMax        = net->sigmoidMax;
  sigMin        = net->sigmoidMin;
}


/*  TRAIN OUTPUTS -  Train the network's output weights for a number of of
    epochs specified in the training parms or until the network is trained to
    satisfaction, or we meet the stagnation criteria set by changeThreshold
    and patience.
*/

status_t train_outputs  ( void )
{
  int   quitEpoch = 0,	/*  Epoch to quit training due to stagnation  */
        i;		/*  Indexing variable  */
  float lastError;	/*  This is the error number to beat  */

  for  ( i = 0 ; i < cParms->outputParm.epochs ; i++ )  {

    /*  Compute an epoch on the training data  */
    init_error( cError, Noutputs );
    output_epoch( );

    if  ( interruptPending ) handle_interrupt( cTData, cDSet->Npts );

    /*  Check for WIN  */
    if ( (cParms->errorMeasure == BITS) && (cError->bits == 0) )
      return WIN;
    else  if (cParms->errorMeasure == INDEX)  {
      cError->index = ERROR_INDEX( cError->sumSqDiffs, cDSet->stdDev, 
				   NtrainOutVals );
      if  ( cError->index <= cParms->indexThreshold )
	return WIN;
    }

    adjust_weights( );
    cNet->epochsTrained++;

    /*  Check for STAGNATION/Improvement  */
    if  ( i == 0 )
      lastError = cError->sumSqDiffs;
    else if  ( fabs( cError->sumSqDiffs - lastError ) >
	       ( lastError * cParms->outputParm.changeThreshold ) )  {
      lastError = cError->sumSqDiffs;
      quitEpoch = cNet->epochsTrained + cParms->outputParm.patience;
    } else if  ( cNet->epochsTrained == quitEpoch )
      return STAGNANT;
  }

  return TIMEOUT;
}


/*  OUTPUT_EPOCH  - Present each pattern to the network once and accumulate
    error from the outputs.
*/

void output_epoch  ( void )
{
  int i;

  for  ( i = 0 ; i < cDSet->Npts ; i++ )  {
    if  ( cParms->useCache )  {
      cNet->values   = cTData->valCache[i];
      cError->errors = cTData->errCache[i];
      compute_outputs( );
    }  else 
      forward_pass( cDSet->data[i].inputs, cDSet->data[i].reset );
    compute_error( cDSet->data[i].outputs, TRUE, TRUE, 
		   (cParms->algorithm == CASCOR), cParms->scoreThreshold );
  }
}


/*	VALIDATION EPOCH -  Present each pattern in the validation set to the
	network and compute the error.  If no validation data is present, the
	training data is used, which should produce no difference in results
	than was received during training.

	The function returns the best score to date in 'bestScore', the
	weights and units accounting for that score in 'bestWeights' and
	'bestUnits', respectively.  The time before stagnation is returned
	in 'cyclesLeft'.  The return value of the function is either TRAINING
	if training should continue or STAGNANT if 'valPatience' epochs have
	passed since 'bestScore' was set.
*/

status_t validation_epoch  ( float *bestScore, float ***bestWeights, 
	  		    int *cyclesLeft, int *bestUnits, boolean init )
{
  trial_result_t valRes;	/*  Result value to return  */
  int            maxUnits,	/*  Maximum number of units in the network  */
                 i,j;		/*  Indexing variables  */
  char           *fn = "Validation Epoch";

  /*  Select the validation data and run a test epoch on it  */
  if  ( cDFile->validate == NULL )  {
    printf ("No validation data.  Validation aborted.\n");
    cParms->validate = FALSE;
    return TRAINING;
  }
  valRes = test_net( cNet, cDFile->validate );

  /*  If this is the first validation epoch this run, init the data structs  */
  if  ( init )  {
    maxUnits = cNet->Nunits+cNet->maxNewUnits;
    *bestWeights = (float **)alloc_mem( cNet->Noutputs,sizeof( float * ),fn );
    for  ( i = 0 ; i < cNet->Noutputs ; i++ )
      (*bestWeights)[i] = (float *)alloc_mem( maxUnits, sizeof(float), fn );
    init = FALSE;
  }

  /*  Compare this result with the previous best and get the weights if this */
  /* epoch had better results  */
  if  ( (valRes.sumSqError < *bestScore) || init )  {
    *bestScore  = valRes.sumSqError;
    *cyclesLeft = cParms->validationPatience;
    *bestUnits  = cNet->Nunits;
    for  ( i = 0 ; i < cNet->Noutputs ; i++ )
      for  ( j = 0 ; j < cNet->Nunits ; j++ )
	(*bestWeights)[i][j] = cNet->outWeights[i][j];
    display_validate_results( valRes, cParms->errorMeasure, *bestScore,
			      *cyclesLeft );
    return TRAINING;
  } else if  ( *cyclesLeft > 0 )  {
    (*cyclesLeft)--;
    display_validate_results( valRes, cParms->errorMeasure, *bestScore,
			      *cyclesLeft );
    return TRAINING;
  }

  /*  Since we stagnated, restore network to its peak performance  */
  cNet->Nunits  = *bestUnits;
  for  ( i = 0 ; i < cNet->Noutputs ; i++ )
    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      cNet->outWeights[i][j] = (*bestWeights)[i][j];
  display_validate_results( valRes, cParms->errorMeasure, *bestScore,
			    *cyclesLeft );
  return STAGNANT;
}


/*  ADJUST WEIGHTS -  Adjust all the weights from the outputs to the units in
    the network according to a quickprop update, based upon the error data
    collected in the output epoch.
*/

void adjust_weights  ( void )
{
  float *ow,  /*  Output weights  */
        *od,  /*  Output deltas   */
        *os,  /*  Output slopes   */
        *op;  /*  Output previous slopes  */
  int   i,j;

  for  ( i = 0 ; i < Noutputs ; i++ )  {
    ow = cNet->outWeights[i];
    od = cTData->output.deltas[i];
    os = cTData->output.slopes[i];
    op = cTData->output.pSlopes[i];
    for  ( j = 0 ; j < cNet->Nunits ; j++ )
      quickprop( ow+j, od+j, os+j, op+j, cTData->outScaledEps, 
		 cParms->outputUpdate.decay, cParms->outputUpdate.mu,
		 cTData->output.shrinkFactor );
  }
}


/*  ADJUST CI WEIGHTS -  Adjust the weights to the inputs of the candidates.
    The epsilon value is scaled by the number of points in the data set and
    the number of units in the network.  Otherwise, this is the same as the
    adjust weights function above.
*/

void  adjust_ci_weights  ( void )
{
  float scaledEpsilon,
        *cw,
        *cd,
        *cs,
        *cp;
  int   i,j;

  scaledEpsilon = cParms->candInUpdate.epsilon / 
                  (float)(cDSet->Npts * cNet->Nunits);

  for  ( i = 0 ; i < Ncand ; i++ )  {
    cw = cTData->candIn.weights[i];
    cd = cTData->candIn.deltas[i];
    cs = cTData->candIn.slopes[i];
    cp = cTData->candIn.pSlopes[i];
    for  ( j = 0 ; j < cNet->Nunits + recurrent; j++ )
      quickprop( cw+j, cd+j, cs+j, cp+j, scaledEpsilon, 
		 cParms->candInUpdate.decay, cParms->candInUpdate.mu,
		 cTData->candIn.shrinkFactor );
  }
}


/*  ADJUST CO WEIGHTS -  Similar to adjust_ci_weights, except that it modifies
    the candidate's output weights.  This function is used by Cascade-2 only.
*/

void  adjust_co_weights  ( void )
{
  float scaledEpsilon,
        *cw,
        *cd,
        *cs,
        *cp;
  int   i,j;

  scaledEpsilon = cParms->candOutUpdate.epsilon  / 
                  (float)(cDSet->Npts * cNet->Nunits);

  for  ( i = 0 ; i < Ncand ; i++ )  {
    cw = cTData->candOut.weights[i];
    cd = cTData->candOut.deltas[i];
    cs = cTData->candOut.slopes[i];
    cp = cTData->candOut.pSlopes[i];
    for  ( j = 0 ; j < Noutputs ; j++ )
      quickprop( cw+j, cd+j, cs+j, cp+j, scaledEpsilon, 
		 cParms->candOutUpdate.decay, cParms->candOutUpdate.mu,
		 cTData->candOut.shrinkFactor );
  }
}


/*  INSTALL CAND -  Installs a new unit in the network.  Given a candidate
    number, the function copies that unit's weights over into the network.  If
    'useOutWeights' is set, the candidate's output weights are copied as well.
    Otherwise an approximation based on the unit's correlation value is used.
*/

void install_cand  ( int candNum, boolean useOutWeights )
{
  float *newWeights,
        *candWeights,
        weightModifier;
  int   i;

  /*  Copy the new unit's inputs to the network  */
  newWeights  = cNet->weights[cNet->Nunits];
  candWeights = cTData->candIn.weights[candNum];
  for  ( i = 0 ; i < (cNet->Nunits+recurrent) ; i++ )
    newWeights[i] = candWeights[i];

  /*  Either copy the output weights over or approximate them  */
  if  ( useOutWeights )
    for  ( i = 0 ; i < Noutputs ; i++ )
      cNet->outWeights[i][cNet->Nunits] = -cTData->candOut.weights[candNum][i];
  else  {
    if  ( cParms->errorMeasure == BITS )
      weightModifier = 1.0;
    else
      weightModifier = 1.0 / cNet->Nunits;
    for  ( i = 0 ; i < Noutputs ; i++ )
      cNet->outWeights[i][cNet->Nunits] = -cTData->candPrevCorr[candNum][i] *
	                                  weightModifier;
  }

  cNet->unitTypes[cNet->Nunits] = cTData->candTypes[candNum];

  /*  Compute the new cache values and then increment/decrement the  */
  /* appropriate counters  */
  if  ( cParms->useCache )
    recompute_cache( cNet->Nunits, cNet, cDSet, cTData->valCache );
  cNet->Nunits++;
  cNet->NhiddenUnits++;
  cNet->maxNewUnits--;
}
