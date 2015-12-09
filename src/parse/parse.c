/*      CMU Learning Benchmark Archive
        Parse Library

	v2.0
	Matt White  (mwhite+@cmu.edu)
	3/5/95

	QUESTIONS/COMMENTS: neural-bench@cs.cmu.edu

This code has been placed in public domain by it author.  As a matter of simple
courtesy, anyone using or adapting this code is expected to acknowledge the 
source.  The author would like to hear about any attempts to use this system, 
successful or not.

The code is currently being maintained by the site contact listed above.  If 
you find a bug, add a useful feature, or discover a hack that will increase 
system performance, please contact the person at the address listed above so 
that the distribution source may be modified accordingly.

This is a library of functions for the parsing of data in the CMU Learning 
Benchmark format.  Given the name of the data file, floating point numbers to 
represent binary values, the parse_data function will build a data structure 
describing the data file and the data sets that it contains.  A function is 
also provided to free the memory associated with this data structure.

Below is a description of the use of this library.  A seperate file, 
DATA-FORMAT, describes the data file format we use in detail.  If anything 
seems unclear, feel free to contact us at the address above.


Compiling the library:

After unpacking the shell archive, which you have clearly done, you should have
the following files:

    DATA-FORMAT:   Description of the CMU Learning Benchmark data format
    parse.c:       Code for the parser
    parse.h:       External declarations for the parser
    queue.c:       Code for queue routines
    queue.h:       External declarations for queue routines
    memory.c:      Memory management routines
    xor.data:      Example data file for the XOR benchmark

Compile 'parse.c' using your favorite compiler.  The command should look
something like:

    % cc -c parse.c

In the source files where you make the call to any library functions, or access
the resulting data structures, you need to include the file "parse.h".  Then,
when you compile your program, be sure to link in the object file 'parse.o' by
including that on the compile command line.


Using the library functions:

    boolean parse_data  ( char *filename, float binPos, float binNeg, 
	                  data_file_t **dFile );

      DESCRIPTION:  Parses a data file and returns the allocated data structure
      to the calling function.  The function exits gracefully if an invalid
      data file is given to it.

      filename:  The name of the data file you wish to parse.  This does NOT
                 add any extension to the filename.  However, for your
                 information, the most common extension in use is '.data'.
      binPos/ :  Binary and enumerated series are broken down into their binary
      binNeg     componants.  These two numbers indicate the values that should
                 be assigned to binary positive and negative values, 
                 respectively.  Most commonly, this corresponds to the upper
                 and lower limits of your activation function, although other
                 variations exist.
      dFile   :  A the address of a pointer to a data file structure.  This
                 pointer should NOT have any memory allocated to it.  Any
                 memory that is allocated to that pointer will be leaked into
                 the abyss.

      RETURNS:  TRUE if parsing was successful, FALSE otherwise.


    void free_data  ( data_file_t *dFile );

      DESCRIPTION:  Frees the memory associated to a data file structure.
      Assumes that the structure is a valid data file type and has memory
      allocated to it.  No guarantees if this is not the case.

      dFile   :  A pointer to the structure to free.


    boolean ttof  ( float *retval, char **tokens, int Ntokens, cvrt_t *map )

      DESCRIPTION:  Takes an pointer to a number of string tokens and returns
      the floating point equivelant under the conversion map, 'map'.  This
      function is useful for creating query modes for neural networks trained
      using these data sets.

      retval     :  An allocated pointer where the floating point equivelance
                    is to be stored.  Be sure that adequate memory is allocated
		    for this pointer.
      tokens     :  A pointer to a number of character strings to be looked up
                    in the mapping table.
      Ntokens    :  The number of tokens to convert.
      map        :  A pointer to the first conversion map.  It is assumed that
                    there are enough conversion maps following this map to
		    convert all the tokens.

      RETURNS    :  TRUE if the tokens are successfully converted, FALSE
                    otherwise.


   char **ftot  ( float *vals, float range, int Ntokens, cvrt_t *map )
   
     DESCRIPTION:  Takes an array of floating point values and converts them
     to an array of character tokens.  This function is useful for interpreting
     the output of a neural network being queried on a particular data set.

     vals       :  An allocated pointer to the values to be converted to 
                   character tokens.
     range      :  The distance an actual value can be from the desired value
                   and still be considered equal to that training value.  For
		   querying a network, this is often set to one half the
		   distance between the positive and negative binary values.
     Ntokens    :  The number of tokens to convert.
     map        :  A pointer to the first conversion map to use when converting
                   back to string tokens.

     RETURNS    :  A pointer to pointers to strings.  That is, a number of
                   strings representing the returned tokens.


Using the library data structures:

    data_file_t  -  Master structure for a parsed data file.  Contains all the
    information available, or pointers to that information.

      filename   :  The character file name passed to the parser.
      Ninputs/   :  The number of input and output series specified in the
      Noutputs      mappings given in the data file.  Mappings for which no
                    outputs are specified are not counted here unless no
                    mapping has outputs specified, in which case Noutputs will
                    be zero.
      NinNodes/  :  The actual number of input and output units specified with
      NoutNodes     the mappings in the data file.  Since a single series may
                    require several units to represent (if it is enumerated),
                    this number may vary from Ninputs/Noutputs.  Otherwise, the
                    same rules apply.
      outputType :  An array describing each of the output nodes.  There are
                    a number of elements equal to NoutNodes, each has a value
                    of either BINARY or CONT.  BINARY means that all expected
                    outputs will either be pinned to binPos or binNeg while
                    CONT denotes that any floating point number goes.
      binPos/    :  The binPos/binNeg values specified to the parser are stored
      binNeg        here for reference.
      inputMap/  :  Both of these fields are arrays with Ninputs/Noutputs
      outputMap     elements respectively.  Each element of these arrays
                    provides a conversion map for each series in the input or
                    output vector.  Enumerated types are listed here with their
                    equivelances as well as what to substitute for unknown
                    values.
      NdataSets  :  The number of data sets that were parsed from the data
                    file.
      dataSets   :  A pointer to the array of data sets taken from the data
                    file.
      train      :  Pointer to the element of 'dataSets' that contains data
                    meant for training.
      validate   :  Pointer to the element of 'dataSets' that contains data
                    meant for cross validation.
      test       :  Pointer to the element of 'dataSets' that contains data
                    meant for testing.
      predict    :  Pointer to the element of 'dataSets' that contains data
                    meant for prediction purposes.


    cvrt_t  -  Conversion map.  Contains the tables to get from an ascii
    string to a list of floating point numbers representing that string.  Note
    that CONT values have no enumerations and an unknown array with only one
    element.  BINARY series are treated as binary enumerated values with two
    enumerations.
    
      Nenums   : Number of enumerations.  Continuous series have 0,  Binary
                 series have 2 and (bin)enumerated series have a number equal
                 to the number of tokens in between their braces.
      Nunits   : Number of nodes needed to represent this series.  Continuous
                 and binary inputs/outputs need 1.  Enumerated values depend
                 on the number of enumerations.
      enums    : An array of character strings.  There is one element for each
                 enumeration.  These represent defined enumerations present.
      equivs   : An array of floating point arrays.  There are a number of
                 arrays equal to the number of enumerations.  Each of these
                 arrays has a number of elements equal to Nunits.  The values
                 stored in these arrays are the floating point representations
                 of the enumerated values.
      unknown  : The value to insert in case an unknown turns up.  This is the
                 same as one of the equiv arrays, filled with the appropriate
                 representation in case we can't determine what the value in a
                 position actually is.  Continuous series have a one value
                 array that contains the mean value for the continuous series.


    data_set_t  -  A data set.  Contains input to output mappings for use by
    your favorite program.

      name        :  The name of the data set.
      Npts        :  The number of data vectors in the data set.
      stdDev      :  The standard deviation of the data set's outputs.
      predictOnly :  TRUE if there are no outputs in this data set.  FALSE
                     otherwise.
      data        :  An array of data vectors.  One element for each point.


    dv_t  -  A data vector.  Contains a set of inputs and the outputs those
    inputs should generate (assuming this isn't a predictOnly set).

      reset   :  For networks with short term memories, this indicates that
                 they should reset their short term memories.
      inputs  :  The floating point input vector.
      outputs :  The floating point output vector.



Revision History
~~~~~~~~ ~~~~~~~
3/5/95          2.0     A completely rewritten version for handling our revised
                        data file format.  See the DATA-FORMAT file.
2/20/94         1.0.4   Fixed a bug in which if there were continuous
                        inputs/outputs following an enumerated input
                        or output, would cause that floating point
                        value to overwrite the enumeration.
10/15/93        1.0.3   Fixed a bug that caused parse to dump core if
                        a data file is not found.
9/30/93	        1.0.2   Fixed bug that caused data sets specifying
                        a domain for continuous values to be
                        rejected as unknown node types.  Fixed
                        another bug that caused datasets with more
                        than one output to be rejected as having too
                        few outputs.  Thanks to Iain Strachan for 
                        pointing this out.
9/24/93	        1.0     Initial Release
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <math.h>

#include <toolkit.h>
#include <queue.h>
#include "parse.h"

#define  VERSION "2.0"
#define  RELDATE "5/5/95"
#define  EMLEN   81


/************************** Lexer Declarations *******************************/


/*	Constant Definitions	*/

#define MAX_TOKEN_LENGTH   210	/*  Maximum length of a character token   */
#define MAX_LINE_LENGTH    25600	/*  Maximum length of an input line       */
#define NUM_LOOKUP_ENTRIES 27	/*  Number of entries in the lookup table */
#define NUM_DELIMITERS     9	/*  Number of single character delimiters */
#define NOT_FOUND          -1	/*  Item not found                        */


typedef enum { dataT, setupT, LparenT, RparenT, commaT, rangeT, colonT, 
	       semiColonT, segMarkT, blockT, equalT, arrowT, questionT, 
	       LbracketT, RbracketT, binaryT, binEnumT, contT, enumT, forT, 
	       NseriesT, seriesT, specEnumT, stepT, toT, LbraceT, RbraceT, 
	       eofT, idT, numT } tok_t;

typedef struct token_type {
  tok_t             tok;	/*  Token type                     */
  int               line;	/*  Line the token occured on      */
  float             numVal;	/*  Numerical value for type numT  */
  char              *idVal;	/*  String value for type idT      */
} token_t;


/*	LOOKUP TABLE -  The lookup table is used by the lexer to convert
	character tokens into type 'tok_t'.  Since a binary search is used
	to search this table, it must be kept in alphabetical order and the
	constant NUM_LOOKUP_ENTRIES is correct.
*/

typedef struct {
  char  *str;
  tok_t token;
} lookup_t;

