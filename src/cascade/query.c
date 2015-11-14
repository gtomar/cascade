/*  CMU Cascade Neural Network Simulator (CNNS)
    Network Query Functions

    v1.0
    Matt White (mwhite+@cmu.edu)
    May 28, 1995
    
    The functions contained herein allow the user to query a network on
    individual problems.  The input is read from the command linein either
    raw or tokenized form.  A prediction is done using the network and the 
    predicted outputs are displayed in both raw and tokenized form.

    The function query_net is linked to the main CLI via the interface table.
    It is called like any other of the interface functions located on that
    table.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cascade.h"
#include "toolkit.h"


/*  External declarations needed for the feedforward predictions  */

extern net_t   *cNet;
extern int     Ninputs,
               Noutputs;
extern float   sigMax,
               sigMin;
extern boolean recurrent,
               interact;


/*  QUERY NET -  This is the main function of the query module.  Note that
    script execution is suspended while in this module and that no scripts
    can be executed while within this module.

    The first thing this function does is establish the network to query,
    locate that network and then set the necessary global variables to
    reflect that network.  After this initial setup is done, a simple command
    line interface is executed as a while loop.  The predictions are handled
    by subordinate functions.
*/

void query_net ( char *netName, char *d1 )
{
  char  inLine[81];

  /*  Determine network to query and locate that network in memory  */
  if  ( netName == NULL )
    if  ( interact )  {
      printf ("Network to query: ");
      gets(inLine);
      netName = strdup( inLine );
    } else {
      fprintf (stderr, 
	       "Network not specified.  Entrance to query mode aborted.\n");
      return;
    }
  if  ( (cNet = select_net( netName )) == NULL )  {
    fprintf ( stderr, "ERROR: Unable to find network %s.\n", netName );
    return;
  }

  /*  Set global variables to reflect this network  */
  Ninputs   = cNet->Ninputs;
  Noutputs  = cNet->Noutputs;
  sigMax    = cNet->sigmoidMax;
  sigMin    = cNet->sigmoidMin;
  recurrent = cNet->recurrent;

  /*  Execute the simple CLI  */
  printf ("Querying '%s'.  Type 'exit' to return to CLI.\n", netName );

  while ( TRUE )  {
    printf ("Query %s> ",netName);
    gets(inLine);
    
    if  ( !strncasecmp( inLine, "exit", 4 ) || 
	  !strncasecmp( inLine, "quit", 4 ) )
      return;
    else if  ( !strncasecmp( inLine, "rawinput", 8 ) )
      raw_input( inLine+8 );
    else if  ( !strncasecmp( inLine, "input", 5 ) )
      token_input( inLine+5 );
  }
}


/*  RAW INPUT -  This function handles network predictions in the case that the
    inputs are given as raw floating point numbers.  No translation is
    performed on the inputs.  The floating point numbers are read from the
    input string and then fed into the network.  The sole exception to this is
    the token 'reset' which causes recurrent networks to reset before
    performing the prediction.
*/

void raw_input  ( char *inp )
{
  float   *inputs;       /*  The floating point inputs to the network  */
  boolean reset = FALSE; /*  Network reset flag  */
  int     i,             /*  Indexing variable  */
          start = 0;     /*  Point on input vector to start loop  */
  char    *tok,          /*  Character token being evaluated  */
          **outtok;      /*  Tokenized outputs  */

  inputs = (float *)alloc_mem( Ninputs, sizeof( float ), "Raw Input" );

  /*  Read first token.  Allow the keyword 'reset' as well as floats  */
  if  ( (tok = strtok( inp, " \t," )) == NULL )  {
    fprintf (stderr,"Insufficient tokens for input.\n");
    return;
  }
  if  ( isfloat( tok ) )  {
    inputs[start++] = ( isint( tok ) ) ? atoi( tok ) : atof( tok );
  } else if ( !strncasecmp( tok, "reset", 5 ) )  {
    reset = TRUE;
  } else {
    fprintf (stderr,"Invalid token '%s', a numerical value is expected.\n",
	     tok);
    return;
  }

  /*  Read the rest of the tokens.  From this point floats only.  */
  for  ( i = start ; i < Ninputs ; i++ )  {
    if  ( (tok = strtok( NULL, " \t," )) == NULL )  {
      fprintf (stderr,"Insufficient tokens for input.\n");
      return;
    }
    if  ( !isfloat( tok ) )  {
      fprintf (stderr,"Invalid token '%s', a numerical value is expected.\n",
	       tok);
      return;
    }
    inputs[i] = ( isint( tok ) ) ? atoi( tok ) : atof( tok );
  }

  /*  Perform the forward pass  */
  cNet->values = cNet->tempValues;
  forward_pass ( inputs, reset );

  /*  Display the raw output  */
  printf ("Raw output: ");
  for  ( i = 0 ; i < Noutputs ; i++ )
    printf ("%5.3f ",cNet->outValues[i]);
  printf ("\n");

  /*  Display the tokenized output  */
  outtok = ftot ( cNet->outValues, (cNet->sigmoidMax-cNet->sigmoidMin)/2.0,
		 Noutputs, cNet->outputMap );
  printf ("Tokenized output: ");
  for  ( i = 0 ; i < Noutputs ; i++ )  {
    printf ("%s%s",outtok[i],(i==Noutputs-1)?"\n":", ");
    free( outtok[i] );
  }
  free( outtok );
  free( inputs );
}


/*  TOKEN INPUT -  This function is similar to the standard raw input, but
    accepts tokenized inputs.  All other aspects of function, including the
    reset keyword, are the same as for raw input
*/

void token_input ( char *inp )
{
  float   *inputs;
  boolean reset = FALSE;
  int     i,
          start = 0;
  char    *tok,
          **intok,
          **outtok;

  inputs = (float *)alloc_mem( Ninputs, sizeof( float ), "Tokenized Input" );
  intok  = (char **)alloc_mem( Ninputs, sizeof( char * ), "Tokenized Input" );

  /*  Tokenize the input string  */
  if  ( (tok = strtok( inp, " \t," )) == NULL )  {
    fprintf (stderr,"Insufficient tokens for input.\n");
    return;
  }
  if  ( !strncasecmp( tok, "reset", 5 ) )
    reset = TRUE;
  else
    intok[start++] = strdup( tok );

  for  ( i = start ; i < Ninputs ; i++ )  {
    if  ( (tok = strtok( NULL, " \t," )) == NULL )  {
      fprintf (stderr,"Insufficient tokens for input.\n");
      return;
    }
    intok[i] = strdup( tok );
  }

  /*  Convert tokens to floating point inputs  */
  if  ( ttof ( inputs, intok, Ninputs, cNet->inputMap ) )  {
    /*  Perform feedforward prediction  */
    cNet->values = cNet->tempValues;
    forward_pass ( inputs, reset );

    /*  Display outputs  */
    printf ("Raw output: ");
    for  ( i = 0 ; i < Noutputs ; i++ )
      printf ("%5.3f ",cNet->outValues[i]);
    printf ("\n");

    outtok = ftot ( cNet->outValues, (cNet->sigmoidMax-cNet->sigmoidMin)/2.0,
		    Noutputs, cNet->outputMap );
    printf ("Tokenized output: ");
    for  ( i = 0 ; i < Noutputs ; i++ )  {
      printf ("%s%s",outtok[i],(i==Noutputs-1)?"\n":", ");
      free( outtok[i] );
    }
    free( outtok );
  }

  for  ( i = 0 ; i < Ninputs ; i++ )
    free( intok[i] );
  free( intok );
}
