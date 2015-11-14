#ifndef _DFA
#define _DFA

#include "toolkit.h"

#define NOT_FOUND -1

#define ISFINAL(x) ((x)->finalState)
#define ISDEAD(x)  ((x)->deadState)

typedef struct {
  char                  *sigma;
  struct dfa_state_type *state;
} transition_t, *transition_p;

typedef struct dfa_state_type  {
  void         *data;
  boolean      finalState,
               deadState;
  int          Ntrans;
  transition_p stateTrans;
} dfa_state_t, *dfa_state_p;

#endif