lookup_t lookupTable [] = { {"#",         blockT},
			    {"##",        segMarkT},
                            {"$data",     dataT},
                            {"$setup",    setupT},
			    {"(",         LparenT},
			    {")",         RparenT},  
			    {",",         commaT},
			    {"..",        rangeT},
			    {":",         colonT},
			    {";",         semiColonT},
			    {"=",         equalT},
			    {"=>",        arrowT},
			    {"?",         questionT},
			    {"[",         LbracketT},
			    {"]",         RbracketT},
			    {"binary",    binaryT},
			    {"binenum",   binEnumT},
			    {"cont",      contT},
			    {"enum",      enumT},
			    {"for",       forT},
			    {"nseries",   NseriesT},
			    {"series",    seriesT},
			    {"specenum",  specEnumT},  
			    {"step",      stepT},
			    {"to",        toT},
			    {"{",         LbraceT},
			    {"}",         RbraceT} };


/*	Function Prototypes	*/

void    lex             ( FILE * );
char    *get_char_token ( char *, char *, int );
token_t *lookup_token   ( char *, int );
int     search_table    ( char * );
boolean isfloat         ( char * );
boolean isint           ( char * );

void    unlex           ( void );


/**************************** Parser Declarations ****************************/

typedef struct {		/*  Information on a series  */
  tok_t type;
  int   Nenum,
        Nnodes;
  char  **enums;
  float **vals;
} ser_t;

typedef struct {		/*  Location for use in mappings  */
  char    *label;  
  int     offset;
} loc_t;

typedef struct map_type {	/*  Mapping information  */
  char            *name,
                  *index;
  int             Ninputs,
                  Noutputs,
                  step;
  loc_t           start,
                  stop;
  int             **input,
                  **output;
  struct map_type *next;
} map_t;

typedef struct dvect {		/*  Data vector node  */
  boolean      *endSeg;
  token_t      **series;
} dvect_t;

typedef struct label_type {	/*  Label node  */
  char              *label;
  int               location;
  struct label_type *next;
} lbl_t;

/*  Parsed information  */

typedef struct {
  int     Nseries,	/*  Number of series declared  */
          Nmappings,	/*  Number of mappings declared  */
          Npts;		/*  Number of data points (including segMarks)  */
  ser_t   *series;	/*  Information about the series  */
  map_t   *mappings;	/*  Information about the mappings  */
  dvect_t data;	        /*  Data vectors  */
  lbl_t   *labels;	/*  Labels  */
} parse_tree_t;


/*	Function Prototypes	*/

void         parse         ( void );
parse_tree_t *init_pt      ( void );

void         parse_setup     ( void );
void         get_Nseries     ( int, int *, ser_t ** );
void         get_serdec      ( int, int, ser_t * );
void         get_enums       ( int, int *, char *** );
void         get_spec_enums  ( int, int *, int *, char ***, float *** );
void         get_mapping     ( int );
void         link_mapping    ( map_t *, int );
void         get_map         ( tok_t, char *, int ***, int * );

void         parse_data_vects  ( void );
void         get_label         ( void );
void         get_segmark       ( void );
void         get_data_line     ( void );

void         consume     ( tok_t );
void         integer     ( int * );
void         number      ( float * );
void         identifier  ( char ** );
tok_t        head        ( int *, float *, char ** );
token_t      first       ( void );
tok_t        peek_tok    ( void );
int          peek_line   ( void );
int          count_tok   ( tok_t, tok_t );

void         unparse      ( void );
void         print_set    ( dvect_t, int );


/********************* Interpreter Declarations ******************************/

data_file_t *interp        ( float, float, char * );
data_file_t *init_dataset  ( char *, float, float );
float       *get_means     ( int, ser_t *, dvect_t );
float       calc_mean      ( dvect_t *, int );

void        setup_equivs   ( parse_tree_t *, data_file_t *, float * );
void        get_equivs     ( int **, ser_t *, float *, float, float, int, 
                             int *, cvrt_t ** );
cvrt_t      bin_equiv      ( float, float );
cvrt_t      enum_equiv     ( ser_t, float, float, boolean );
cvrt_t      senum_equiv    ( ser_t );
int         num_bin        ( int );
void        get_types      ( int, int, cvrt_t *, out_t ** );

data_set_t  gen_dataset    ( data_file_t *, map_t *, dvect_t, lbl_t * );
int         lookup_label   ( char *, lbl_t * );
float       *gen_datavect  ( int, int, cvrt_t *, int **, dvect_t * );
float       get_cont_data  ( token_t *, float );
float       *get_enum_data ( token_t *, cvrt_t * );
float       *lookup_ident  ( char *, cvrt_t *, int );
float       calc_std_dev   ( dv_t *, int, int );

void        uninterp       ( data_file_t * );
void        print_map      ( cvrt_t );
void        print_data     ( data_set_t, int, int );
 

/********************* General Declarations **********************************/

/*	Global variables for error recovery	*/

static       jmp_buf error_trap;
queue_p      tStream;
parse_tree_t *parseTree;
data_file_t  *dFilePtr;

int          Ndvects;		/*  An estimate of the number of data    */
	          		/* vectors in the data set.  This number */
	 			/* bounds the actual number from above.  */
float        binaryPos,		/*  Values of binary positive and        */
             binaryNeg;		/* negative, respectively.		 */


/*	Function Prototypes	*/

void    print_banner    ( void );
FILE    *init_parse     ( char * );

void    parse_err       ( int, char * );
void    freeTStream     ( token_t * );
void    freePTree       ( parse_tree_t * );
void    freeSer         ( ser_t *, int );
void    freeMap         ( map_t * );
void    freeData        ( dvect_t *, int );
void    freeLabels      ( lbl_t * );

void    free_cmap       ( cvrt_t );
void    free_data_set   ( data_set_t );

char    *ttoa           ( tok_t );



/************************ Visible Functions **********************************/

/*	PARSE DATA -  The main function for parsing data files in the CMU 
	Data File Format.  'filename' is the name of the file to parse, 
	'binPos' and 'binNeg' are the values to assign positive and negative
	boolean values, respectively.  'data' is the address of a pointer
	where the data should be stored.  This pointer does NOT need to be
	allocated.  parse_data returns TRUE if the data file parsed 
	successfully, FALSE otherwise.
*/

boolean parse_data  ( char *filename, float binPos, float binNeg, 
		      data_file_t **data )
{
  if  ( setjmp( error_trap ) != 0 )
    return FALSE;

  binaryPos = binPos;
  binaryNeg = binNeg;

  print_banner( );
  lex( init_parse( filename ) );
  parse( );
  *data = interp( binPos, binNeg, filename );
  dFilePtr = NULL;

  return TRUE;
}


/*	FREE DATA -  Deallocate memory associated with 'data'.  This includes
	all conversion tables and data vectors.
*/

void free_data  ( data_file_t **data )
{
  int  i;

  if ( *data == NULL )
    return;
  if ( (*data)->filename != NULL )
    free( (*data)->filename );
  if ( (*data)->outputType != NULL )
    free( (*data)->outputType );
  
  if  ( (*data)->inputMap != NULL )  {
    for  ( i = 0 ; i < (*data)->Ninputs ; i++ )
      free_cmap  ( (*data)->inputMap[i] );
    free( (*data)->inputMap );
  }

  if  ( (*data)->outputMap != NULL )  {
    for  ( i = 0 ; i < (*data)->Noutputs ; i++ )
      free_cmap  ( (*data)->outputMap[i] );
    free( (*data)->outputMap );
  }

  if  ( (*data)->dataSets != NULL )  {
    for  ( i = 0 ; i < (*data)->NdataSets ; i++ )
      free_data_set  ( (*data)->dataSets[i] );
    free( (*data)->dataSets );
  }

  free( *data );
  *data = NULL;
}


/*	TOKEN TO FLOAT -  Converts an array of strings into an array of
	floating point numbers.  See the header information for usage
	information.
*/

boolean ttof ( float *retval, char **tokens, int Ntokens, cvrt_t *map )
{
  boolean found; 
  int     i,j,
          node = 0;
  float   *val;
  
  for ( i = 0 ; i < Ntokens ; i++ )  {
    if  ( map[i].Nenums == 0 )  {
      if  ( isfloat( tokens[i] ) )
	retval[node++] = ( isint( tokens[i] ) ) ? atoi( tokens[i] ) : 
	  atof( tokens[i] ); 
      else  {
	fprintf (stderr, "\nERROR:  Was expecting a floating point number");
	fprintf (stderr, " and found %s.\n", tokens[i]);
	return FALSE;
      }
      continue;
    }

    if  (!strcmp( tokens[i], "?" ) || !strcmp( tokens[i], "#" ))  {
      for  ( j = 0 ; j < map[i].Nunits ; j++ )
	retval[node++] = map[i].unknown[j];
      continue;
    }

    found = FALSE;
    for  ( j = 0 ; j < map[i].Nenums ; j++ )
      if  ( !strcmp( tokens[i], map[i].enums[j] ) )  {
	val   = map[i].equivs[j];
	found = TRUE;
	break;
      }
    if  ( !found )  {
      fprintf (stderr, "\nERROR:  Unknown identifier %s\n", tokens[i] );
      return FALSE;
    }

    for  ( j = 0 ; j < map[i].Nunits ; j++ )
      retval[node++] = val[j];
  }

  return TRUE;
}


/*	FLOAT TO TOKEN -  Converts a vector of floating point numbers into its
	corrosponding character representation.  For information on usage, see
	the header information at the top of this file.
*/

char **ftot ( float *vals, float range, int Ntokens, cvrt_t *map )
{
  int     node = 0,
          tnode,
          i,j,k;
  boolean found;
  char    **temp,
          *fn = "Float to token";

  temp = (char **)alloc_mem(Ntokens, sizeof( char * ), fn);

  for  ( i = 0 ; i < Ntokens ; i++ )  {
    if  ( map[i].Nenums == 0 )  {
      temp[i] = (char *)alloc_mem( MAX_TOKEN_LENGTH, sizeof( char ), fn );
      sprintf ( temp[i], "%f", vals[node++] );
      continue;
    }

    j = 0;
    while ( TRUE )  {
      if  ( j == map[i].Nenums )  {
	temp[i] = strdup("?");
	break;
      } else {
	found = TRUE;
	for  ( k = 0, tnode = node; k < map[i].Nunits ; k++, tnode++ )
	  if  ( fabs( vals[tnode] - map[i].equivs[j][k] ) > range )  {
	    found = FALSE;
	    j++;
	    break;
	  }
	if  ( found )  {
	  temp[i] = strdup( map[i].enums[j] );
	  break;
	}
      }
    }
    node += map[i].Nunits;
  }

  return temp;
}

/********************************* Lexer *************************************/


