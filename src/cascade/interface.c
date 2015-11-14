/*  CMU Cascade Neural Network Simulator (CNNS)
    User Interface Code

    v1.0
    Matt White (mwhite+@cmu.edu)
    May 26, 1995

    This code provides the top level interface with the user.  Also handled
    here are links to other user interface modules, such as the query module.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include "toolkit.h"
#include "cascade.h"

/*  Constants needed for the table lookup  */

#define NUM_PARMS 55
#define NOT_FOUND -1


/*  External structures we need access to  */

extern data_file_t   *cDFile;
extern net_t         *cNet,
                     *nets;
extern df_t          *dFiles;
extern train_parm_t  *cParms;
extern boolean       interruptPending,
                     interact;
extern jmp_buf       abort_trap;


/*  A table of parameters and functions for the interface  */

parm_t parmTable [NUM_PARMS] = {
  { "?",                  FUNC,    NULL, TRUE },
  { "algorithm",          ALGO,    NULL, FALSE },
  { "candChgThresh",      FLOAT,   NULL, TRUE },
  { "candEpochs",         INT,     NULL, TRUE },
  { "candInDecay",        FLOAT,   NULL, TRUE },
  { "candInEpsilon",      FLOAT,   NULL, TRUE },
  { "candInMu",           FLOAT,   NULL, TRUE },
  { "candOutDecay",       FLOAT,   NULL, TRUE },
  { "candOutEpsilon",     FLOAT,   NULL, TRUE },
  { "candOutMu",          FLOAT,   NULL, TRUE },
  { "candPatience",       INT,     NULL, TRUE },
  { "candType",           NODE,    NULL, FALSE },
  { "errorIndexThresh",   FLOAT,   NULL, TRUE },
  { "errorMeasure",       ERR,     NULL, TRUE },
  { "errorScoreThresh",   FLOAT,   NULL, TRUE },
  { "exit",               FUNC,    NULL, TRUE },
  { "inspectData",        FUNC,    NULL, TRUE },
  { "inspectNet",         FUNC,    NULL, TRUE },
  { "interact",           BOOLEAN, NULL, TRUE },
  { "killData",           FUNC,    NULL, FALSE },
  { "killNet",            FUNC,    NULL, FALSE },
  { "list",               FUNC,    NULL, TRUE },
  { "listData",           FUNC,    NULL, TRUE },
  { "listNets",           FUNC,    NULL, TRUE },
  { "loadData",           FUNC,    NULL, TRUE },
  { "loadNet",            FUNC,    NULL, FALSE },
  { "loadScript",         FUNC,    NULL, TRUE },
  { "maxNewUnits",        INT,     NULL, FALSE },
  { "NCands",             INT,     NULL, FALSE },
  { "outPrimeOffset",     FLOAT,   NULL, TRUE },
  { "outputChgThresh",    FLOAT,   NULL, TRUE },
  { "outputDecay",        FLOAT,   NULL, TRUE },
  { "outputEpochs",       INT,     NULL, TRUE },
  { "outputEpsilon",      FLOAT,   NULL, TRUE },
  { "outputMu",           FLOAT,   NULL, TRUE },
  { "outputPatience",     INT,     NULL, TRUE },
  { "overshootOK",        BOOLEAN, NULL, TRUE },
  { "predictNet",         FUNC,    NULL, TRUE },
  { "query",              FUNC,    NULL, FALSE },
  { "quit",               FUNC,    NULL, TRUE },
  { "recurrent",          BOOLEAN, NULL, FALSE },
  { "resizeNet",          FUNC,    NULL, FALSE },
  { "runTrials",          FUNC,    NULL, FALSE },
  { "saveNet",            FUNC,    NULL, TRUE },
  { "saveScript",         FUNC,    NULL, TRUE },
  { "sigMax",             FLOAT,   NULL, TRUE },
  { "sigMin",             FLOAT,   NULL, TRUE },
  { "syncNet",            FUNC,    NULL, FALSE },
  { "test",               BOOLEAN, NULL, TRUE },
  { "testNet",            FUNC,    NULL, FALSE },
  { "train",              FUNC,    NULL, FALSE },
  { "useCache",           BOOLEAN, NULL, FALSE },
  { "validate",           BOOLEAN, NULL, TRUE },
  { "validationPatience", INT,     NULL, TRUE },
  { "weightRange",        FLOAT,   NULL, TRUE }
};


/*  CLI -  Command Line Interface.  This function simply reads in command lines
    and then dispatches to another function that processes the commands.  This
    process continues until a false response is returned from the command 
    processor.
*/

void cli  ( boolean runStarted )
{
  char inBuffer [MAX_INPUT],
       *parm = NULL,
       *parmVal = NULL,
       *parmVal2 = NULL;

  printf ("Cascade v%s  Command Line Interface\n",VER);
  printf ("'?' for a list of parameters and commands.\n");

  do  {
    do
      printf ( "%s ", PROMPT );
    while( strlen( fgets( inBuffer, MAX_INPUT, stdin ) ) <= 1 );
    inBuffer[strlen( inBuffer )-1] = 0x00;

    parm     = strtok( inBuffer, " \t" );
    parmVal  = strtok( NULL,     " \t" );
    parmVal2 = strtok( NULL,     " \t" );

  }  while( process_command( runStarted, parm, parmVal, parmVal2 ) );
}


/*  PROCESS COMMAND -  This function dispatches the commands received at the
    cli.  There are two commands that need to be checked for specially:
    continue and abort.  These two commands are looked up explicitly, the
    others done via table lookup.
*/

boolean process_command  ( boolean runStarted, char *parm, char *parmVal, 
			   char *parmVal2 )
{
  int loc;

  /*  Check for continue and abort  */
  if  ( runStarted && !strcasecmp( parm, "continue" ) )  return FALSE;
  if  ( !strcasecmp( parm, "abort" ) )
    if  ( runStarted )  {
      printf ( "Aborting training of %s on %s at epoch %d.\n",
	       cNet->name, cDFile->filename, cNet->epochsTrained );
      longjmp( abort_trap, 1 );
    }  else  {
      printf ( "Run not started.\n" );
      return TRUE;
    }

  /*  Do table lookup  */
  if  ( (loc = find_key( parm )) == NOT_FOUND )  {
    printf  ("Key (%s) not found.\n", parm);
    return TRUE;
  }

  /*  If no value was specified for a parm, display information on it.  */
  if  ( interact && (parmVal == NULL) && (parmTable[loc].type != FUNC) )
    display_parm( parmTable[loc] );
  set_parm( runStarted, parmTable[loc], parmVal, parmVal2 );

  return TRUE;
}


/*  FIND KEY -  Do a lookup on the parameter table.  A binary search is used,
    so the keys must be in sorted order.
*/

int find_key  ( char *key )
{
  int location,
      start,
      end,
      dir;

  start    = 0;
  end      = NUM_PARMS;
  location = end/2;
  
  while  ( start <= end )  {
    if  ( (dir = strcasecmp( key, parmTable[location].name )) == 0 )
      return location;
    if  ( dir < 0 )
      end = location-1;
    else
      start = location+1;
    location = (start+end)/2;
  }

  return NOT_FOUND;
}


/*  DISPLAY PARM -  Display information on the specified parameter.
*/

