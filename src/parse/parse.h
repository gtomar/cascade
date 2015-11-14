/*       CMU Learning Benchmark Archive
         Parse Library

         v2.0
         Matt White  (mwhite+@cmu.edu)
         3/5/95
*/

#ifndef PARSE
#define PARSE

#include "toolkit.h"

/*  Type Declarations  */

typedef enum {		/*  Outputs can be either BINARY or                  */
  CONT,			/*  CONTinuous.  BINARY values are used for binary   */
  BINARY		/*  and enumerated units while CONTinuous values are */
  }  out_t;		/*  used for standard floating point numbers.        */


/*	DATA SET -  This structure contains the information needed to train a
	network to perform a specific task.  The filename that the data came
	from, the output types, number of inputs, outputs and the actual 
        training vectors.  For more information on this structure, refer to the
        file 'parse.c'.
*/


typedef struct  {
  int   Nenums,
        Nunits;
  char  **enums;
  float **equivs,
        *unknown;
} cvrt_t;

typedef struct  {
  float   *inputs,
          *outputs;
  boolean reset;
} dv_t;

typedef struct  {
  char     *name;
  int      Npts;
  float    stdDev;
  boolean  predictOnly;
  dv_t     *data;
}  data_set_t;

typedef struct {
  char          *filename;
  out_t         *outputType;
  int           Ninputs,
                Noutputs,
                NinNodes,
                NoutNodes,
		NdataSets;
  float         binPos,
                binNeg;
  cvrt_t        *inputMap,
                *outputMap;
  data_set_t    *dataSets,
                *train,
                *validate,
                *test,
                *predict;
} data_file_t;


/*  Function Prototypes  */

boolean parse_data  ( char *, float, float, data_file_t ** );
void    free_data   ( data_file_t ** );
boolean ttof        ( float *, char **, int, cvrt_t * );
char    **ftot      ( float *, float, int, cvrt_t * );
char    *otoa       ( out_t );
#endif