/*	LEX -  This is the main lexing function.  It takes a file pointer and
	lexes the file into a linked list of tokens.  I call this a 'token
	stream'.  It's not really, since there is no delayed computation.
*/

void lex  ( FILE *fptr )
{
  char    inputLine [MAX_LINE_LENGTH],
          charToken [MAX_TOKEN_LENGTH],
          *linePtr = NULL;
  boolean dataSeg;
  int     lineNum = 0;
  token_t *newToken,
          lastToken;


  fprintf (stderr,"Lexing...");
  fflush  (stderr);

  tStream = queue_alloc( 0, sizeof( token_t ) );

  while  ( !feof( fptr ) || (linePtr != NULL) )  {

    /*  Get new line if the current line is NULL.  If we get a NULL line,  */
    /* then we have reached the end of the file.                           */

    if  ( linePtr == NULL )
      if  ( (linePtr = fgets ( inputLine, MAX_LINE_LENGTH, fptr )) == NULL )
	continue;
      else
	lineNum++;

    /*  Get the next character token from the line and store it in  */
    /* charToken.  linePtr is updated to the next character after   */
    /* this token.                                                  */

    linePtr = get_char_token ( linePtr, charToken, lineNum );

    /*  If we didn't get a null token, we convert this token into an  */
    /* element of our token list.  We then link the new token to the  */
    /* current list.  If this is the first token, we also set         */
    /* tStreamPtr to it, which allows us to deallocate memory in case */
    /* of an error.                                                   */

    if  ( *charToken != '\0' )  {
      newToken =  lookup_token ( charToken, lineNum );
      if  ( newToken->tok == dataT )  dataSeg = TRUE;
      if  ( dataSeg && 
	   ((newToken->tok == semiColonT) || (newToken->tok == segMarkT)) )  
	Ndvects++;
      enqueue( tStream, (void *)newToken );
    }
  }

  /*  After we have parsed the entire data file, we need to append the  */
  /* end of file token onto the end of the list.                        */

  lastToken.tok  = eofT;
  enqueue( tStream, (void *)&lastToken );

  fclose( fptr ); 
}


/*	GET CHAR TOKEN -  Get a token from the line pointed to by 'line'.
	Store the new token in 'token' and return an updated pointer to that 
	line.  'token' should have already have memory allocated to it.
	lineNum is used in case of error to indicate what line this error
	occurred on.
*/

char *get_char_token  ( char *line, char *token, int lineNum )
{
  static char *seperators = "(),:[]{};+-=\n\t ",/*  Token seperators        */
              *whitespace = "\t\n ";            /*  Valid whitespace chars  */
  char        *cptr;                            /*  Temp character pointer  */
  int         index;                            /*  Array index             */

  /*  Eliminate leading whitespace as defined by 'whitespace'  */

  line += strspn ( line, whitespace );

  /*  If all that's left is the terminating NULL, return a NULL token  */

  if  ( line[0] == '\0' )  {
    token[0] = '\0';
    return NULL;
  }

  /*  Check for the '=' and '=>' delimiters  */

  if  (line[0] == '=')
    if (line[1] == '>')  {
      strcpy ( token, "=>" );
      return line+2;
    } else {
      strcpy ( token, "=" );
      return line+1;
    }

  /*  Check to see if the token begins with either a plus or a minus.        */
  /* If this is not the case, first cycle through the first NUM_DELIMITERS   */
  /* of the seperators.  If any of these are the first character on the      */
  /* line, return it as the character token.  Now check for a '..' token and */
  /* return that if it is found.  Last we check for a leading apostrophe.  A */
  /* leading apostrophe causes it and the next character to be copied to be  */
  /* copied blindly to the character token.  Lexing resumes normally with    */
  /* the next character.                                                     */
  /*  If the first character is a +/-, then there are two things that can    */
  /* happen:  1) it can be a boolean or enumerated value  2) it can be part  */
  /* of a number.  If it is boolean or enumerated value, it will be folowed  */
  /* by another delimeter.  If it is part of a number, then there can be     */
  /* be some amount of whitespace and then the number.  In either case, we   */
  /* can copy the +/-, ignore the following whitespace, and then start       */
  /* looking for another delimeter.                                          */

  if  ( (line[0] == '+') || (line[0] == '-') )  {
    token[0] =  line[0];
    line     += strspn( line+1, whitespace )+1;
    index    =  1;
  }  else  {
    cptr = seperators;
    for  ( index = NUM_DELIMITERS ; index > 0 ; index-- )  {
      if  ( *line == *cptr )  {
	*token     = *cptr;
	*(token+1) = '\0';
	if  ( *cptr == ';' )
	  return NULL;
	else
	  return ++line;
      }
      cptr++;
    }
  
    if  ((line[0] == '.') && (line[1] = '.'))  {
      strcpy ( token, ".." );
      return line+2;
    }
    
    if  ( *line == 0x27 )  {
      token[0] = line[0];
      token[1] = line[1];
      line += 2;
      index = 2;
    } else
      index = 0;
  }
    
  /*  This is where the majority of character tokens are discerned.  We  */
  /* simply copy characters until a seperator is encountered.  Note that */
  /* '..' is a seperator as well as those listed in the 'seperators'     */
  /* string.  Also, if we try to go over MAX_TOKEN_LENGTH, we flame out. */

  while  ( (line[0] != '\0') && (strspn( line, seperators ) == 0) )  {
    if  ( (line[0] == '.') && (line[1] == '.') )
      break;
    token [index++] = *line;
    if  ( index == MAX_TOKEN_LENGTH )
      parse_err  ( lineNum, "Token too long" );
    line++;
  }
  token [index] = '\0';

  return line;
}


/*	LOOKUP TOKEN -  Decide whether this is a normal token, number or misc
	identifier.  Allocate memory for a new token and initialize it's
	values as appropriate for it's type.  Return the resulting token to
	the caller.  charTok is the token to lookup and lineNum is the line
	number to associate with the new token.
*/
 
token_t *lookup_token  ( char *charTok, int lineNum )
{
  int             refNum;               /*  Token's location in the lookup  */
                                        /* table  */
  char            *fn = "Lookup Token"; /*  Function name in case we flame  */
  static token_t token;                 /*  New token being created */
  

  /*  Search the table for the token.  If the token is found, initialize  */
  /* the newly allocated token structure and return it to the caller.     */

  if  ( (refNum = search_table ( charTok )) >= 0 )  {
    token.tok  = lookupTable[refNum].token;
    token.line = lineNum;
    return &token;
  }

  /*  If this character string is a number, we return a numerical token.    */
  /* Since atof doesn't work on numbers without a decimal, we have to check */
  /* whether one exists or not.  Note that all numbers are stored as        */
  /* standard floats.                                                       */

  if  ( isfloat( charTok ) )  {
    token.tok    = numT;
    token.line   = lineNum;
    token.numVal = (isint( charTok )) ? atoi( charTok ) : atof( charTok );
    return &token;
  }

  /*  This token does not fit one of the two previous molds and so is       */
  /* considered an identifier.  We copy the character string into the new   */
  /* token's idVal field and return the newly created token.                */

  token.tok   = idT;
  token.line  = lineNum;
  token.idVal = (char *)alloc_mem ( strlen( charTok )+1, sizeof( char ), fn );
  strcpy ( token.idVal, charTok );
  return &token;
}


/*	SEARCH TABLE -  Search the token lookup table for 'token'. The search 
	is NOT case sensitive.  It returns NOT_FOUND if the token is not found
	in the table.  Otherwise, it returns the character token's location in
	the table.  The algorithm is a standard binary search.
*/

int search_table  ( char *token )
{
  int begin = 0,
      end   = NUM_LOOKUP_ENTRIES-1,
      loc,
      dir;
  
  while  ( begin <= end )  {
    loc = (begin+end)/2;
    if  ( (dir = strcasecmp( token, lookupTable[loc].str )) == 0 )
      return loc;
    else if  ( dir < 0 )
      end = loc - 1;
    else
      begin = loc + 1;
  }
  
  return NOT_FOUND;
}


/*	UNLEX -  Goes through the token list and prints out the values of all
	the tokens, in order.  This function is not used anywhere, but is
	useful for debugging purposes.
*/

void unlex  ( void )
{
  token_t tok;

  while  ( dequeue( tStream, (void *)&tok ) )  {
    switch (tok.tok)  {
    case numT: printf ("%s(%f) ",ttoa(tok.tok),tok.numVal);
      break;
    case idT:  printf ("%s(%s) ",ttoa(tok.tok),tok.idVal);
      break;
    default:   printf ("%s ",ttoa(tok.tok));
      break;
    }
  }
  printf ("\n");
}



/*********************************** Parser **********************************/

/*	PARSE -  The second major phase of this process.  Accepts a list of
	tokens 'tStream', from the lexer and extracts useful information from
	them.  This information is assembled into a 'parse tree'.  Again, this
	really isn't a tree, but more of a lookup table.
*/

void parse  ( void )
{
  int          lineNum;		/*  Current line number                */
  tok_t        token;		/*  Token from the head of the stream  */

  fprintf (stderr,"Parsing...");
  fflush  (stderr);

  parseTree = init_pt( );
  
  /*  Discard any leading semicolons and look for the $SETUP section.  */

  while ( (token = head( &lineNum, NULL, NULL )) == semiColonT );
  if ( token != setupT )
    parse_err ( lineNum, "No setup section found" );

  /*  Get the infromation from the setup section and then look for the data  */
  /* section.  After we have parsed the data section, consume the EOF token, */
  /* which should be the last token on the list.                             */

  parse_setup( );
  consume( dataT );
  parse_data_vects( );
  consume( eofT );
  queue_free( &tStream );
}


/*	INIT PT -  Allocate memory for a new parse tree and initialize it's
	fields to resonable values.
*/

parse_tree_t *init_pt ( void )
{
  parse_tree_t *tree;
  char         *fn = "Init Parse Tree";
  
  tree = (parse_tree_t *)alloc_mem( 1, sizeof( parse_tree_t ), fn );
  
  tree->Nseries   = 0;
  tree->Nmappings = 0;
  tree->Npts      = 0;

  tree->series   = NULL;
  tree->mappings = NULL;
  tree->labels   = NULL;
  
  return tree;
}


/*	PARSE SETUP -  Parse the setup section of the data file.  We are
	looking for 4 things:

        1) NseriesT is taken as the beginning of a declaration of the number
	   of series.
	2) seriesT is a type declaration for a specific series.
	3) idT is a name associated to a mappings, which is assumed to follow.
	4) semiColonT is an extraneous semicolon and is ignored.

	Anything else is bogus and is flagged as an error.
*/