void display_parm  ( parm_t parm )
{
  printf  ("Parameter:\t%s\n", parm.name);
  switch  ( parm.type )  {
    case INT:       printf ("Type:\t\tInteger\n");
                    printf ("Current value:\t%d",*(int *)parm.ptr);
                    break;
    case FLOAT:     printf ("Type:\t\tFloat\n");
                    printf ("Current value:\t%f",*(float *)parm.ptr);
                    break;
    case BOOLEAN:   printf ("Type:\t\tBoolean (true/false)\n");
                    printf ("Current value:\t%s",
			    btoa( 1, *(boolean *)parm.ptr ));
                    break;
    case NODE:      printf ("Type:\t\tNode ");
                    printf ("(Sigmoid, Asigmoid, Linear, Gaussian,");
                    printf (" Varsigmoid, Varied)\n");
                    printf ("Current value:\t%s",
			    ntoa( *(node_t *)parm.ptr ));
                    break;
    case ALGO:      printf ("Type:\t\tAlgorithm (CasCor, Cascade2)\n");
                    printf ("Current value:\t%s",
			    altoa( *(algo_t *)parm.ptr ));
                    break;
    case ERR:       printf ("Type:\t\tError Type (Bits, Index)\n");
                    printf ("Current value:\t%s",
			    etoa( *(error_t *)parm.ptr ));
                    break;
    case FUNC:      printf ("Type:\t\tSpecial Function");
                    break;
    }

  printf ("\n");
}


/*  SET PARM -  Set the specified parameter.  Checks are done to insure that
    we are eligible to modify this parameter at the present time.
*/

void set_parm  ( boolean runStarted, parm_t parm, char *parmVal, char *parmVal2 )
{
  char   val [41];

  if  ( runStarted && !parm.modWRun )  {
    printf ("Cannot modify parameter '%s' while a trial is in progress.\n",
	    parm.name);
    return;
  }

  if  ( parm.type != FUNC )
    if ( parmVal != NULL )  {
      strcpy (val, parmVal);
    } else if ( interact ) {
      printf ("New value: ");
      fgets  (val, 41, stdin);
      val[strlen( val )-1] = 0x00;
    } else {
      fprintf (stderr,"No value specified for %s. Parameter not set.\n",
	       parm.name);
      return;
    }

  switch  ( parm.type )  {
    case INT:      if ( isint( val ) ) *(int *)parm.ptr = atoi( val );
                   break;
    case FLOAT:    if ( isfloat( val ) ) *(float *)parm.ptr = atof( val );
                   break;
    case BOOLEAN:  if ( isboolean( val ) ) *(boolean *)parm.ptr = atob( val );
                   break;
    case NODE:     *(node_t *)parm.ptr = aton( val );
                   break;
    case ALGO:     *(algo_t *)parm.ptr = atoal( val );
                   break;
    case ERR:      *(error_t *)parm.ptr = atoe( val );
                   break;
    case FUNC:     ((void (*)(char *, char *))parm.ptr)(parmVal, parmVal2);
                   break;
    }
}


/*  SET PARMTABLE -  Sets the pointers in the parmTable to point to a specific
    set of parameters.
*/

void set_parmtable  ( train_parm_t *parms )
{
  int i = 0;

  parmTable[i++].ptr =  (void *)list_parms;
  parmTable[i++].ptr =  (void *)&(parms->algorithm);
  parmTable[i++].ptr =  (void *)&(parms->candidateParm.changeThreshold);
  parmTable[i++].ptr =  (void *)&(parms->candidateParm.epochs);
  parmTable[i++].ptr =  (void *)&(parms->candInUpdate.decay);
  parmTable[i++].ptr =  (void *)&(parms->candInUpdate.epsilon);
  parmTable[i++].ptr =  (void *)&(parms->candInUpdate.mu);
  parmTable[i++].ptr =  (void *)&(parms->candOutUpdate.decay);
  parmTable[i++].ptr =  (void *)&(parms->candOutUpdate.epsilon);
  parmTable[i++].ptr =  (void *)&(parms->candOutUpdate.mu);
  parmTable[i++].ptr =  (void *)&(parms->candidateParm.patience);
  parmTable[i++].ptr =  (void *)&(parms->candType);
  parmTable[i++].ptr =  (void *)&(parms->indexThreshold);
  parmTable[i++].ptr =  (void *)&(parms->errorMeasure);
  parmTable[i++].ptr =  (void *)&(parms->scoreThreshold);
  parmTable[i++].ptr =  (void *)quit;
  parmTable[i++].ptr =  (void *)inspect_data;
  parmTable[i++].ptr =  (void *)inspect_net;
  parmTable[i++].ptr =  (void *)&interact;
  parmTable[i++].ptr =  (void *)kill_data;
  parmTable[i++].ptr =  (void *)kill_net;
  parmTable[i++].ptr =  (void *)list_parms;
  parmTable[i++].ptr =  (void *)list_data;
  parmTable[i++].ptr =  (void *)list_nets;
  parmTable[i++].ptr =  (void *)load_data;
  parmTable[i++].ptr =  (void *)load_net;
  parmTable[i++].ptr =  (void *)load_script;
  parmTable[i++].ptr =  (void *)&(parms->maxNewUnits);
  parmTable[i++].ptr =  (void *)&(parms->Ncand);
  parmTable[i++].ptr =  (void *)&(parms->outPrimeOffset);
  parmTable[i++].ptr =  (void *)&(parms->outputParm.changeThreshold);
  parmTable[i++].ptr =  (void *)&(parms->outputUpdate.decay);
  parmTable[i++].ptr =  (void *)&(parms->outputParm.epochs);
  parmTable[i++].ptr =  (void *)&(parms->outputUpdate.epsilon);
  parmTable[i++].ptr =  (void *)&(parms->outputUpdate.mu);
  parmTable[i++].ptr =  (void *)&(parms->outputParm.patience);
  parmTable[i++].ptr =  (void *)&(parms->overshootOK);
  parmTable[i++].ptr =  (void *)predict;
  parmTable[i++].ptr =  (void *)query_net;
  parmTable[i++].ptr =  (void *)quit;
  parmTable[i++].ptr =  (void *)&(parms->recurrent);
  parmTable[i++].ptr =  (void *)resize_net;
  parmTable[i++].ptr =  (void *)run_trials;
  parmTable[i++].ptr =  (void *)save_net;
  parmTable[i++].ptr =  (void *)save_script;
  parmTable[i++].ptr =  (void *)&(parms->sigMax);
  parmTable[i++].ptr =  (void *)&(parms->sigMin);
  parmTable[i++].ptr =  (void *)sync_net;
  parmTable[i++].ptr =  (void *)&(parms->test);
  parmTable[i++].ptr =  (void *)test;
  parmTable[i++].ptr =  (void *)train;
  parmTable[i++].ptr =  (void *)&(parms->useCache);
  parmTable[i++].ptr =  (void *)&(parms->validate);
  parmTable[i++].ptr =  (void *)&(parms->validationPatience);
  parmTable[i++].ptr =  (void *)&(parms->weightRange);
}


/*  TRAIN -  Train a network.  If the network already exists in memory, use it.
    Otherwise, create a new network with the appropriate dimensions.  If the
    data file is in memory, it is used, otherwise it is loaded from disk and
    parsed.
*/

