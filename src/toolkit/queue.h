/*	Queue Library Functions

	v2.0
	Matt White (mwhite+@cmu.edu)
	10/2/94

	See 'queue.c' for details'.
*/

#ifndef QUEUE
#define QUEUE

#include "toolkit.h"


/*	Macros	*/

#define QUEUE_NELEM(x)   ((x).Nelem)
#define QUEUE_ISEMPTY(x) (QUEUE_NELEM(x) == 0)


/*	Constants	*/

#define QUEUE_DEF_GRAN 100


/*	Data Structure Definitions	*/

typedef struct queue_cell_type {
  void                   *data;
  int                    first,
                         last,
                         Nalloc;
  boolean                full;
  struct queue_cell_type *next,
                         *prev;
} queue_cell_t;

typedef struct {
  int          Nelem,
               granularity,
	       size;
  queue_cell_t *head,
               *tail;
} queue_t, *queue_p;


/*	Visible Function Prototypes	*/

queue_p      queue_alloc               ( int, int );
void         queue_set_granularity     ( queue_p, int );
void         queue_flush               ( queue_p );
void         queue_free                ( queue_p * );
void         enqueue                   ( queue_p, void * );
boolean      dequeue                   ( queue_p, void * );
void         *queue_peek               ( queue_p );
boolean      queue_append              ( queue_p, queue_p * );

#endif