void parse_setup  ( void )
{
  char     errMess [EMLEN];

  while ( TRUE )
    switch ( peek_tok( ) )  {
    case NseriesT:   get_Nseries ( peek_line( ), &(parseTree->Nseries),
				   &(parseTree->series) );
                     break;
    case seriesT:    get_serdec  ( peek_line( ), parseTree->Nseries, 
				   parseTree->series );
                     break;
    case idT:        get_mapping ( peek_line( ) );
                     break;
    case dataT:      return;
    case semiColonT: consume( semiColonT ); 
                     break;

    default:         sprintf   ( errMess, "Unexpected token '%s'", 
			         ttoa( peek_tok( ) ) );
                     parse_err ( peek_line( ), errMess );
    }
}


/*	GET NSERIES -  Get the number of series from the token stream.
	Allocate memory for the type descriptions.  It is considered an error
	to either redeclare the number of series or to declare a 
	non-positive number.
*/

void get_Nseries ( int lineNum, int *num, ser_t **types )
{
  if  ( *num != 0 )
    parse_err ( lineNum, "Attempt to reset Nseries after already set" );

  consume( NseriesT );
  consume( colonT );
  integer( num );
  consume( semiColonT );

  if  ( *num < 1 )
    parse_err ( lineNum, "Number of series must be positive" );
  *types = (ser_t *)alloc_mem(*num,sizeof( ser_t ),"Get Number Inputs/Outputs");
}


/*	GET SERDEC -  Get a type declaration for a series.  The series (or
	range of series) is described as either: cont, binary, enum or 
	binenum.  Enums and binenums also have attached lists of enumerations.
*/

void get_serdec ( int lineNum, int totalNum, ser_t *types )
{
  tok_t   token;			/*  Type of the series  */
  int     num,		        	/*  Index of the series  */
          endNum,			/*  Index of the end (ranges only)  */
          i;
  char    errMess [EMLEN];
  
  /*  Get the beginning and ending index from the token stream.  If a range  */
  /* is present, then that is read as well.  In either case, tokens are read */
  /* through the delimiting colon.  Both the beginning and ending indices    */
  /* then checked to see if they are in range.                               */

  consume( seriesT );
  consume( LbracketT );
  integer( &num );
  
  if  ( peek_tok( ) == rangeT )  {
    consume( rangeT );
    integer( &endNum );
    consume( RbracketT );
    consume( colonT );
  } else {
    consume( RbracketT );
    consume( colonT );
    endNum = num;
  }
  
  if  ( (num < 1) || (num > totalNum) )  {
    sprintf   ( errMess, "Index (%d) is out of range", num );
    parse_err ( lineNum, errMess );
  }
  
  if  ( (endNum < 1) || (endNum > totalNum) )  {
    sprintf   ( errMess, "Index (%d) is out of range", endNum );
    parse_err ( lineNum, errMess );
  }
  
  num--;	/*  Indices in the data files are 1-based, while C is  */
  endNum--;	/* 0-based.                                            */
  
  /*  Get the type from the token stream.  If this is an enumerated type,  */
  /* parse the enumerations as well.                                       */

  switch ( (token = head( &lineNum, NULL, NULL )) )  {
    case binaryT:
    case contT:    types[num].type = token;
                   types[num].Nenum = 0;
                   types[num].enums = NULL;
                   break;
    case enumT:
    case binEnumT: types[num].type = token;
                   get_enums ( lineNum, 
			       &(types[num].Nenum), &(types[num].enums) );
                   break;
    case specEnumT:types[num].type = token;
                   get_spec_enums  ( lineNum, &(types[num].Nenum),
				     &(types[num].Nnodes),
				     &(types[num].enums), &(types[num].vals) );
                   break;
    default:       sprintf ( errMess, "Unexpected token '%s'", ttoa( token ) );
                   parse_err ( lineNum, errMess );
  }

  /*  If the same declaration holds over a number of series, copy this  */
  /* information into those series.                                     */
  
  for  ( i = num+1 ; i <= endNum ; i++ )
    types[i] = types[num];
  
  consume( semiColonT );
}


/*	GET ENUMS -  Get the character enumerations from the token stream.
	The number of enumerations is determined by counting the number of 
	commas and adding one.  If no enumerations are present, an error is
	flagged.
*/

void get_enums  ( int lineNum, int *Nenums, char ***enums )
{
  int     i;
  char    *fn = "Get enumerations";

  /*  Determine the number of enumerations and allocate memory for them.  */
  
  consume( LbraceT );
  if  ( peek_tok( ) == RbraceT )
    parse_err ( lineNum, 
	        "Enumerated values must have at least one enumeration" );
  *Nenums = count_tok( commaT, RbraceT ) + 1;
  *enums = (char **)alloc_mem( *Nenums, sizeof( char * ), fn );

  /*  Consume the identifiers and their trailing commas.  Since the last  */
  /* identifier is followed by a brace instead of a comma, it rates a     */
  /* special case.                                                        */

  for  ( i = 0 ; i < (*Nenums-1) ; i++ )  {
    identifier( (*enums)+i );
    consume( commaT );
  }
  identifier( (*enums)+(*Nenums)-1 );
  consume( RbraceT );
}


/*	GET SPECIFIED ENUMERATIONS -  Get the enumerations in the case that the
	user has specified what each enumeration should corrospond to.
*/

void get_spec_enums  ( int lineNum, int *Nenums, int *Nnodes, char ***enums, 
		       float ***vals )
{
  int  i,j;
  char *fn = "Get Specified Enums",
       *ctok;

  consume( LbraceT );

  *Nenums = count_tok( equalT, RbraceT );
  *enums  = (char **)alloc_mem( *Nenums, sizeof( char * ), fn );
  *vals   = (float **)alloc_mem( *Nenums, sizeof( float * ), fn );
  *Nnodes = 0;

  for  ( i = 0 ; i < (*Nenums) ; i++ )  {
    identifier( (*enums)+i );
    consume( equalT );
    consume( LparenT );
    if  ( *Nnodes == 0 )
      *Nnodes = count_tok ( commaT, RparenT )+1;
    (*vals)[i] = (float *)alloc_mem( *Nnodes, sizeof( float ), fn );
    for  ( j = 0 ; j < (*Nnodes) ; j++ )  {
      identifier( &ctok );
      if  ( j == (*Nnodes)-1 )
	consume( RparenT );
      else
	consume( commaT );
      if  ( !strcasecmp( ctok, "+" ) )
	(*vals)[i][j] = binaryPos;
      else if  ( !strcasecmp( ctok, "-" ) )
	(*vals)[i][j] = binaryNeg;
      else
	parse_err( lineNum, "Specified enumeration values must be + or -" );
      free( ctok );
    }

    if ( i == (*Nenums)-1 )
      consume( RbraceT );
    else
      consume( commaT );
  }
}

  
/*	GET MAPPING -  Read a mapping from the token stream.  No magic here.
	Just read things in and put them in their proper slots.
*/

void get_mapping  ( int lineNum )
{
  loc_t   *loc;    /*  Pointer to the location we're storing  */
  int     i;
  map_t   *newMap;

  /*  Get the: <name>: for <index> = part of the mapping expression  */

  newMap = (map_t *)alloc_mem(1,sizeof( map_t ), "Get Mapping");
  identifier( &(newMap->name) );
  consume( colonT );
  consume( forT );
  identifier( &(newMap->index) );
  consume( equalT );

  /*  Get the beginning and ending locations from the token stream.  Form is */
  /* is to get the label's id, if any, and then the offset, if any.  If      */
  /* neither is present, we flame out.                                       */

  loc = &(newMap->start);
  for ( i = 0 ; i < 2 ; i++ )  {
    if ( peek_tok( ) == idT )
      identifier( &(loc->label) );
    else
      loc->label = NULL;

    if ( peek_tok( ) == numT )
      integer( &(loc->offset) );
    else  {
      if  ( loc->label == NULL )
	parse_err ( lineNum, "Neither label nor numerical value present" );
      loc->offset = 0;
    }
    
    if  ( i == 0 )  {
      consume( toT );
      loc = &(newMap->stop);
    }
  }

  /*  Get the step size for this mapping.  A step size can be any positive   */
  /* integer.  If no step size is specified, assign an automatic value of 1. */

  if  ( peek_tok( ) == stepT )  {
    consume( stepT );
    integer( &(newMap->step) );
    if  ( newMap->step <= 1 )
      parse_err ( lineNum, "Step size must be positive" );
  }  else
    newMap->step = 1;

  /*  First get the input mapping and then the output mapping.  */

  get_map( arrowT, newMap->index, &(newMap->input), &(newMap->Ninputs) );
  get_map( semiColonT, newMap->index, &(newMap->output), &(newMap->Noutputs) );

  /*  Link the mapping to the existing mappings.  */

  link_mapping( newMap, lineNum );
}


/*	LINK MAPPING -  Check a new mapping against existing ones to see
	if they are consistant.  An invariant is maintained that if a mapping
	exists that has outputs specified, it will be first in the list.  If
	no mappings have outputs, then any order can exist.

	A mapping is said to be consistant if it has the same number of inputs
	and outputs and if these inputs and outputs come from the same series
	as each other.  Variance is allowed as to WHERE from each of these
	series the i/o points come from.  In addition, any mapping is allowed
	to have NO outputs, if desired.

	After a mapping is determined to be consistant with previous mappings,
	it is linked in so as to preserve the invariance.
*/

void link_mapping  ( map_t *newMap, int lineNum )
{
  int i;

  if  ( parseTree->mappings != NULL )  {

    /*  Check inputs  */

    if  ( parseTree->mappings->Ninputs != newMap->Ninputs )
      parse_err ( lineNum,"Ninputs must be consistant across mappings" );
    for  ( i = 0 ; i < newMap->Ninputs ; i++ )
      if  ( parseTree->mappings->input[i][0] != newMap->input[i][0] )
	parse_err(lineNum,"Inputs must have consistant types across mappings");

    /*  Check outputs  */

    if  ( parseTree->mappings->Noutputs != 0 )  {
      if  ( newMap->Noutputs != 0 )  {
	if  ( newMap->Noutputs != parseTree->mappings->Noutputs )
	  parse_err(lineNum, "Noutputs must be consistant across mappings");
	for  ( i = 0 ; i < newMap->Noutputs ; i++ )
	  if  ( parseTree->mappings->output[i][0] != newMap->output[i][0] )
	    parse_err(lineNum,"Outputs must be consistant across mappings");

    /*  Link the new mapping to current ones  */

	newMap->next              = parseTree->mappings;
	parseTree->mappings       = newMap;
      }  else  {
	newMap->next              = parseTree->mappings->next;
	parseTree->mappings->next = newMap;
      }
    }  else  {
      newMap->next        = parseTree->mappings;
      parseTree->mappings = newMap;
    }
  }  else  {
    newMap->next          = NULL;
    parseTree->mappings   = newMap;
  }

  parseTree->Nmappings++;
}