void train  ( char *netName, char *dFileName )
{
  char        prompt[61],
              nName[61],
              dFName[61];
  net_t       *net;
  data_file_t *dFile;

  /*  Get the network name  */
  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Network Name: ");
      scanf  ("%s",nName);
      netName = nName;
    } else {
      fprintf ( stderr, "No name specified for network to train.\n");
      fprintf ( stderr, "Training aborted.\n");
      return;
    }

  /*  Get the data file name  */
  if  ( dFileName == NULL )
    if  ( interact )  {
      printf ( "Data file name: " );
      scanf  ( "%s", dFName );
      dFileName = dFName;
    } else {
      fprintf ( stderr, "No data file specified for training.\n" );
      fprintf ( stderr, "Training aborted.\n" );
      return;
    }

  /*  Locate the desired data set  */
  if  ( (dFile = select_data ( dFileName )) == NULL )  {
    if  ( !parse_data ( dFileName, DEF_SIGMAX, DEF_SIGMIN, &dFile ) )  {
      fprintf ( stderr, "Unable to parse data file '%s'.\n", dFileName );
      return;
    }
    add_data_file( dFile );
  }

  /*  Check data file for training data  */
  if  (dFile->train == NULL)  {
    fprintf (stderr,
	     "No training data available in file '%s'.  Run not started.\n",
	     cDFile->filename);
    return;
  }

  /*  Locate or build the desired network  */
  if  ( (net = select_net( netName )) == NULL )  {
    net = build_net( netName, dFile->NinNodes, dFile->NoutNodes, 
		     cParms->maxNewUnits,  cParms->weightRange, 
		     cParms->sigMax, cParms->sigMin, cParms->recurrent );
    init_net( net, cParms->weightRange );
    if  ( !interact || prompt_yn( "Sync net outputs to data set", TRUE ) )
      sync( net, dFile );
    add_net( net );
  } else if  ( (net->Ninputs != dFile->NinNodes) &&
	       (net->Noutputs != dFile->NoutNodes ) )  {
    fprintf (stderr,"Number of inputs/outputs in net and data file must");
    fprintf (stderr," be the same.\nRun not started.\n");
    return;
  }

  /*  Train the network  */
  train_net ( net, cParms, dFile, 1 );
}


/*  TEST -  Run a test epoch on the indicated network and then report the
    results.  Same rules for locating networks and data apply as in training.
    The value of the global variables is NOT maintained, but this should not
    cause a problem since they are assumed unset for interface routines.
*/

void test  ( char *netName, char *dFileName )
{
  char           prompt[61],
                 nName[61],
                 dFName[61];
  net_t          *net;
  data_file_t    *dFile;
  trial_result_t results;
  error_data_t   *error;
  int            outVals;

  /*  Get the name of the network  */
  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Network Name: ");
      scanf  ("%s",nName);
      netName = nName;
    } else {
      fprintf ( stderr, "No name specified for network to test.\n");
      fprintf ( stderr, "Testing aborted.\n");
      return;
    }

  /*  Get the name of the data file  */
  if  ( dFileName == NULL )
    if  ( interact )  {
      printf ( "Data file name: " );
      scanf  ( "%s", dFName );
      dFileName = dFName;
    } else {
      fprintf ( stderr, "No data file specified for testing.\n" );
      fprintf ( stderr, "Testing aborted.\n" );
      return;
    }

  /*  Locate the data and check for testing data  */
  if  ( (dFile = select_data ( dFileName )) == NULL )  {
    if  ( !parse_data ( dFileName, DEF_SIGMAX, DEF_SIGMIN, &dFile ) )  {
      fprintf ( stderr, "Unable to parse data file '%s'.\n", dFileName );
      return;
    }
    add_data_file( dFile );
  }
  if  (dFile->test == NULL)  {
    fprintf (stderr,
	     "No testing data available in file '%s'.  Test not started.\n",
	     dFile->filename);
    return;
  }

  /*  Locate the network  */
  if  ( (net = select_net( netName )) == NULL )  {
    fprintf (stderr, "Network '%s' not found.  Testing aborted.\n", netName );
    return;
  } else if  ( (net->Ninputs != dFile->NinNodes) &&
	       (net->Noutputs != dFile->NoutNodes ) )  {
    fprintf (stderr,"Number of inputs/outputs in net and data file must");
    fprintf (stderr," be the same.\nRun not started.\n");
    return;
  }

  /*  Set global variables and perform the test epoch  */
  printf ("Testing '%s' on test data in '%s'...", netName, dFileName);

  set_globals ( net, cParms, NULL, dFile, NULL );
  results = test_net( net, dFile->test );
  outVals = dFile->test->Npts * net->Noutputs;
  results.perCorrect = (((float)(outVals-results.bits))/outVals)*100.0;
  
  printf ("done!\n");
  display_test_results  ( results );
}


/*  PREDICT -  Perform a prediction epoch.  The prediction data is given as
    inputs only.  The network indicated by netName is used to do feedforward
    predictions on the data.  The tokenized results of this are displayed as
    the predictions are made.  Rules for network and data file selection are
    the same as for the train and test functions.
*/

void predict  ( char *netName, char *dFileName )
{
  char           prompt[61],
                 nName[61],
                 dFName[61],
                 **intok,
                 **outtok;
  data_file_t    *dFile;
  data_set_t     *dSet;
  int            outVals,
                 i,j;
  float          aveSig;

  /*  Get the name of the network to use  */
  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Network Name: ");
      scanf  ("%s",nName);
      netName = nName;
    } else {
      fprintf ( stderr, "No name specified for network to test.\n");
      fprintf ( stderr, "Predicting aborted.\n");
      return;
    }

  /*  Get the name of the data file to use  */
  if  ( dFileName == NULL )
    if  ( interact )  {
      printf ( "Data file name: " );
      scanf  ( "%s", dFName );
      dFileName = dFName;
    } else {
      fprintf ( stderr, "No data file specified for predicting.\n" );
      fprintf ( stderr, "Predicting aborted.\n" );
      return;
    }

  /*  Select the data file and check it for prediction data  */
  if  ( (dFile = select_data ( dFileName )) == NULL )  {
    if  ( !parse_data ( dFileName, DEF_SIGMAX, DEF_SIGMIN, &dFile ) )  {
      fprintf ( stderr, "Unable to parse data file '%s'.\n", dFileName );
      return;
    }
    add_data_file( dFile );
  }
  if  (dFile->predict == NULL)  {
    fprintf (stderr,
	     "No prediction data available in file '%s'.\n", dFile->filename);
    fprintf (stderr, "Prediction aborted\n");
    return;
  }

  /*  Select the network and check for appropriate inputs/outputs  */  
  if  ( (cNet = select_net( netName )) == NULL )  {
    fprintf (stderr, "Network '%s' not found.  Prediction aborted.\n", 
	     netName );
    return;
  } else if  ( (cNet->Ninputs != dFile->NinNodes) &&
	       (cNet->Noutputs != dFile->NoutNodes ) )  {
    fprintf (stderr,"Number of inputs/outputs in net and data file must");
    fprintf (stderr," be the same.\nRun not started.\n");
    return;
  }

  printf ("Testing '%s' on prediction data in '%s'.\n", netName, dFileName);

  /*  Set up for the epoch  */
  set_globals ( cNet, cParms, NULL, dFile, NULL );
  cNet->values = cNet->tempValues;
  dSet         = dFile->predict;
  aveSig       = (cNet->sigmoidMax-cNet->sigmoidMin)/2.0;

  for  ( i = 0 ; i < dSet->Npts ; i++ )  {
    /*  Predict on a data point  */
    forward_pass ( dSet->data[i].inputs, dSet->data[i].reset );

    /*  Compute input/output token strings  */
    intok  = ftot ( dSet->data[i].inputs, aveSig, cNet->Ninputs, 
		    cNet->inputMap );
    outtok = ftot ( cNet->outValues, aveSig, cNet->Noutputs, cNet->outputMap );

    /*  Display the tokens  */
    for  ( j = 0 ; j < cNet->Ninputs ; j++ )  {
      printf ("%s%s",intok[j],(j==cNet->Ninputs-1)?" => ":", ");
      free( intok[j] );
    }
    free( intok );
    for  ( j = 0 ; j < cNet->Noutputs ; j++ )  {
      printf ("%s%s",outtok[j],(j==cNet->Noutputs-1)?"\n":", ");
      free( outtok[j] );
    }
    free( outtok );
  }
}


