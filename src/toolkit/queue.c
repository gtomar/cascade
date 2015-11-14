/*	Queue Library Functions

	v2.0
	Matt White (mwhite+@cmu.edu)
	10/2/94
	
	Version 2.0 of this library is a much more efficient version of the
	previous version 1.0.  This extra efficiency is gained by utilizing
	a linked list of arrays instead of the the linked list of elements
	that were previously used.  A selectable parameter, 'granularity',
	allows the user to select the size of these arrays (called cells).

	When a queue is allocated, a cell is allocated as well.  Then, when
	this cell is filled and another item is placed in the queue, a second
	cell is allocated to hold that item and those that may follow it.  This
	process is repeated as often as necessary, or until memory runs out.

	As items are removed from the queue, cells may become empty.  These
	empty cells are deallocated and returned to the heap.  The sole 
	exception to this is that there will always be at least one cell
	allocated to a queue at any one time.

	Note that all processing is lazy.  Memory allocated until the user
	attempts to place an item in a nonexistant cell.  Memory is not
	deallocated until the user tries to remove an item from an empty cell.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toolkit.h"
#include "queue.h"


/*	Local Function Prototypes	*/

queue_cell_t *queue_alloc_cell   ( int, int );
void         queue_free_cell    ( queue_cell_t ** );


/************************** Visible Functions ********************************/

/*	QUEUE ALLOC -  Allocate a queue of elements of size 'size' and with a
	granularity of 'granularity'.  Note that if 'granularity' is equal to
	or less than 0, it will default to the value defined in the constant
	QUEUE_DEF_GRAN.
*/

queue_p queue_alloc ( int granularity, int size )
{
  queue_p      Q;
  char         *fn = "Initialize Queue";

  Q              = (queue_p)alloc_mem( 1, sizeof( queue_t ), fn );
  Q->Nelem       = 0;
  Q->granularity = (granularity <= 0) ? QUEUE_DEF_GRAN : granularity;
  Q->size        = size;

  Q->head = Q->tail = queue_alloc_cell( Q->granularity, Q->size );

  return Q;
}


/*	QUEUE SET GRANULARITY -  Change the granularity on queue 'Q' to 'gran'.
*/

void queue_set_granularity ( queue_p Q, int gran )
{
  Q->granularity = (gran <= 0) ? QUEUE_DEF_GRAN : gran;
}


/*	QUEUE FLUSH -  Flush the contents of queue 'Q', bit do not deallocate
	it.
*/

void queue_flush ( queue_p Q )
{
  void *temp;

  temp = (void *)alloc_mem ( 1, Q->size, "Flush Queue" );
  while ( !QUEUE_ISEMPTY( *Q ) )
    dequeue( Q, temp );
  free( temp );
}


/*	QUEUE FREE -  Flush and deallocate the memory associated with 'Q'.
*/

void queue_free ( queue_p *Q )
{
  queue_flush( *Q );
  queue_free_cell( &((*Q)->head) );
  free( *Q );
  *Q = NULL;
}


/*	ENQUEUE -  Add the data pointed to by 'newData' to 'Q'.  If the current
	cell is already full, allocate a new one and continue.  Note that new
	items are added to the 'last' element of the 'tail' section.

	If this new element fills the current cell, a special 'full' bit is set
	in addition to incrementing the last pointer.  Since the 'last' pointer
	will become equal to the 'first' pointer when the queue is full, we
	need this extra bit to distinguish from when the queue is empty.
*/

void enqueue ( queue_p Q, void *newData )
{
  if  ( Q->tail->full )  {
    Q->tail->next       = queue_alloc_cell( Q->granularity, Q->size );
    Q->tail->next->prev = Q->tail;
    Q->tail             = Q->tail->next;
  }
   
  memcpy( ((byte *)Q->tail->data)+((Q->tail->last)*(Q->size)), newData, 
	  Q->size );

  if  ((Q->tail->last == Q->tail->first-1) || 
       ((Q->tail->last == Q->tail->Nalloc-1) && (Q->tail->first == 0)) )
    Q->tail->full = TRUE;
  if (Q->tail->last == Q->tail->Nalloc-1)
    Q->tail->last = 0;
  else
    Q->tail->last++;
  Q->Nelem++;
}


/*	DEQUEUE -  Remove the first element from 'Q' and place it in 'data'.
	If the current cell was already empty, deallocate it (assuming that it
	was not the last cell).  Items are taken from the 'first' element of
	the 'head' cell.  Dequeue switches off the 'full' bit if it removes an
	item from a full cell.
*/

boolean dequeue ( queue_p Q, void *data )
{
  if  ( (Q->head->first == Q->head->last) && !Q->head->full )
    if  ( Q->head == Q->tail )  {
      data = NULL;
      return FALSE;
    } else {
      Q->head       = Q->head->next;
      queue_free_cell( &(Q->head->prev) );
      Q->head->prev = NULL;
    }

  memcpy( data, ((byte *)Q->head->data)+((Q->head->first)*(Q->size)), 
	  Q->size );
  if  ( Q->head->full )
    Q->head->full = FALSE;
  if (Q->head->first == Q->head->Nalloc-1)
    Q->head->first = 0;
  else
    Q->head->first++;
  Q->Nelem--;

  return TRUE;
}

void *queue_peek ( queue_p Q )
{
  queue_cell_t *head;

  if  ( (Q->head->first == Q->head->last) && !Q->head->full )
    if  ( Q->head == Q->tail )
      return NULL;
    else
      head       = Q->head->next;
  else
    head = Q->head;

  return &(((byte *)(head->data))[head->first*Q->size]);
}

boolean queue_append  ( queue_p dest, queue_p *source )
{
  if  ( dest->size != (*source)->size )
    return FALSE;

  dest->tail->next      =  (*source)->head;
  (*source)->head->prev =  dest->tail;
  dest->tail            =  (*source)->tail;
  dest->Nelem           += (*source)->Nelem;

  free( *source );
  *source = NULL; 
  return TRUE;
}

/************************* Internal Functions ********************************/

/*	QUEUE ALLOC CELL -  Allocate a new cell of 'size' and 'gran'ularity.
	Return a pointer to the allocated cell.
*/

queue_cell_t *queue_alloc_cell ( int gran, int size )
{
  queue_cell_t *newCell;
  char         *fn = "Queue Initialize Cell";
  
  newCell = (queue_cell_t *)alloc_mem( 1, sizeof( queue_cell_t ), fn );
  newCell->data = (void *)alloc_mem  ( gran, size, fn );

  newCell->first  = 0;
  newCell->last   = 0;
  newCell->Nalloc = gran;
  newCell->full   = FALSE;
  newCell->next   = NULL;
  newCell->prev   = NULL;

  return newCell;
}


/*	QUEUE FREE CELL -  Free the memory associated with the cell pointed to
	by '*cell'.  Set '*cell' to NULL and return.
*/

void queue_free_cell ( queue_cell_t **cell )
{
  free( (*cell)->data );
  free( *cell );
  *cell = NULL;
}