/*	GET MAP -  Get an input or output mapping from the token stream.  Note
	that we only record offsets.  There are a number of elements for each
	mapping equal to the number of times the word "series" appears before
	the "=>".  There are two offsets for each element, the first is the
	series number to reference and the second is the temporal offset.  In
	other words 'series[3,i+4]' is represented as [2,4] internally.
*/

void get_map  ( tok_t endDelim, char *index, int ***offsets, 
	        int *num )
{
  char *idx,
       errMess [EMLEN];
  int  i;

  /*  Get the number of elements in the mapping and allocate memory for the  */
  /* offsets.                                                                */
  
  *num = count_tok (  seriesT, endDelim );
  if  (*num == 0)  {
    *offsets = NULL;
    return;
  }
  *offsets = (int **)alloc_mem(*num,sizeof( int * ), "Get Map");

  /*  Read in a single mapping and record its offsets.  Decrement the series */
  /* number because array indices start at '0' in C.                         */

  for  ( i = 0 ; i < *num ; i++ )  {
    (*offsets)[i] = (int *)alloc_mem( 2, sizeof( int ), "Get Map" );
    consume( seriesT );
    consume( LbracketT );
    integer( &((*offsets)[i][0]) );
    consume( commaT );
    identifier( &idx );
    if  ( strcasecmp( idx, index ) )  {
      sprintf   ( errMess, "Unknown identifier '%s' found in mapping", idx );
      parse_err ( -1, errMess );
    }
    if  ( peek_tok( ) == numT )  {
      integer( &((*offsets)[i][1]) );
      consume( RbracketT );
    } else {
      (*offsets)[i][1] = 0;
      consume( RbracketT );
    }
    if  ( i != (*num - 1) )
      consume( commaT );
    else
      consume( endDelim );
    
    (*offsets)[i][0]--;
  }
}


/*	PARSE DATA VECTS -  Central dispatch function for parsing the $DATA
	section.  The only things we want are data, semicolons, segment
	markers, labels and the EOF token.
*/

void parse_data_vects  ( void )
{
  token_t block = {blockT,0,0,NULL};
  int     i;
  char    *fn = "Parse Data Vectors";

  Ndvects+=2;
  parseTree->data.endSeg = (boolean *)alloc_mem( Ndvects, sizeof( boolean ),
						  fn );
  parseTree->data.series = (token_t **)alloc_mem( parseTree->Nseries,
						  sizeof( token_t * ), fn );
  for  ( i = 0 ; i < parseTree->Nseries ; i++ )  {
    parseTree->data.series[i] = (token_t *)alloc_mem( Ndvects, 
						      sizeof( token_t ), fn );
    parseTree->data.series[i][0] = block;
  }
  parseTree->data.endSeg[0] = TRUE;
  parseTree->Npts++;

  while  ( TRUE )
    switch  ( peek_tok( ) )  {
      case LbracketT:  get_label( );
	               break;
      case segMarkT:   get_segmark( );
	               break;
      case semiColonT: consume( semiColonT );
	               break;
      case eofT:       for  ( i = 0 ; i < parseTree->Nseries ; i++ )
	                 parseTree->data.series[i][parseTree->Npts] = block;
	               parseTree->data.endSeg[parseTree->Npts++] = TRUE;
	               return;

      default:         get_data_line( );
	               break;
      }
}


/*	GET LABEL -  Read in a label and record its information in a
	structure.  Note that we keep a pointer to the place in the data
	*before* where the label occurred.  This allows us quick access to
	label spots during interpretation.
*/

void get_label  ( void )
{
  lbl_t *newLabel;

  newLabel = (lbl_t *)alloc_mem ( 1, sizeof( lbl_t ), "Get Label");

  consume( LbracketT );
  identifier( &(newLabel->label) );
  consume( RbracketT );

  newLabel->location = parseTree->Npts;
  newLabel->next     = parseTree->labels;
  parseTree->labels  = newLabel;
}


/*	GET SEGMARK -  Insert a segment marker into the data at the current
	location.
*/

void get_segmark  ( void )
{
  int     i;
  token_t block = {blockT,0,0,NULL};

  parseTree->data.endSeg[parseTree->Npts] = TRUE;
  for  ( i = 0 ; i < parseTree->Nseries ; i++ )
    parseTree->data.series[i][parseTree->Npts] = block;

  parseTree->Npts++;
  consume( segMarkT );
}


/*	GET DATA LINE -  Read in a line of data.  If the number of data points
	on the line does not agree with Nseries, an error will be flagged.
	Otherwise, no type checking takes place at this point.  Note that we
	simply store the tokens where the data occurs in an array.
*/

void get_data_line  ( void )
{
  dvect_t *newVect;
  token_t *nextTok;
  int     i;

  for  ( i = 0 ; i < parseTree->Nseries ; i++ )  {
    parseTree->data.endSeg[parseTree->Npts]    = FALSE;
    parseTree->data.series[i][parseTree->Npts] = first( );
    if  ( i == parseTree->Nseries-1 )
      consume( semiColonT );
    else
      consume( commaT );
  }
  parseTree->Npts++;
}


/*	CONSUME -  Get the head of the token stream and compare it with
	constraint.  If they are the same, return the rest of the token list,
	otherwise flag a parse error.
*/

void consume   ( tok_t constraint )
{
  int     lineNum;
  char    errMess [EMLEN];
  tok_t   token;
  
  token = head ( &lineNum, NULL, NULL );
  
  if  ( token == constraint )
    return;
  
  sprintf   ( errMess, "Unexpected token.  Expected %s and found %s", 
	      ttoa( constraint ), ttoa( token ) );
  parse_err ( lineNum, errMess );
}


/*	INTEGER -  Read an integer from the token stream.  The token must be
	a numT and an integer value, or an error is signalled.
*/

void integer  ( int *val )
{
  tok_t token;
  char  errMess [EMLEN];
  int   lineNum;
  float numVal;
  
  if  ( (token = head ( &lineNum, &numVal, NULL )) != numT )  {
    sprintf ( errMess, "Unexpected token.  Expected an integer and found %s",
	     ttoa( token ) );
    parse_err ( lineNum, errMess );
  }
  if  ( fmod (numVal,1.0) != 0.0 )  {
    sprintf   ( errMess, "Numerical value %f must be an integer", numVal );
    parse_err ( lineNum, errMess );
  }
  
  *val = (int) numVal;
}


/*	NUMBER -  Read a number from the token stream.  The token must be a
	numT.
*/

void number  ( float *val )
{
  char  errMess [EMLEN];
  int   lineNum;
  tok_t token;
  
  if  ( (token = head ( &lineNum, val, NULL )) != numT )  {
    sprintf ( errMess, "Unexpected token.  Expected a number and found %s",
	     ttoa( token ) );
    parse_err ( lineNum, errMess );
  }
}


/*	IDENTIFIER -  Get an identifier from the token stream.  If the next
	token is not idT, flag an error.  Return the remains of the token list.
*/

void identifier  ( char **idVal )
{
  tok_t token;
  char  errMess [EMLEN];
  int   lineNum;
  
  token = head ( &lineNum, NULL, idVal );
  if  ( token != idT )  {
    sprintf ( errMess, 
	     "Unexpected token.  Expected an identifier and found %s",
	     ttoa( token ) );
    parse_err ( lineNum, errMess );
  }
}


/*	HEAD -  Eat the first token from the start of the token stream.  If
	it is either a numT or idT, and the appropriate parameter is not set
	to NULL, store it's value there.  Destructively modifies tStream and
	returns the token read.  The line number is stored in 'line'.
*/

tok_t head  ( int *line, float *numValue, char **idValue )
{
  tok_t   token;
  token_t temp;
  
  dequeue ( tStream, (void *)&temp );
  token    = temp.tok;
  *line    = temp.line;
  
  switch ( token )  {
  case idT:  if ( idValue != NULL )
               *idValue = temp.idVal;
             else
               free( temp.idVal );
             break;
  case numT: if ( numValue != NULL )
               *numValue = temp.numVal;
             break;
  }

  return token;
}


/*	FIRST -  Unlink the first token from the list and return it.  tStream
	is destructively modified.
*/

token_t first ( void )
{
  token_t temp;
  
  dequeue( tStream, (void *)&temp );

  return temp;
}


/*	PEEK TOKEN -  Peek at the next token in the token queue without
	removing it.
*/

tok_t peek_tok ( void )
{
  token_t *nextTok;

  nextTok = (token_t *)queue_peek( tStream );

  return nextTok->tok;
}


/*	PEEK LINE -  Peek at the next line number in the token queue without
	removing it.
*/

int peek_line ( void )
{
  token_t *nextTok;
  
  nextTok = (token_t *)queue_peek( tStream );

  return nextTok->line;
}


/*	COUNT TOK -  Count the number of tokens of type cTok occur in the
	token stream before a token of type endTok.
*/

int count_tok ( tok_t cTok, tok_t endTok )
{
  int     count = 0;
  queue_p temp;
  token_t token;

  temp = queue_alloc( 10, sizeof( token_t ) );
  while ( dequeue( tStream, (void *)&token ) && (token.tok != endTok) )  {
    if  ( token.tok == cTok )
      count++;
    enqueue( temp, (void *)&token );
  }
  
  if  (token.tok == endTok)
    enqueue( temp, (void *)&token );

  queue_append( temp, &tStream );
  queue_set_granularity( temp, 0 );
  tStream = temp;

  return count;
}


/*	UNPARSE -  Print out the contents of the parse table.  Useful for
	debugging, but not used in general.
*/