/*  QUIT -  Quit the program.  Use a yes/no confirmation.  */

void quit ( char *confirm, char *d2 )
{
  if  ( !interact || ((confirm != NULL) && atob( confirm )) )
    exit( 0 );
  if  ( prompt_yesno( "Really quit" ) )
    exit( 0 );
  else
    return;
}


/*  LIST PARMS -  List the changeable parameters in the program as well as the
    callable functions.
*/

void list_parms  ( char *d1, char *d2 )
{
  int i,j,k;

  printf ("\nParameters\n");
  printf ("~~~~~~~~~~\n");
  for  ( i = 0 ; i < NUM_PARMS ; i++ )
    if  ( parmTable[i].type != FUNC )  {
      printf ("%c%s",(parmTable[i].modWRun)?'*':' ',parmTable[i].name);
      for  ( j = strlen( parmTable[i].name ) ; j < 39 ; j++ )
	printf (" ");
      switch( parmTable[i].type )  {
        case INT:     printf ("%d\n",*(int *)(parmTable[i].ptr));
	              break;
	case FLOAT:   printf ("%f\n",*(float *)(parmTable[i].ptr));
	              break;
	case BOOLEAN: printf ("%s\n",btoa( ON_OFF,
					   *(boolean *)(parmTable[i].ptr) ));
	              break;
	case NODE:    printf ("%s\n",ntoa( *(node_t *)(parmTable[i].ptr) ));
	              break;
	case ALGO:    printf ("%s\n",altoa( *(algo_t *)(parmTable[i].ptr) ));
	              break;
	case ERR:     printf ("%s\n",etoa( *(error_t *)(parmTable[i].ptr) ));
	              break;
	}
    }

  printf ("\nFunctions\n");
  printf ("~~~~~~~~~\n");
  for  ( i = 0, j = 0; i < NUM_PARMS ; i++ )
    if  ( parmTable[i].type == FUNC )  {
      printf ("%c%s",(parmTable[i].modWRun)?'*':' ',parmTable[i].name);
      if  ( (j++ % 6) == 5 ) 
	printf ("\n");
      else
	for  ( k = strlen( parmTable[i].name ) ; k < 12 ; k++ )
	  printf (" ");
    }
  if  ( (j % 6) != 0 )
    printf ("\n");
  printf ("\n*abort:     Abort current run.\n");
  printf ("*continue:  Continue with current run.\n");
  printf ("\n* - Can be modified while training is in progress.\n\n");
}


/*  LIST NETS -  List the neural networks that currently reside in memory.
*/

void list_nets ( char *d1, char *d2 )
{
  net_t *index;

  printf ("Loaded networks:\n");
  index = nets;
  while ( index != NULL )  {
    printf ("  %s\n",index->name);
    index = index->next;
  }
}


/*  LIST DATA -  List the data files that currently reside in memory.
*/

void list_data ( char *d1, char *d2 )
{
  df_t *index;

  printf ("Loaded data files:\n");
  index = dFiles;
  while ( index != NULL )  {
    printf ("  %s\n",index->data->filename);
    index = index->next;
  }
}


/*  RUN TRIALS -  Run a number of trials on a data file.  A dummy network is
    used for training and is discarded after the results of training have been
    accumulated.  The aggregate results of training are reported at the end of
    the last trial.
*/

void run_trials  ( char *numTrials, char *dataFile )
{
  int            Ntrials,
                 i;
  net_t          *tempNet;
  data_file_t    *dFile;
  trial_result_t trialResult,
                 runResult;
  char           dFileName [41];

  /*  Get the number of trials to run  */
  if  ( numTrials == NULL )
    if  ( interact )  {
      printf ("Number of trials to run: ");
      scanf  ("%d", &Ntrials );
    } else {
      fprintf ( stderr, "Number of trials not specified.  Run not started.\n");
      return;
    }
  else
    Ntrials = atoi( numTrials );
  if ( Ntrials < 1 )  {
    fprintf ( stderr, "Number of trials should be a positive integer.\n");
    fprintf ( stderr, "Run not started.\n");
    return;
  }

  /*  Get the data file  */
  if  ( dataFile == NULL )
    if  ( interact )  {
      printf ("Data file for training: ");
      scanf ("%s", dFileName);
      dataFile = dFileName;
    } else {
      fprintf ( stderr, "Data file for training not specified.\n");
      fprintf ( stderr, "Run not started.\n");
      return;
    }
  if  ( (dFile = select_data ( dataFile )) == NULL )  {
    if  ( !parse_data ( dataFile, DEF_SIGMAX, DEF_SIGMIN, &dFile ) )  {
      fprintf ( stderr, "Unable to parse data file '%s'.\n", dataFile );
      return;
    }
    add_data_file( dFile );
  }
  if  (dFile->train == NULL)  {
    fprintf (stderr,
	     "No training data available in file '%s'.  Run not started.\n",
	     cDFile->filename);
    return;
  }

  tempNet   = build_net("Trial Net", dFile->NinNodes, dFile->NoutNodes,
			cParms->maxNewUnits, cParms->weightRange, 
			cParms->sigMax, cParms->sigMin, cParms->recurrent);

  for  ( i = 0 ; i < Ntrials ; i++ )  {
    /*  Run a trial  */
    init_net( tempNet, cParms->weightRange );
    trialResult = train_net( tempNet, cParms, dFile, i+1 );

    /*  Add the results of this trial to the previous trials  */
    if  ( i == 0 )  {
      runResult        = trialResult;
      runResult.Nunits -= tempNet->Ninputs+1;
    } else {
      runResult.bits       += trialResult.bits;
      runResult.Nepochs    += trialResult.Nepochs;
#ifdef CONNX
      runResult.connx      += trialResult.connx;
#endif
      runResult.time       += trialResult.time;
      runResult.Nvictories += trialResult.Nvictories;
      runResult.Nunits     += trialResult.Nunits-cNet->Ninputs-1;
      runResult.perCorrect += trialResult.perCorrect;
      runResult.index      += trialResult.index;
      runResult.sumSqDiffs += trialResult.sumSqDiffs;
      runResult.sumSqError += trialResult.sumSqError;
    }
  }

  display_run_results  ( runResult, Ntrials, cParms->errorMeasure );
  free_net( &tempNet );
}


