#include "dfa.h"
#include "toolkit"

dfa_state_p dfa_create_state ( void *data, boolean final, boolean dead );
{
  retVal dfa;
  dfa = alloc_mem( 1, sizeof( dfa_state_t ), "DFA Initialize" );

  dfa->data       = data;
  dfa->finalState = final;
  dfa->deadState  = dead;
  dfa->Ntrans     = 0;
  dfa->stateTrans = NULL;
}

boolean  dfa_create_transition  ( dfa_state_p start, dfa_state_p end,
				  char *sigmaPrime, boolean overwrite )
{
  int   link = NOT_FOUND,
        i;
  char  *fn = "DFA Create State Transition";

  for ( i = 0 ; i < start->Ntrans ; i++ )  {
    if ( start->stateTrans[i].state = end )  {
      lint = i;
      continue;
    }
    if ( strpbrk( sigmas, start->stateTrans[i].sigma ) )
      return FALSE;
  }

  if ( link != NOT_FOUND )
    str_or( sigma, sigmaPrime );
  else {
    start->stateTrans = (transition_p)realloc_mem(start->stateTrans,
						  start->Ntrans,
						  sizeof( transition_t ), fn);
    start->stateTrans[start->Ntrans].sigma = strdup( sigmaPrime );
    start->stateTrans[start->Ntrans].state = end;
    start->Ntrans++;
  }

  return TRUE;
}