void unparse  ( void )
{
  int  i,j;
  void *index;

  printf ("Parse Tree\n");
  printf ("  Nseries:   %d\n", parseTree->Nseries );
  printf ("  Nmappings: %d\n", parseTree->Nmappings );
  printf ("  Npts:      %d\n", parseTree->Npts );

  for  ( i = 0 ; i < parseTree->Nseries ; i++ )  {
    printf ("  series[%d]: %s",i+1,ttoa( parseTree->series[i].type ));
    for  ( j = 0 ; j < parseTree->series[i].Nenum ; j++ )
      printf ("  %s",parseTree->series[i].enums[j]);
    printf ("\n");
  }
  
  index = parseTree->mappings;
  while  ( index != NULL )  {
    printf ("\n  Mapping: %s\n", ((map_t *)index)->name);
    printf ("    Index: %s\n", ((map_t *)index)->index);
    printf ("    Start-  Label: %s Offset: %d\n", 
	    ((map_t *)index)->start.label, ((map_t *)index)->start.offset );
    printf ("    Stop-   Label: %s  Offset: %d\n",
	    ((map_t *)index)->stop.label, ((map_t *)index)->stop.offset );
    printf ("    Step: %d\n", ((map_t *)index)->step );
    printf ("    Ninputs: %d  Noutputs: %d\n", ((map_t *)index)->Ninputs,
	    ((map_t *)index)->Noutputs );
    printf ("    Mapping:  ");
    for  ( i = 0 ; i < ((map_t *)index)->Ninputs ; i++ )
      printf ("[%d,%d] ", ((map_t *)index)->input[i][0],
	      ((map_t *)index)->input[i][1] );
    printf ("=> ");
    for  ( i = 0 ; i < ((map_t *)index)->Noutputs ; i++ )
      printf ("[%d,%d] ", ((map_t *)index)->output[i][0],
	      ((map_t *)index)->output[i][1] );
    printf ("\n\n");
    index = ((map_t *)index)->next;
  }

  printf ("\n\n  Labels\n");
  index = parseTree->labels;
  while  ( index != NULL )  {
    printf  ("    [%s]: %d\n", ((lbl_t *)index)->label, 
	     ((lbl_t *)index)->location );
    index = ((lbl_t *)index)->next;
  }
  
  printf ("\n\n  Data\n");
  for  ( i = 0 ; i < parseTree->Npts ; i++ )  {
    if  ( parseTree->data.endSeg[i] )
      printf  ("[reset]  ");
    else
      printf  ("         ");
    for  ( j = 0 ; j < parseTree->Nseries ; j++ )
      printf  ("%s  ", ttoa( parseTree->data.series[j][i].tok ) );
    printf ("\n");
  }
}


/***************************** Interpreter ***********************************/


/*	INTERP -  Main dispatch function for the interpreter.  This function
	builds the final data structure that is returned to the calling
	program.
*/

data_file_t *interp  ( float binPos, float binNeg, 
		       char *filename )
{
  data_file_t *dSet;
  float       *means;
  char        *fn = "Interpret Data File";
  int         i;

  fprintf (stderr,".....$Interpreting$...");
  fflush  (stderr);

  /*  Allocate memory for and initialize the new data set.  */

  dFilePtr = dSet = init_dataset ( filename, binPos, binNeg );
  
  /*  Calculate the mean values of continuous series.  Calculate the  */
  /* floating point equivelances for binary and enumerated series     */

  means = get_means  ( parseTree->Nseries, parseTree->series, parseTree->data);
  setup_equivs       ( parseTree, dSet, means );

  /*  Allocate memory for the data sets and then compute their contents in  */
  /* the order the mappings occur in the list.                              */

  dSet->dataSets = (data_set_t *)alloc_mem( parseTree->Nmappings,
 					    sizeof( data_set_t ), fn );
  dSet->NdataSets = parseTree->Nmappings;
  for  ( i = 0 ; i < parseTree->Nmappings ; i++ )  {
    dSet->dataSets[i] = gen_dataset( dSet, parseTree->mappings, 
				     parseTree->data, parseTree->labels );
    parseTree->mappings = parseTree->mappings->next;

    /*  If this data set has one of the magic names (train, validate, test,  */
    /* predict), link it to the appropriate pointer.                         */

    if  ( !strcasecmp( dSet->dataSets[i].name, "train" ) )
      dSet->train = (dSet->dataSets)+i;
    else if  ( !strcasecmp( dSet->dataSets[i].name, "validate" ) )
      dSet->validate = (dSet->dataSets)+i;
    else if  ( !strcasecmp( dSet->dataSets[i].name, "test" ) )
      dSet->test     = (dSet->dataSets)+i;
    else if  ( !strcasecmp( dSet->dataSets[i].name, "predict" ) )
      dSet->predict  = (dSet->dataSets)+i;
  }

  /*  Deallocate memory for the parse tree  */

  //freePTree( parseTree );
  parseTree = NULL;

  fprintf (stderr,"Done!\n");
  fflush  (stderr);

  return dSet;
}


/*	INIT DATASET -  Allocate memory for a new data set and assign its
	fields reasonable default values.
*/

data_file_t *init_dataset  ( char *filename, float binPos, float binNeg )
{
  data_file_t *dSet;
  
  dSet = (data_file_t *)alloc_mem (1,sizeof( data_file_t ), "Init data set" );
  
  dSet->filename = strdup( filename );
  dSet->binPos   = binPos;
  dSet->binNeg   = binNeg;
  
  dSet->Ninputs    = 0;
  dSet->Noutputs   = 0;
  dSet->NinNodes   = 0;
  dSet->NoutNodes  = 0;
  
  dSet->inputMap   = NULL;
  dSet->outputMap  = NULL;
  dSet->dataSets   = NULL;
  dSet->train      = NULL;
  dSet->validate   = NULL;
  dSet->test       = NULL;
  dSet->predict    = NULL;

  return dSet;
}


/*	GET MEANS -  For each series, if it is of type CONT, calculate the
	arithmatic mean of its elements.  Non continuous series are awarded the
	mean value of 0.0.
*/

float *get_means  ( int Nseries, ser_t *series, dvect_t data )
{
  float *means;
  int   i,j;

  means = (float *)alloc_mem(Nseries, sizeof( float ), "Get Means" );
  for  ( i = 0 ; i < Nseries ; i++ )  {
    if  ( series[i].type == contT )
      for  ( j = 0 ; j < parseTree->Npts ; j++ )
	means[i] += (parseTree->data.series[i][j].tok == numT) ? 
	  parseTree->data.series[i][j].numVal : 0.0;
    means[i] /= (float)(parseTree->Npts)-2;
  }

  return means;
}



/*	SETUP EQUIVS -  Get equivelances for the input and output vectors.
	Also, designate a type (CONT, BINARY) for each output UNIT.
*/

void  setup_equivs  ( parse_tree_t *pTree, data_file_t *dSet, float *means )
{
  dSet->Ninputs  = pTree->mappings->Ninputs;
  dSet->Noutputs = pTree->mappings->Noutputs;

  get_equivs ( pTree->mappings->input, pTree->series, means,
	       dSet->binPos, dSet->binNeg, dSet->Ninputs, &(dSet->NinNodes),
	       &(dSet->inputMap) );
  get_equivs ( pTree->mappings->output, pTree->series, means+dSet->Ninputs,
	       dSet->binPos, dSet->binNeg, dSet->Noutputs, 
	       &(dSet->NoutNodes), &(dSet->outputMap) );
  get_types  ( dSet->NoutNodes, dSet->Noutputs,
	       dSet->outputMap, &(dSet->outputType) );
}


/*	GET EQUIVS -  Build an equivelance map for the input/output mapping
	specified.  We also count the number of nodes/units used for each
	element.  Continuous elements have their unknown vector set to a single
	element that contains the mean value for that series.
*/

void  get_equivs  ( int **mapping, ser_t *series, float *means, float binPos,
		    float binNeg, int Nitems, int *Nunits, cvrt_t **cvrtMap )
{
  ser_t ser;
  int   i, 
        totNodes = 0;
  char  *fn = "Get equivelances";

  *cvrtMap = (cvrt_t *)alloc_mem ( Nitems, sizeof( cvrt_t ), fn );
  for  ( i = 0 ; i < Nitems ; i++ )  {
    ser = series[ mapping[i][0] ];
    switch  ( ser.type )  {
      case contT:      (*cvrtMap)[i].Nunits = 1;
	               (*cvrtMap)[i].Nenums = 0;
	               (*cvrtMap)[i].enums  = NULL;
	               (*cvrtMap)[i].equivs = NULL;
	               (*cvrtMap)[i].unknown = (float *)alloc_mem( 1,
							sizeof(float), fn );
	               (*cvrtMap)[i].unknown[0] = means[i];
	               break;
      case binaryT:    (*cvrtMap)[i] = bin_equiv( binPos, binNeg );
	               break;
      case enumT:      (*cvrtMap)[i] = enum_equiv(ser, binPos, binNeg, FALSE);
	               break;
      case binEnumT:   (*cvrtMap)[i] = enum_equiv( ser, binPos, binNeg, TRUE );
	               break;
      case specEnumT:  (*cvrtMap)[i] = senum_equiv( ser );
	               break;
      };
    totNodes += (*cvrtMap)[i].Nunits;
  }

  *Nunits = totNodes;
}


/*	BIN EQUIV -  This equivelance is hard wired into the program.  A
	"+" is mapped to binPos and a "-" is mapped to binNeg.  The unknown
	value is set to the average of binPos and binNeg.
*/

cvrt_t bin_equiv  ( float binPos, float binNeg )
{
  char   *fn = "Binary equivelance";
  cvrt_t temp;

  temp.Nunits   = 1;
  temp.Nenums   = 2;

  temp.enums    = (char **) alloc_mem( 2, sizeof( char * ), fn );
  temp.enums[0] = (char *)  alloc_mem( 2, sizeof( char ), fn );
  temp.enums[1] = (char *)  alloc_mem( 2, sizeof( char ), fn );
  strcpy( temp.enums[0], "+" );
  strcpy( temp.enums[1], "-" );

  temp.equivs    = (float **) alloc_mem( 2, sizeof( float * ), fn );
  temp.equivs[0] = (float *)  alloc_mem( 1, sizeof( float ), fn );
  temp.equivs[1] = (float *)  alloc_mem( 1, sizeof( float ), fn );
  temp.equivs[0][0] = binPos;
  temp.equivs[1][0] = binNeg;

  temp.unknown = (float *)alloc_mem( 1, sizeof( float ), fn );
  temp.unknown[0] = (binPos+binNeg)/2;

  return temp;
}


/*	ENUM EQUIV -  Establish equivelance mappings for enums and binenums.
	First, the identifiers are copied over into the conversion map.  At
	the same time, a floating point array is established for each
	enumeration.  Each value in this array is either binPos or binNeg.
	For enums, this array will be the unary representation of the
	enumeration number.  For binenums, this representation will be binary.
*/