/*  LOAD SCRIPT -  A script in CNNS is simply a list of commands as they would
    be typed at the CLI.  The commands are processed in an identical manner to
    how they are processed at the CLI, so care must be taken with automated
    scripts so that they do not prompt the user for information.  One sure way
    of doing this is to set 'interact FALSE' at the beginning of the script.

    Lines beginning with a '#' are ignored.  There can be only one command
    per line.  Scripts are nestable.
*/

void load_script  ( char *script, char *d1 )
{
  char infile[41],
       input_line[81],
       *parm,
       *parmVal,
       *parmVal2;
  FILE *fptr;

  /*  Open script file  */
  if ( script == NULL )
    if  ( interact )  {
      printf ("Filename of script to load: ");
      scanf  ("%s",infile);
      script = infile;
    } else {
      fprintf (stderr,"No script file specified.  Script not loaded.\n");
      return;
    }
  if  ((fptr = fopen( script, "r" )) == NULL)  {
    fprintf (stderr,"ERROR: Unable to open data file %s\n.", script);
    return;
  }

  /*  Repeatedly execute the lines in the script file  */
  fgets ( input_line, 80, fptr );
  while ( !feof( fptr ) )  {
    if  ( input_line[0] != '#' )  {
      input_line[strlen(input_line)-1] = '\0';
      parm     = strtok( input_line, " \t" );
      parmVal  = strtok( NULL,   " \t" );
      parmVal2 = strtok( NULL,   " \t" );
      
      if  ( parm != NULL )
	process_command ( FALSE, parm, parmVal, parmVal2 );
    }
    fgets ( input_line, 80, fptr );
  }
  fclose( fptr );
}


/*  SAVE SCRIPT -  Saves set parameters as a script file.  Functions are
    ignored.  The user can then later load the script file to recover all the
    training parameters he has set.
*/

void save_script  ( char *script, char *d1 )
{
  char   infile [41];
  FILE   *fptr;
  int    i;
  time_t timer;

  /*  Open script file  */
  if  ( script == NULL )
    if  ( interact )  {
      printf ("Filename for saved script: ");    
      scanf  ("%s",infile);
      script = infile;
    } else {
      fprintf (stderr,"No filename specified for script.");
      fprintf (stderr,"Script not saved.\n");
      return;
    }
  if  ((fptr = fopen( script, "w" )) == NULL)  {
    fprintf (stderr,"ERROR: Unable to open data file %s\n.", script);
    return;
  }

  /*  Print out the parameters  */
  time( &timer );
  fprintf (fptr,"# CNNS Ver %s script file created on %s",VER,ctime( &timer ));

  for  ( i = 0 ; i < NUM_PARMS ; i++ )  {
    if  ( parmTable[i].type == FUNC )
      continue;
    fprintf (fptr,"%s\t",parmTable[i].name);
    switch ( parmTable[i].type )  {
      case INT:     fprintf (fptr, "%d\n",*(int *)(parmTable[i].ptr));
	            break;
      case FLOAT:   fprintf (fptr, "%f\n",*(float *)(parmTable[i].ptr));
	            break;
      case BOOLEAN: fprintf (fptr, "%s\n",
			     btoa( ON_OFF, *(boolean *)(parmTable[i].ptr)));
	            break;
      case NODE:    fprintf (fptr, "%s\n",
			     ntoa( *(node_t *)(parmTable[i].ptr) ));
	            break;
      case ALGO:    fprintf (fptr, "%s\n",
			     altoa( *(algo_t *)(parmTable[i].ptr) ));
	            break;
      case ERR:     fprintf (fptr, "%s\n",
			     etoa( *(error_t *)(parmTable[i].ptr) ));
	            break;
    }
  }

  fclose( fptr );
}


/*  SAVE NET -  Prints the vital information about a network to the file
    specified.  This file then contains all that is needed to reconstruct the
    network later.
*/

