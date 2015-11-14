/*	Toolkit Header File

	v1.0
	Matt White (mwhite+@cmu.edu)
	10/2/94

	This file contains the declaration of some useful data types and
	constants.  It also contains function prototypes for the smaller
	modules in the toolkit.
*/

#ifndef TOOLKIT
#define TOOLKIT


/*	Constant Declarations	*/

#define TRUE  1
#define FALSE 0
#define YES   1
#define NO    0
#define ON    1
#define OFF   0

#define TRUE_FALSE 1
#define YES_NO     2
#define ON_OFF     3

#define ERROR -1
#define NOT_FOUND -1

/*      Useful Macros   */

#define LIMIT(x,l) ((x < l) ? x : l)

/*	Data Type Declarations	*/

typedef unsigned char boolean;
typedef unsigned char byte;


/*	Visible Function Prototypes	*/

/*  memory.c  */
void     *alloc_mem    ( int, int, char * );
void     *realloc_mem  ( void *, int, int, char * ); 
void     *free_mem     ( void * );

/*  prompt.c  */
boolean  prompt_yn     ( char *, boolean );
boolean  prompt_yesno  ( char * );

/*  string.c  */

void     str_addch     ( char *, char );
char     *str_or       ( char *, char * );
char     *str_order    ( char * );

/*  str_cvrt.c  */

boolean  isint         ( char * );
boolean  isfloat       ( char * );
boolean  isboolean     ( char * );

char     *btoa         ( int, boolean );
boolean  atob          ( char * );


/*  The following are included to help fix brokeness in various vendor
    unices.
    */

#ifdef HPUX
#define srandom(x)     srand(x)
#define random()      rand()
#endif

#endif