cvrt_t enum_equiv  ( ser_t decl, float binPos, float binNeg, boolean binary )
{
  char   *fn = "Enumerated equivelances";
  cvrt_t temp;
  int    num,
         i, j;

  temp.Nenums = decl.Nenum;
  temp.Nunits = ( binary ) ? num_bin( temp.Nenums ) : temp.Nenums;

  temp.enums  = (char **)alloc_mem( temp.Nenums, sizeof( char * ), fn );
  temp.equivs = (float **)alloc_mem( temp.Nenums, sizeof( float * ), fn );
  temp.unknown = (float *)alloc_mem( temp.Nunits, sizeof( float ), fn );
 
  for  ( i = 0 ; i < temp.Nenums ; i++ )  {
    temp.enums[i] = (char *)alloc_mem(strlen(decl.enums[i])+1,sizeof(char),fn);
    temp.equivs[i] = (float *)alloc_mem( temp.Nunits, sizeof( float ), fn );
    strcpy( temp.enums[i], decl.enums[i] );

    /*  Establish the equivelance pattern.  */

    num = i;
    for  ( j = 0 ; j < temp.Nunits ; j++ )
      if  ( binary )  {
	temp.equivs[i][j] = ( num & 1 ) ? binPos : binNeg;
	num >>= 1;
      } else
	temp.equivs[i][j] = ( num == j ) ? binPos : binNeg;
  }

  /*  The unknown value is an array of floating point numbers, each set to  */
  /* the average between binPos and binNeg.                                 */

  for  ( i = 0 ; i < temp.Nunits ; i++ )
    temp.unknown[i] = (binPos+binNeg)/2;

  return temp;
}


/*	NUM BIN -  Return the argument taken to log base two and cast to an
	integer.
*/

int num_bin  ( int num )
{
  int retVal = 0;
  
  num--;
  while ( num > 0 )  {
    num >>= 1;
    retVal++;
  }
  
  return retVal;
}


/*	SPECIFIED ENUMERATION EQUIVELANCES -  Create a conversion map for the
	declarations read in during parsing.  Notice that the actual mappings
	have already been computed by this point.
*/
 
cvrt_t senum_equiv  ( ser_t decl )
{
  char   *fn = "Specified enumerated equivelances";
  cvrt_t temp;
  int    num,
         i, j;

  temp.Nenums = decl.Nenum;
  temp.Nunits = decl.Nnodes;

  temp.enums  = (char **)alloc_mem( temp.Nenums, sizeof( char * ), fn );
  temp.equivs = (float **)alloc_mem( temp.Nenums, sizeof( float * ), fn );
  temp.unknown = (float *)alloc_mem( temp.Nunits, sizeof( float ), fn );
 
  for  ( i = 0 ; i < temp.Nenums ; i++ )  {
    temp.enums[i] = (char *)alloc_mem(strlen(decl.enums[i])+1,sizeof(char),fn);
    temp.equivs[i] = (float *)alloc_mem( temp.Nunits, sizeof( float ), fn );
    strcpy( temp.enums[i], decl.enums[i] );

    /*  Establish the equivelance pattern.  */

    for  ( j = 0 ; j < temp.Nunits ; j++ )
      temp.equivs[i][j] = decl.vals[i][j];
  }

  /*  The unknown value is an array of floating point numbers, each set to  */
  /* the average between binPos and binNeg.                                 */

  for  ( i = 0 ; i < temp.Nunits ; i++ )
    temp.unknown[i] = (binaryPos+binaryNeg)/2;

  return temp;
}


/*	GET TYPES -  Cycle through each output unit and assign it a type.
	Units associated with continuous series are of type CONT, all others
	are type BINARY.
*/

void get_types  ( int Nnodes, int Nio, cvrt_t *map, out_t **types )
{
  int i,j,k=0;

  *types = (out_t *)alloc_mem(Nnodes,sizeof( out_t ), "Get Types");

  for  ( i = 0 ; i < Nio ; i++ )
    for  ( j = 0 ; j < map[i].Nunits ; j++ )
      (*types)[k++] = ( map[i].Nenums == 0 ) ? CONT : BINARY;
}


/*	GEN DATASET -  This is the heart of the interpreter.  We allocate
	memory for the data set based on the guess that the number of elements
	will equal the distance between the begin and end of the mapping,
	divided by the step size.

	We cycle through the elements of the mapping with two pointers.  The
	first is the main index pointer, referred to as 'data'.  The other
	pointer is set to |end offset| ahead of the data pointer.  This pointer
	detects sequence markers and the end of data before we actually go too
	far.  If end offset is zero, this pointer points to the same node as
	data.

	Whenever data points to a segment marker, we jump ahead by begin
	offset + 1 (to get off of the marker).  If lookAhead points to a 
	segment marker, we jump ahead by |end offset|.  This, of course,
	will immediately followed by the data pointing to a segment marker, and
	so we jump ahead again.  This continues until lookAhead no longer
	points to a segment marker.
*/

data_set_t  gen_dataset  ( data_file_t *dFile, map_t *map, dvect_t data,
			   lbl_t *labels )
{
  data_set_t dSet;
  cvrt_t     *cvrtMap;
  int        begin, bOff,
             end, eOff,
             index, step,
             len, Nio,
             point, node,
             i,j,k;
  token_t    **tptrs;
  float      *temp;
  char       *fn = "Generate Data Set";

  /*  Find the begin and end of the data, plus their offsets  */

  bOff  = map->start.offset;
  eOff  = map->stop.offset;
  begin = lookup_label( map->start.label, labels ) + bOff;
  end   = lookup_label( map->stop.label, labels ) + eOff;
  
  Nio = map->Ninputs+map->Noutputs;
  tptrs = (token_t **)alloc_mem( Nio, sizeof( token_t * ), fn );
  for ( i = 0 ; i < Nio ; i++ )
    if  ( i < map->Ninputs )
      tptrs[i] = parseTree->data.series[map->input[i][0]];
    else
      tptrs[i] = parseTree->data.series[map->output[i-map->Ninputs][0]];

  /*  Take an initial guess at the size of this data set, and begin  */
  /* initializing some fields of the data record.                    */

  len = (int) ceil( ((end - begin) / (map->step)) ) + 1;
  dSet.name = (char *) alloc_mem( strlen( map->name )+1, sizeof( char ), fn );
  dSet.data = (dv_t *) alloc_mem( len, sizeof( dv_t ), fn );
  strcpy( dSet.name, map->name );
  dSet.Npts        = 0;
  dSet.predictOnly = (map->Noutputs == 0);

  step  = map->step;
  point = 0;
  for  ( index = begin ; index < end ; index += step, point++ )  {
    dSet.data[point].reset = data.endSeg[index];
    
    dSet.data[point].inputs = (float *)alloc_mem( dFile->NinNodes, 
						 sizeof( float ), fn );
    node = 0;
    for  ( i = 0 ; i < map->Ninputs ; i++ )  {
      cvrtMap = (dFile->inputMap)+i;
      j = 0;
      while ( j != map->input[i][1] )  {
	if  ( (tptrs[i]+index+j)->tok == blockT )
	  break;
	j += ( j < map->input[i][1] ) ? 1 : -1;
      }
      if  ( cvrtMap->Nenums == 0 )
	dSet.data[point].inputs[node++] = get_cont_data ( &(tptrs[i][index+j]),
				 			 cvrtMap->unknown[0] );
      else  {
	temp = get_enum_data ( &(tptrs[i][index+j]), cvrtMap );
	for  ( k = 0 ; k < cvrtMap->Nunits ; k++ )
	  dSet.data[point].inputs[node++] = temp[k];
      }
    }

    dSet.data[point].outputs = (float *)alloc_mem( dFile->NoutNodes, 
			 			  sizeof( float ), fn );
    node = 0;
    for  ( i = 0 ; i < map->Noutputs ; i++ )  {
      cvrtMap = (dFile->outputMap)+i;
      j = 0;
      while ( j != map->output[i][1] )  {
	if  ( (tptrs[i+dFile->Ninputs]+index+j)->tok == blockT )
	  break;
	j += ( j < map->output[i][1] ) ? 1 : -1;
      }
      if  ( cvrtMap->Nenums == 0 )
	dSet.data[point].outputs[node++] = get_cont_data 
	  (&(tptrs[map->Ninputs+i][index+j]), cvrtMap->unknown[0] );
      else  {
	temp = get_enum_data ( &(tptrs[i+dFile->Ninputs][index+j]), cvrtMap );
	for  ( k = 0 ; k < cvrtMap->Nunits ; k++ )
	  dSet.data[point].outputs[node++] = temp[k];
      }
    }
  }

  dSet.Npts = point;

  /*  Deallocate memory that was not used due to segment markers in the data */

  dSet.data = (dv_t *)realloc_mem( dSet.data, dSet.Npts, sizeof( dv_t ), fn );

  /*  Calculate the standard deviation for data sets that have outputs  */

  dSet.stdDev = (dSet.predictOnly) ? 0.0 : calc_std_dev(dSet.data, 
							dFile->NoutNodes,
							dSet.Npts);

  return dSet;
}


/*	GET CONTINUOUS DATA -  Given a token, return the continuous data from
	that token, or the default value if the token contains an unknown
	value.
*/

float get_cont_data  ( token_t *token, float def )
{
  char errMess[EMLEN];

  if  ( (token->tok == questionT) || (token->tok == blockT) )
    return def;

  if  ( token->tok != numT )  {
    sprintf ( errMess, "Illegal token '%s'.\n", ttoa( token->tok ) );
    parse_err ( token->line, errMess );
  }

  return token->numVal;
}


/*	GET ENUMERATED DATA -  Lookup a token in its conversion map and then
	return the associated mapping.  If the token contains a block or
	unknown, return the unknown value.
*/

float *get_enum_data ( token_t *token, cvrt_t *map )
{
  char errMess [EMLEN];

  if  ( (token->tok == blockT) || (token->tok == questionT) )
    return map->unknown;

  if  ( token->tok != idT )  {
    sprintf ( errMess, "Illegal token '%s'.\n", ttoa( token->tok ) );
    parse_err( token->line, errMess );
  }

  return lookup_ident( token->idVal, map, token->line );
}


/*	LOOKUP LABEL -  Lookup a character string in the label table and
	return that label's location in the data file.  Also, if ptr is not
	NULL, set it equal to the point in the data series where the label
	occurred.  If the label is not found, gripe about it.
*/

int lookup_label ( char *lbl, lbl_t *labels )
{
  char errMess [EMLEN];

  if  ( lbl == NULL )
    return 0;
  while  ( labels != NULL )  {
    if  ( !strcmp( lbl, labels->label ) )  {
      return labels->location;
    }
    labels = labels->next;
  }

  sprintf   ( errMess, "Label '%s' not found", lbl );
  parse_err ( -1, errMess );
  return NOT_FOUND;
}


/*	LOOKUP IDENT -  Lookup an identifier in mapping table.  Complain if
	you can't find it.  The search is linear.
*/

float *lookup_ident  ( char *ident, cvrt_t *cMap, int line )
{
  char errMess [EMLEN];
  int  i;

  for  ( i = 0 ; i < cMap->Nenums ; i++ )
    if  ( !strcmp( ident, cMap->enums[i] ) )
      return cMap->equivs[i];

  sprintf   ( errMess, "Unknown identifier '%s'", ident );
  parse_err ( line, errMess );
  return NULL;
}


/*	CALC STD DEV -  Calculate the standard deviation for the outputs of a
	data set and return that value.
*/