void save_net ( char *netName, char *filename )
{
  FILE   *netFile;
  net_t  *net;
  int    i,j,k;
  cvrt_t *map;
  char   name    [41];
  char   outfile [41];

  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Name of network to save: ");
      scanf  ("%s", name );
      netName = name;
    } else {
      fprintf ( stderr, "No name specified for network to save.\n");
      fprintf ( stderr, "Network not saved.\n");
      return;
    }

  if  ( filename == NULL )
    if  ( interact )  {
      printf ("File name to save '%s' to: ",name);
      scanf  ("%s",outfile);
      filename = outfile;
    } else {
      fprintf (stderr,"No filename specified for output file.");
      fprintf (stderr,"Network not saved.\n");
      return;
    }

  if  ( (net = select_net( netName )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find network %s.\n", netName );
    return;
  }

  if  ((netFile = fopen( filename, "w" )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to open network file %s.\n",filename );
    return;
  }

  fprintf (netFile,"Name: %s\n",net->name);
  fprintf (netFile,"epochsTrained: %d\tNunits: %d\tNinputs: %d\n",
	   net->epochsTrained, net->Nunits, net->Ninputs);
  fprintf (netFile,"Noutputs: %d\tNhiddenUnits: %d\n",
	   net->Noutputs, net->NhiddenUnits);
  fprintf (netFile,"sigmoidMax: %f\tsigmoidMin: %f\trecurrent: %s\n\n",
	   net->sigmoidMax, net->sigmoidMin, btoa( YES_NO,net->recurrent ));

  fprintf (netFile,"$outputTypes\n");
  i = 0;
  while  ( i < net->Noutputs )  {
    fprintf  ( netFile, "%s  ", ntoa( net->outputTypes[i] ) );
    i++;
    if  (  ( i % 6 ) == 0 )
      fprintf ( netFile, "\n");
  }
  if  ( ( i % 6 ) != 0 )
    fprintf  ( netFile,"\n");
  fprintf ( netFile, "\n" );

  fprintf (netFile,"$unitTypes\n");
  i = 0;
  while  ( i < (net->Nunits - net->Ninputs - 1) )  {
    fprintf ( netFile, "%s  ", ntoa( net->unitTypes [i+net->Ninputs+1] ) );
    i++;
    if  ( ( i % 6 ) == 0 )
      fprintf ( netFile, "\n" );
  }
  if  ( ( i % 6 ) != 0 )
    fprintf ( netFile, "\n");
  fprintf ( netFile, "\n" );

  for  ( i = 0 ; i < net->Ninputs ; i++ )  {
    map = &(net->inputMap[i]);
    fprintf ( netFile, "$inputMap(%d)\n",i+1 );
    fprintf ( netFile, "Nenums: %d\tNunits: %d\n", map->Nenums, map->Nunits );
    for  ( j = 0 ; j < map->Nenums ; j++ )  {
      fprintf ( netFile, "%s ",map->enums[j]);
      for  ( k = 0 ; k < map->Nunits ; k++ )
	fprintf ( netFile, "%f ", map->equivs[j][k]);
      fprintf ( netFile, "\n" );
    }
    fprintf ( netFile, "unknown: " );
    for  ( j = 0 ; j < map->Nunits ; j++ )
      fprintf ( netFile, "%f ", map->unknown[j] );
    fprintf ( netFile, "\n\n" );
  }

  for  ( i = 0 ; i < net->Noutputs ; i++ )  {
    map = &(net->outputMap[i]);
    fprintf ( netFile, "$outputMap(%d)\n",i+1 );
    fprintf ( netFile, "Nenums: %d\tNunits: %d\n", map->Nenums, map->Nunits );
    for  ( j = 0 ; j < map->Nenums ; j++ )  {
      fprintf ( netFile, "%s ",map->enums[j]);
      for  ( k = 0 ; k < map->Nunits ; k++ )
	fprintf ( netFile, "%f ", map->equivs[j][k]);
      fprintf ( netFile, "\n" );
    }
    fprintf ( netFile, "unknown: " );
    for  ( j = 0 ; j < map->Nunits ; j++ )
      fprintf ( netFile, "%f ", map->unknown[j] );
    fprintf (netFile, "\n\n" );
  }

  for  ( i = 0 ; i < net->Noutputs ; i++ )  {
    fprintf  ( netFile, "$outWeights(%d)\n",i+1);
    j = 0;
    while  ( j < net->Nunits )  {
      fprintf  ( netFile, "%f  ", net->outWeights[i][j] );
      j++;
      if  ( ( j % 6 ) == 0 )
        fprintf  ( netFile, "\n" );
    }
    if  ( ( j % 6 ) != 0 )
      fprintf  ( netFile, "\n" );
    fprintf ( netFile, "\n");
  }

  /*  Print the hidden unit weights  */

  for  ( i = net->Ninputs + 1; i < net->Nunits ; i++ )  {
    fprintf  ( netFile, "$hiddenWeights(%d)\n",i+1);
    j = 0;
    while  ( j < i )  {
      fprintf  ( netFile, "%f  ", net->weights[i][j] );
      j++;
      if  ( ( j % 6 ) == 0 )
        fprintf  ( netFile, "\n" );
    }
    if  ( ( j % 6 ) != 0 )
      fprintf  ( netFile, "\n" );
    fprintf ( netFile, "\n");
  }

  fclose( netFile );
}


/*  LOAD NET -  Load a network from disk.
*/

void load_net ( char *fname, char *d1 )
{
  FILE    *netFile;
  net_t   *net;
  char    filename[81],
          netName[81],
          lineIn[81],
          recurrent[21],
          *tok,
          *delim = " \t\n",
          *fn = "Load Network";
  int     eTrained, Nunits, Ninputs,
          Noutputs, NhiddenUnits,
          count = 0,index,i,j;
  float   sigMax, sigMin;

  if  ( fname == NULL )
    if  ( interact )  {
      printf ("Network file to load: ");
      scanf  ("%s",filename);
      fname = filename;
    } else {
      fprintf ( stderr, "No network file name specified.\n");
      fprintf ( stderr, "Network not loaded.\n");
      return;
    }

  if  ((netFile = fopen( fname, "r" )) == NULL)  {
    fprintf (stderr, "ERROR: Unable to open network data file %s.\n",fname);
    return;
  }

  while ( !feof( netFile ) )  {
    fgets ( lineIn, 80, netFile );
    if  ( lineIn[0] == '$' )  {
      index = 0;
      count++;
      continue;
    }
    if  ( lineIn[0] != '#' && lineIn[0] != '\n' )
      switch ( count )  {
        case 0:  sscanf (lineIn, "Name: %s", netName);
	         if ( select_net( netName ) != NULL )  {
		   fprintf (stderr,"ERROR: Network '%s' already in memory.\n",
			    netName);
		   return;
		 }
	         count++;
	         break;
	case 1:  sscanf (lineIn, "epochsTrained: %d Nunits: %d Ninputs: %d",
			 &eTrained, &Nunits, &Ninputs );
	         if ( (eTrained < 0) || (Nunits < 2) || (Ninputs < 1) )  {
		   fprintf ( stderr, "ERROR: Illegal value for " );
		   fprintf ( stderr, "epochsTrained, Nunits or Ninputs.\n");
		   fprintf ( stderr, "epochsTrained= %d\tNunits= %d\t",
			     eTrained, Nunits );
		   fprintf ( stderr, "Ninputs= %d\n", Ninputs);
		   fprintf ( stderr, "Network not loaded.\n");
		   return;
		 }
	         count++;
	         break;
	case 2:  sscanf (lineIn, "Noutputs: %d NhiddenUnits: %d",&Noutputs,
			 &NhiddenUnits);
	         if  ( (Noutputs < 1) || (NhiddenUnits < 0) )  {
		   fprintf ( stderr, "ERROR: Illegal value for " );
		   fprintf ( stderr, "Noutputs or NhiddenUnits.\n" );
		   fprintf ( stderr, "Noutputs= %d\tNhiddenUnits= %d\n",
			     Noutputs, NhiddenUnits );
		   fprintf ( stderr, "Network not loaded.\n");
		   return;
		 }
	         count++;
	         break;
	case 3:  sscanf (lineIn,"sigmoidMax: %f sigmoidMin: %f recurrent: %s",
			 &sigMax, &sigMin, recurrent);
	         if ( sigMax < sigMin )  {
		   fprintf ( stderr, "ERROR: sigmoidMax may not be less than");
		   fprintf ( stderr, " sigmoidMin\n" );
		   fprintf ( stderr, "sigmoidMax= %f\tsigmoidMin= %f\n",
			     sigMax, sigMin );
		   fprintf ( stderr, "Network not loaded.\n");
		   return;
		 }
	         net = build_net ( netName, Ninputs, Noutputs, NhiddenUnits,
				    1.0, sigMax, sigMin, atob( recurrent ) );
	         net->NhiddenUnits = NhiddenUnits;
	         net->Nunits       = Nunits;
	         net->maxNewUnits  = 0;
	         net->epochsTrained = eTrained;
	         net->inputMap = (cvrt_t *)alloc_mem( Ninputs, 
						       sizeof( cvrt_t ), fn );
	         net->outputMap = (cvrt_t *)alloc_mem( Noutputs,
						        sizeof( cvrt_t ), fn );
	         add_net( net );
	         break;
	case 4:  tok = strtok (lineIn, delim);
	         while ( tok != NULL && tok[0] != '\n' && index < Noutputs )  {
		   net->outputTypes[index++] = aton( tok );
		   if ( net->outputTypes[index-1] == UNDEFINED )  {
		     fprintf ( stderr, 
			      "ERROR: Output type for %d is undefined.\n",
			      index);
		     fprintf ( stderr, "Network not loaded.\n");
		     del_net ( netName );
		     return;
		   }
		   tok = strtok( NULL, delim );
		 }
	         break;
	case 5:  tok = strtok (lineIn, delim);
	         while ( tok != NULL && tok[0] != '\n' 
			 && index < NhiddenUnits )  {
		   net->unitTypes[Ninputs+1+index++] = aton( tok );
		   if ( net->unitTypes[Ninputs+index] == UNDEFINED )  {
		     fprintf ( stderr,
			      "ERROR: Unit type for %d is undefined.\n",
			      Ninputs+index );
		     fprintf ( stderr, "Network not loaded.\n" );
		     del_net ( netName );
		     return;
		   }
		   tok = strtok( NULL, delim );
		 }
	         break;
	default: if ( count-6 < Ninputs )
	           net->inputMap[count-6] = read_map( netFile, lineIn );
	         else if ( count-6-Ninputs < Noutputs )
		   net->outputMap[count-6-Ninputs] = read_map(netFile,lineIn);
	         else if ( count-6-Ninputs-Noutputs < Noutputs )  {
		   tok = strtok( lineIn, delim );
		   while ( tok != NULL && tok[0] != '\n' && index < Nunits )  {
		     if  ( !isfloat( tok ) )  {
		       fprintf ( stderr, "ERROR: Invalid weight value.\n" );
		       fprintf ( stderr, "Network not loaded.\n" );
		       del_net ( netName );
		       return;
		     }
		     net->outWeights[count-6-Ninputs-Noutputs][index++] =
		       atof( tok );
		     tok = strtok( NULL, delim );
		   }
		 } else {
		   tok = strtok( lineIn, delim );
		   while ( tok != NULL && tok[0] != '\n' && 
			  index < count-2*Noutputs-5 )  {
		     if  ( !isfloat( tok ) )  {
		       fprintf ( stderr, "ERROR: Invalid weight value.\n" );
		       fprintf ( stderr, "Network not loaded.\n" );
		       del_net ( netName );
		       return;
		     }
		     net->weights[count-5-2*Noutputs][index++] =
		       atof( tok );
		     tok = strtok( NULL, delim );
		   }
		 }
	}
  }

  printf ("Network '%s' loaded.\n", netName );
}


/*  READ MAP -  This function is used by load_net to read the conversion maps
    for the network in from disk.
*/

cvrt_t read_map ( FILE *fptr, char *firstline )
{
  cvrt_t temp;
  int    i, j;
  char   token[21],
         linein[161],
         *tok;

  sscanf ( firstline, "Nenums: %d Nunits: %d", &temp.Nenums, &temp.Nunits );
  if ( temp.Nenums > 0 )  {
    temp.enums = (char **)alloc_mem( temp.Nenums,sizeof( char * ),"Read Map" );
    temp.equivs = (float **)alloc_mem(temp.Nenums,sizeof(float *),"Read Map");
    for ( i = 0 ; i < temp.Nenums ; i++ )  {
      fgets ( linein, 160, fptr );
      tok = strtok ( linein, " \t\n");
      temp.enums[i] = strdup( tok );
      temp.equivs[i] = (float *)alloc_mem(temp.Nunits,sizeof(float *),
					  "Read Map");
      for ( j = 0 ; j < temp.Nunits ; j++ )  {
	tok = strtok ( NULL, " \t\n" );
	temp.equivs[i][j] = atof( tok );
      }
    }
  }
  temp.unknown = (float *)alloc_mem(temp.Nunits,sizeof( float ),"Read Map");
  fgets ( linein, 160, fptr );
  tok = strtok ( linein+8, " \t\n" );
  for ( i = 0 ; i < temp.Nunits ; i++ )  {
    temp.unknown[i] = atof( tok );
    tok = strtok ( NULL, " \t\n" );
  }

  return temp;
}


/*  RESIZE NET -  This is essentially a wrapper around the realloc_net
    function.  Select the network and then give it the additional units asked
    for.
*/

void resize_net ( char *netName, char *newUnits )
{
  int   nUnits;
  net_t *net;
  char  name [41];

  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Name of network to resize: ");
      scanf ("%s",name);
      netName = name;
    } else {
      fprintf ( stderr, "No network name specified.  Network not resized.\n" );
      return;
    }

  if  ( newUnits == NULL )
    if  ( interact )  {
      printf ( "Number of units to add: ");
      scanf ( "%d", &nUnits );
    } else {
      fprintf ( stderr, "Number of units to add not specified.\n" );
      fprintf ( stderr, "Network not resized.\n" );
      return;
    }
  else
    nUnits = atoi( newUnits );

  if  ( nUnits < 1 )  {
    fprintf ( stderr, "Number of units added must be positive.\n");
    return;
  }

  if  ( (net = select_net( netName )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find network %s.\n", netName );
    return;
  }

  realloc_net ( net, nUnits );
  printf ("%s resized to allow an additional %d units.\n",netName, nUnits);
}


/*  KILL NET -  Remove a network from memory.  The network is referred to by
    name.  It is looked up and then demolished.
*/

void kill_net  ( char *netName, char *d1 )
{
  char name[61];

  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Network to remove: ");
      scanf  ("%s", name);
      netName = name;
    } else {
      fprintf ( stderr, "Name of network to kill not specified.\n" );
      fprintf ( stderr, "No networks removed from memory.\n" );
      return;
    }

  if  ( !del_net( name ) )
    printf ("ERROR: Could not find network '%s'.\n",name);
  else
    printf ("Network '%s' removed from memory.\n",name);
}


