/*	CMU Cascade Neural Network Simulator (CNNS)
	Console display routines

	v1.0
	Matt White  (mwhite+@cmu.edu)
	December 7, 1994

	Routines to display progress reports during training.
*/

#include <time.h>
#include <stdio.h>

#include "cascade.h"
#include "toolkit.h"

#ifdef CONNX
extern int connx;
#endif


/*  DISPLAY BANNER -  Prints out the program's welcome banner.
*/

void display_banner  ( void )
{
  printf  ("CMU Cascade Neural Network Simulator v%s\n", VER );
  printf  ("  Question/Comments: %s\n", CONTACT);
  printf  ("  Compiled %s at %s.\n", __DATE__, __TIME__);
#ifdef CONNX
  printf  ("  Connection crossing statistics ENABLED.\n\n");
#else
  printf  ("  Connection crossing statistics DISABLED.\n\n");
#endif
}


/*  DISPLAY BEGIN TRIAL -  Displays information about beginning a trial
*/

void display_begin_trial  ( int trialNum, time_t startTime )
{
  printf ("\n\nTrial %d begun at %s\n", trialNum, ctime( &startTime ));
}


/*  DISPLAY TRAINOUT RESULTS -  Display results from the output training
    phase.
*/

void display_trainout_results  ( net_t *net, error_data_t *err, 
				 status_t stat, error_t measure )
{
  int i, j;

  printf  ("\n  End Output Training Cycle (%s)\n", stoa( stat ) );
  printf  ("    Epoch: %d", net->epochsTrained);
#ifdef CONNX
  printf  ("\t\tConnection crossings: %d\n",connx );
#else
  printf  ("\n");
#endif

  if  ( measure == BITS )
    printf  ("    Error bits: %d  ", err->bits);
  else
    printf  ("    Error index: %.3f  ", err->index);
  printf  ("Sum squared diffs: %.3f  Sum squared error: %.3f\n",
	   err->sumSqDiffs, err->sumSqError);

  for  ( i = 0 ; i < net->Noutputs ; i++ )  {
    printf  ("    Output %2d:  ",i+1);
    j = 0;
    while  ( j < net->Nunits )  {
      printf  ("%8.3f  ", net->outWeights[i][j]);
      j++;
      if  ( j == net->Nunits )
	printf  ("\n");
      else if ( (j % 6) == 0 )
	printf  ("\n                ");
    }
  }
  printf ("\n");
}


/*  DISPLAY VALIDATE RESULTS -  Display the results from the validation epoch.
*/

void display_validate_results  ( trial_result_t res, error_t measure,
				 float bestErr, int passesLeft)
{
  printf  ("  Validation Epoch\n");
  if  ( measure == BITS )
    printf  ("    Error bits: %d\t", res.bits);
  else
    printf  ("    Error index: %.3f\t", res.index);
  printf  ("Sum sq diffs: %.3f\tSum sq error: %.3f\n",res.sumSqDiffs,
	   res.sumSqError);
  printf  ("    Best sum sq error: %.3f\tPasses until stagnation: %d\n\n",
	   bestErr, passesLeft);
}


/*  DISPLAY TRAINCAND RESULTS -  Display the results from candidate training.
*/

void display_traincand_results ( net_t *net, train_data_t *tData, 
				 status_t stat )
{
  int i = 0;

  printf  ("  End Candidate Training Cycle (%s)\n", stoa( stat ));
  printf  ("    Epoch: %d", net->epochsTrained);
#ifdef CONNX
  printf  ("\t\tConnection crossings: %d\n", connx );
#else
  printf  ("\n");
#endif

  printf  ("    Adding unit: %d\tUnit type: %s\tScore: %8.3f\n",
	   tData->candBest, ntoa( net->unitTypes[net->Nunits-1] ),
	   tData->candBestScore);

  printf  ("    Unit %2d:  ", net->Nunits);
  while  ( i < (net->Nunits-1+net->recurrent) )  {
    printf  ("%8.3f  ", net->weights [net->Nunits-1][i]);
    i++;
    if  ( i == (net->Nunits-1+net->recurrent) )
      printf ("\n");
    else if  ( ( i % 6 ) == 0 )
      printf ("\n              ");
  }
  printf ("\n");
}


/*  DISPLAY TRIAL RESULTS -  Display the results of training this network.
*/

void display_trial_results  ( trial_result_t res, int trialNum, boolean test, 
			      error_t measure, int Ninputs, time_t endTime )
{
  printf ("  End Trial Results\n");
  printf ("    Epochs: %d\tAverage epoch time: %.2f sec (%.2f epochs/sec)\n",
	  res.Nepochs, ((float)res.time)/res.Nepochs, 
	  ((float)res.Nepochs)/(res.time==0?1:res.time));
#ifdef CONNX
  printf ("    Connection crossings: %d\tCrossings per second: %.2f\n",
	  res.connx, ((float)res.connx)/(res.time==0?1:res.time));
#endif
  printf ("    Total units: %d\t\t\tHidden units: %d\n", res.Nunits,
	  res.Nunits - Ninputs - 1);
  
  if  ( test )
    printf ("    Test results:     ");
  else
    printf ("    Training results: ");
  printf ("Sum sq diffs: %.3f\tSum sq error: %.3f\n", res.sumSqDiffs,
	  res.sumSqError);
  if  ( measure == BITS )
    printf ("                      Error bits: %d\t\tPercent correct: %.2f\n",
	    res.bits, res.perCorrect);
  else
    printf ("                      Error index: %.2f\n", res.index);

  printf ("\nTrial %d ended at %s\n\n", trialNum, ctime( &endTime ) );
}


/*  DISPLAY RUN RESULTS -  Display the results from training this whole set of
    networks.
*/

void display_run_results  ( trial_result_t res, int Ntrials, error_t measure )
{
  printf  ("\n\n");
  printf  ("Run Results\n");
  printf  ("~~~~~~~~~~~\n");
  printf  ("  %d trials\t%d victories\t%d defeats\n", Ntrials, res.Nvictories,
	   Ntrials-res.Nvictories);
  printf  ("  Run time: %d hrs  %d min  %d sec",
	   res.time/3600, (res.time%3600)/60, res.time%60);
#ifdef CONNX
  printf  ("\t\t%.1f conn/sec\n",((float)res.connx)/((res.time==0?1:res.time)*Ntrials));
#else
  printf  ("\n");
#endif
  printf  ("  Ave epochs: %.1f\t\tAve hidden units: %.1f\n",
	   ((float)res.Nepochs)/Ntrials,((float)res.Nunits)/Ntrials);
  printf  ("  Ave sum sq diffs: %.3f\tAve sum sq error: %.3f\n",
	   res.sumSqDiffs/Ntrials, res.sumSqError/Ntrials);
  if  ( measure == BITS )
    printf ("  Ave bits wrong: %.1f\tAve percent correct: %.1f\n",
	    ((float)res.bits)/Ntrials, res.perCorrect/Ntrials);
  else
    printf ("  Ave error index: %.2f\n", res.index/Ntrials);
  printf ("\n\n");
}


/*  DISPLAY TEST RESULTS -  Display the results from the test epoch.
*/

void display_test_results  ( trial_result_t res )
{
  printf ("Test Results\n");
  printf ("  Sum sq diffs: %.3f\tSum sq error: %.3f\n", res.sumSqDiffs,
	  res.sumSqError);
  printf ("  Error bits: %d\t\tPercent correct: %.2f\n",
	    res.bits, res.perCorrect);
  printf ("  Error index: %.2f\n", res.index);
}
 