float  calc_std_dev  ( dv_t *data, int NoutNodes, int Npts )
{
  float cur,		/*  Current output value  */
        sum,		/*  Sum of output values  */
        sumSq;		/*  Sum of the squared output values  */
  int   i, j;		/*  Indexing variables  */

  sum 		= 0.0;
  sumSq 	= 0.0;

  for  ( i = 0 ; i < Npts ; i++ )
    for  ( j = 0 ; j < NoutNodes ; j++ )  {	/*  Compute values for this  */
      cur   =  data[i].outputs[j];	        /* output		     */
      sum   += cur;
      sumSq += cur * cur;
    } 

  /*  Return the standard deviation of this data set  */

  return  ( sqrt( (Npts * sumSq - sum * sum) / (Npts * (Npts - 1.0)) ) );
}


/*	UNINTERP -  Prints out the interpretted results.  Not generally used
	except for debugging purposes.
*/

void  uninterp  ( data_file_t *dFile )
{
  int i;

  printf  ("File:      %s\n", dFile->filename );
  printf  ("Ninputs:   %d  Noutputs:  %d\n", dFile->Ninputs, dFile->Noutputs );
  printf  ("NinNodes:  %d  NoutNodes: %d\n", dFile->NinNodes,dFile->NoutNodes);
  printf  ("binPos:    %4.2f  binNeg:    %4.2f\n\n", dFile->binPos, 
	   dFile->binNeg );

  printf  ("Output types:  ");
  for  ( i = 0 ; i < dFile->NoutNodes ; i++ )
    printf  ("%s  ", otoa( dFile->outputType[i] ) );
  printf  ("\n\n");

  printf  ("Input Map\n");
  for  ( i = 0 ; i < dFile->Ninputs ; i++ )
    print_map  ( dFile->inputMap[i] );
  printf  ("\n\n");

  printf  ("Output Map\n");
  for  ( i = 0 ; i < dFile->Noutputs ; i++ )
    print_map  ( dFile->outputMap[i] );
  printf  ("\n\n");

  printf  ("Data\n");
  for  ( i = 0 ; i < dFile->NdataSets ; i++ )
    print_data  ( dFile->dataSets[i], dFile->NinNodes, dFile->NoutNodes );
}

void  print_map  ( cvrt_t map )
{
  int i,j;

  printf  ("Nenums:  %d  Nunits:  %d\n", map.Nenums, map.Nunits );
  for  ( i = 0 ; i < map.Nenums ; i++ )  {
    printf ("%s:  ", map.enums[i] );
    for  ( j = 0 ; j < map.Nunits ; j++ )
      printf ("%6.4f ", map.equivs[i][j]);
    printf ("\n");
  }
  printf ("Unknown: ");
  if  ( map.Nenums == 0 )
    printf  ("%6.4f",map.unknown[0]);
  else
    for  ( i = 0 ; i < map.Nunits ; i++ )
      printf  ("%6.4f ",map.unknown[i]);
  printf ("\n\n");
}

void  print_data  ( data_set_t data, int Nin, int Nout )
{
  int i,j;

  printf  ("Name:  %s\n", data.name);
  printf  ("Npts:  %d  stdDev:  %f   predictOnly:  %d\n",
	   data.Npts, data.stdDev, data.predictOnly);
  for  ( i = 0 ; i < data.Npts ; i++ )  {
    if  ( data.data[i].reset )
      printf ("[reset] ");
    else
      printf ("        ");
    for  ( j = 0 ; j < Nin ; j++ )
      printf ("%6.4f ", data.data[i].inputs[j]);
    if  ( !data.predictOnly )  {
      printf ("=> ");
      for  ( j = 0 ; j < Nout ; j++ )
	printf ("%6.4f ", data.data[i].outputs[j]);
    }
    printf ("\n");
  }
}


/*********************************** General *********************************/


/*	PRINT BANNER -  Print a short welcome banner.
*/

void print_banner  ( void )
{
  fprintf (stderr, "CMU Learning Benchmark Archive\n");
  fprintf (stderr, "Data File Parser  v%s (%s)\n\n", VERSION, RELDATE);
}

/*	INIT PARSE -  Initialize the data parser by opening the specified data
	file and returning a pointer to that file.
*/
    
FILE *init_parse  ( char *filename )
{
  FILE *fptr;
  char err_mess[EMLEN];

  if ( (fptr = fopen (filename,"r")) == NULL )  {
    sprintf   (err_mess,"Unable to open data file '%s'",filename);
    parse_err (-1,err_mess);
  }

  fprintf (stderr,"Opening '%s'...",filename);
  fflush  (stderr);
 
  return fptr;
}


/*	PARSE ERR -  So we flamed out.  We need to print out an error message
	describing (hopefully) the problem with the data file.  Then we
	deallocate any memory that has been allocated, so as to avoid memory
	leaks.  Finally, we long jump back to the main routine so that we
	can exit gracefully.
*/
  
void  parse_err  ( int lineNum, char *error )
{
  if  ( lineNum <= 0 )
    fprintf ( stderr, "\nParse Error: %s\n", error );
  else
    fprintf  ( stderr, "\nParse Error (line %d): %s\n", lineNum, error );
  
  if  ( tStream != NULL )
    queue_free( &tStream );
  if  ( parseTree != NULL )
    freePTree( parseTree );
  if  ( dFilePtr != NULL )
    free_data( &dFilePtr );

  longjmp( error_trap, 1 );
}


/*	FREEPTREE -  Free the memory associated to a parse tree.  There are a
	number of small supporting functions that all free small parts of the
	parse tree.  These are uncommented and self-explanitory.
*/

void freePTree  ( parse_tree_t *tree )
{
  int i,j;

  if  ( tree->series != NULL )
    freeSer( tree->series, tree->Nseries );
  if  ( tree->mappings != NULL )
    freeMap( tree->mappings );
  if  ( tree->labels != NULL )
    freeLabels( tree->labels );

  if  ( parseTree->data.endSeg != NULL )
    free( parseTree->data.endSeg );
  if  ( parseTree->data.series != NULL )  {
    for  ( i = 0 ; i < parseTree->Nseries ; i++ )
      if  ( parseTree->data.series[i] != NULL )  {
	for  ( j = 0 ; j < parseTree->Npts ; j++ )
	  if  ( parseTree->data.series[i][j].idVal != NULL )
	    free( parseTree->data.series[i][j].idVal );
	free( parseTree->data.series[i] );
      }
    free( parseTree->data.series );
  }

  free( tree );
}


void freeSer  ( ser_t *series, int num )
{
  int i, j;

  if  ( series == NULL )
    return;

  for  ( i = 0 ; i < num ; i++ )
    if  ( series[i].enums != NULL )  {
      for  ( j = 0 ; j < series[i].Nenum ; j++ )
	if  ( series[i].enums[j] != NULL )
	  free( series[i].enums[j] );
      free( series[i].enums );
    }
  free( series );
}

void freeMap  ( map_t *map )
{
  map_t *temp;
  int   i;

  while  ( map != NULL )  {
    temp = map;
    map  = map->next;

    if  ( temp->name != NULL )
      free( temp->name );
    if  ( temp->index != NULL )
      free( temp->index );

    if  ( temp->input != NULL )  {
      for  ( i = 0 ; i < temp->Ninputs ; i++ )
	if  ( temp->input[i] != NULL )
	  free( temp->input[i] );
      free( temp->input );
    }
    if  ( temp->output != NULL )  {
      for  ( i = 0 ; i < temp->Noutputs ; i++ )
	if  ( temp->output[i] != NULL )
	  free( temp->output[i] );
      free( temp->output );
    }
    
    free( temp );
  }
}

void freeLabels  ( lbl_t *labels )
{
  lbl_t *temp;

  while  ( labels != NULL )  {
    temp   = labels;
    labels = labels->next;

    if  ( temp->label != NULL )
      free( temp->label );
    free( temp );
  }
}


/*	FREE CMAP, FREE DATA SET -  Supporting functions to the main FREE DATA
	described above.
*/

void  free_cmap  ( cvrt_t cmap )
{
  int i;
  
  for  ( i = 0 ; i < cmap.Nenums ; i++ )  {
    if  ( cmap.enums[i] != NULL )
      free( cmap.enums[i] );
    if  ( cmap.equivs[i] != NULL )
      free( cmap.equivs[i] );
  }
  if  ( cmap.enums != NULL )
    free( cmap.enums );
  if  ( cmap.equivs != NULL )
    free( cmap.equivs );
  if  ( cmap.unknown != NULL )
    free( cmap.unknown );
}

void  free_data_set  ( data_set_t dSet )
{
  int i;
  
  if  ( dSet.data != NULL )  {
    for  ( i = 0 ; i < dSet.Npts ; i++ )  {
      if  ( dSet.data[i].inputs != NULL )
	free( dSet.data[i].inputs );
      if  ( dSet.data[i].outputs != NULL )
	free( dSet.data[i].outputs );
    }
    free( dSet.data );
  }
  if  ( dSet.name != NULL )
    free( dSet.name );
}


/*	TTOA -  Token TO Ascii.  Takes a token and returns an ascii equivelant.
	Used mostly for constructing error messages.
*/

char *ttoa  ( tok_t token )
{
  switch  ( token )  {
    case dataT:       return "dataT";
    case setupT:      return "setupT";
    case LparenT:     return "LparenT";
    case RparenT:     return "RparenT";
    case commaT:      return "commaT";
    case rangeT:      return "rangeT";
    case colonT:      return "colonT";
    case semiColonT:  return "semiColonT";
    case segMarkT:    return "segMarkT";
    case blockT:      return "blockT";
    case equalT:      return "equalT";
    case arrowT:      return "arrowT";
    case questionT:   return "questionT";
    case LbracketT:   return "LbracketT";
    case RbracketT:   return "RbracketT";
    case binaryT:     return "binaryT";
    case binEnumT:    return "binEnumT";
    case contT:       return "contT";
    case enumT:       return "enumT";
    case forT:        return "forT";
    case NseriesT:    return "NseriesT";
    case seriesT:     return "seriesT";
    case specEnumT:   return "specEnumT";
    case stepT:       return "stepT";
    case toT:         return "toT";
    case LbraceT:     return "LbraceT";
    case RbraceT:     return "RbraceT";
    case eofT:        return "eofT";
    case idT:         return "idT";
    case numT:        return "numT";
    default:          return "Unknown token";
    }
}


/*	OTOA -  Output TO Ascii.  Takes an output type and returns an ascii
	string.  Used mainly in debugging.
*/

char *otoa  ( out_t val )
{
  switch  ( val )  {
    case CONT:   return "Cont";
    case BINARY: return "Binary";
    default:     return "Huh?";
    }
}