/*  KILL DATA -  Similar to the kill_net function, except this function
    removes a data file from memory.
*/

void kill_data  ( char *filename, char *d1 )
{
  char fname[61];
  
  if  ( filename == NULL )
    if  ( interact )  {
      printf ("Data file to remove: ");
      scanf  ("%s",fname);
      filename = fname;
    }  else  {
      fprintf ( stderr, "Name of data file to kill not specified.\n" );
      fprintf ( stderr, "No data files removed from memory.\n" );
      return;
    }

  if  ( !del_data_file( filename ) )
    printf ("ERROR: Could not find data file '%s'.\n",filename);
  else
    printf ("Data file '%s' removed from memory.\n",filename);
}


/*  INSPECT NET -  Display the vital statistics about a network to the screen.
    This function is very similar to the save_net function with only small
    differences in formatting (of course, this function does not print to 
    disk).
*/

void inspect_net ( char *netName, char *d1 )
{
  FILE   *netFile;
  net_t  *net;
  int    i,j,k;
  char   name [41];

  if  ( netName == NULL ) 
    if  ( interact )  {
      printf ("Name of network to inspect: ");
      scanf  ("%s", name);
      netName = name;
    } else {
      fprintf ( stderr, "Network to inspect not specified.\n" );
      return;
    }

  if  ( (net = select_net( netName )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find network %s.\n", netName );
    return;
  }

  printf ( "Name: %s\n", net->name );
  printf ( "  epochsTrained: %d\trecurrent: %s\n", net->epochsTrained,
	   btoa( YES_NO, net->recurrent ) );
  printf ( "  Nunits: %d\tNinputs: %d\tNoutputs: %d\tNhiddenUnits: %d\n", 
	   net->Nunits, net->Ninputs, net->Noutputs, net->NhiddenUnits );
  printf ( "  sigmoidMax: %f\tsigmoidMin: %f\n\n", net->sigmoidMax, 
	   net->sigmoidMin );

  for  ( i = 0 ; i < net->Ninputs ; i++ )  {
    printf ( "Input Map (%d)\n",i+1 );
    print_cvrt( &(net->inputMap[i]) );
  }

  for  ( i = 0 ; i < net->Noutputs ; i++ )  {
    printf ( "Output Map (%d)\n",i+1 );
    print_cvrt( &(net->outputMap[i]) );
  }

  for  ( i = 0 ; i < net->Noutputs ; i++ )  {
    printf  ( "Output %d\n",i+1);
    printf  ( "  Type: %s\n  ", ntoa( net->outputTypes[i] ) );
    j = 0;
    while  ( j < net->Nunits )  {
      printf  ( "%f  ", net->outWeights[i][j] );
      j++;
      if  ( ( j % 6 ) == 0 )
        printf  ( "\n  " );
    }
    if  ( ( j % 6 ) != 0 )
      printf  ( "\n" );
    printf ( "\n");
  }

  /*  Print the hidden unit weights  */

  for  ( i = net->Ninputs + 1; i < net->Nunits ; i++ )  {
    printf  ( "Unit %d\n", i+1 );
    printf  ( "  Type: %s\n  ", ntoa( net->unitTypes[i] ) );
    j = 0;
    while  ( j < i )  {
      printf  ( "%f  ", net->weights[i][j] );
      j++;
      if  ( ( j % 6 ) == 0 )
        printf  ( "\n  " );
    }
    if  ( ( j % 6 ) != 0 )
      printf  ( "\n" );
    printf ( "\n");
  }
}


/*  INSPECT DATA -  Display the vital stats on a data file that has been loaded
    into memory.
*/

void inspect_data ( char *dataFile, char *d1 )
{
  char        dFile[61];
  data_file_t *data;
  int         i;

  if  ( dataFile == NULL )
    if  ( interact )  {
      printf ("Data file to inspect: ");
      scanf  ("%s", dFile);
      dataFile = dFile;
    } else {
      fprintf ( stderr, "Data file to inspect not specified.\n" );
      return;
    }

  if  ( (data = select_data( dataFile )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find data file %s.\n", dataFile );
    return;
  }

  printf ( "File: %s\n", data->filename );
  printf ( "  Inputs:  %d\tNInput Nodes:  %d\n", data->Ninputs, 
	   data->NinNodes );
  printf ( "  Outputs: %d\tNOutput Nodes: %d\n", data->Noutputs,
	   data->NoutNodes );
  printf ( "  Binary Positive:  %f\tBinary Negative: %f\n\n", data->binPos,
	   data->binNeg );

  printf ( "Output Types\n");
  for  ( i = 0 ; i < data->Noutputs ; i++ )
    printf ("%8s%s",otoa( data->outputType[i] ), (i%8==7)?"\n  ":"  ");
  if  ( i%8 != 7 ) printf ("\n");
  printf ("\n");

  for  ( i = 0 ; i < data->Ninputs ; i++ )  {
    printf ( "Input Map %d\n", i+1 );
    print_cvrt( &(data->inputMap[i]) );
  }

  for  ( i = 0 ; i < data->Noutputs ; i++ )  {
    printf ( "Output Map %d\n", i+1 );
    print_cvrt( &(data->outputMap[i]) );
  }

  printf ("Data Sets\n");
  for  ( i = 0 ; i < data->NdataSets ; i++ )
    printf ( "  %s\tNpts: %d  standard devation: %f  predict only: %s\n",
	     data->dataSets[i].name, data->dataSets[i].Npts,
	     data->dataSets[i].stdDev, 
	     btoa( YES_NO, data->dataSets[i].predictOnly ) );
}


/*  PRINT CVRT -  Print a conversion map.  This function is used by both
    inspect_net and inspect_data.
*/

void print_cvrt ( cvrt_t *map )
{
  int j,k;

  printf ( "  Nenums: %d\tNunits: %d\n", map->Nenums, map->Nunits );
  for  ( j = 0 ; j < map->Nenums ; j++ )  {
    printf ( "  %s: ",map->enums[j]);
    for  ( k = 0 ; k < map->Nunits ; k++ )
      printf ( "%f%s", map->equivs[j][k], (k%6==5)?"\n\t":" " );
    if  ( k%6 != 5 ) printf ("\n");
  }
  printf ( "  unknown: " );
  for  ( j = 0 ; j < map->Nunits ; j++ )
    printf ( "%f%s", map->unknown[j], (j%6==5)?"\n\t":" " );
  if ( j%6 != 5 ) printf ("\n");
  printf ( "\n" );
}


/*  LOAD DATA -  Load a data file and add it to those stored in memory.
*/

void load_data ( char *filename, char *d1 )
{
  char        fname[61];
  data_file_t *dFile;


  if  ( filename == NULL )
    if  ( interact )  {
      printf ("Data file to load: ");
      scanf  ("%s",fname);
      filename = fname;
    } else {
      fprintf ( stderr, "Data file to load not specified.\n");
      return;
    }

  if  ( (dFile = select_data ( filename )) == NULL )  {
    if  ( !parse_data ( filename, DEF_SIGMAX, DEF_SIGMIN, &dFile ) )  {
      fprintf ( stderr, "Unable to parse data file '%s'.\n", filename );
      return;
    }
    add_data_file( dFile );
  }  else  {
    fprintf ( stderr, "Data file already in memory.\n" );
    fprintf ( stderr, "Unload the data file using killData and try again.\n");
    return;
  }
}


/*  SYNC NET -  This is a wrapper for the sync function.
*/

void sync_net ( char *netName, char *dFileName )
{
  char        nName[61],
              dFName[61];
  net_t       *net;
  data_file_t *dFile;

  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Name of network to sync: ");
      scanf  ("%s",nName);
      netName = nName;
    } else {
      fprintf ( stderr, "No network name specified.  Network not synced.\n");
      return;
    }

  if  ( (net = select_net( netName )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find network %s.\n", nName );
    return;
  }

  if  ( dFileName == NULL )
    if  ( interact )  {
      printf ("Name of data file to sync network to: ");
      scanf  ("%s", dFName);
      dFileName = dFName;
    } else {
      fprintf ( stderr, "No data file specified.  Network not synced.\n");
      return;
    }

  if  ( (dFile = select_data( dFName )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find data file %s.\n", dFName );
    return;
  }

  sync( net, dFile );
  printf ("%s synchronized with %s.\n", net->name, dFile->filename );
}


/*  TRAP CTRL C -  Trap the Control-C signal so that we can break out of
    training once it's started.
*/

void trap_ctrl_c  ( int sig )
{
  interruptPending = TRUE;
  signal( SIGINT, trap_ctrl_c );
}


/*  HANDLE INTERRUPT -  This function handles the actually break out of the
    training cycle.  It starts up the CLI and then, upon returning,
    recalculates dependant variables.
*/

void handle_interrupt  ( train_data_t *tData, int Npts )
{
  printf  ("\nSimulation suspended at epoch %d.\n",cNet->epochsTrained);
  interruptPending = FALSE;
  cli( TRUE );

  tData->outScaledEps         = cParms->outputUpdate.epsilon / Npts;
  tData->output.shrinkFactor  = cParms->outputUpdate.mu /
                                (cParms->outputUpdate.mu + 1.0);
  tData->candIn.shrinkFactor  = cParms->candInUpdate.mu / 
                                (cParms->candInUpdate.mu + 1.0);
  tData->candOut.shrinkFactor = cParms->candOutUpdate.mu /
                                (cParms->candOutUpdate.mu + 1.0);

  printf ("Simulation continuing...\n");
}
