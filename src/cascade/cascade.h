/*  CMU Cascade Neural Network Simulator (CNNS)
    Header information

    v1.0
    Matt White (mwhite+@cmu.edu)
    May 28, 1995

    CMU Site Contact:  neural-bench@cs.cmu.edu

    NOTE:  Please do not contact me directly.  All inquiries regarding this
    code should be directed to 'neural-bench@cs.cmu.edu'.

    This code has been placed in the public domain by its author.  As a matter
    of simple courtesy, any use of this code should aknowledge its author.
    The author would like to hear of any attempts to use this code, successful
    or not.

    This program implements the Cascade-Correlation [2], Recurrent 
    Cascade-Correlation [3], Cascade-2 [unpublished] and the Recurrent 
    Cascade-2 [unpublished] architectures.  All weight updates make use of
    the Quickprop [1] weight update algorithm.  Enjoy!

    References:
    [1] Scott Fahlman, "Faster Learning Variations on Back-Propagation:
        An Empirical Study",  1988.
    [2] Scott Fahlman and Christian Lebiere, "The Cascade-Correlation
        Learning Architecture",  1990.
    [3] Scott Fahlman, "The Recurrent Cascade-Correlation Architecture",  1991.

    This file contains the declarations for the Cascade Neural Network
    Simulator.  Add any new functions you define here.
*/

#ifndef CASCADE
#define CASCADE

#include <time.h>
#include <stdio.h>

#include "toolkit.h"
#include "parse.h"

#ifndef VER                                /*  Program version  */
#define VER "1.0"
#endif  

#ifndef CONTACT                            /*  CMU Site contact  */
#define CONTACT "neural-bench@cs.cmu.edu"
#endif

#ifndef PROMPT                             /*  CLI command line prompt  */
#define PROMPT "cli>"
#endif

#ifndef MAX_INPUT                          /*  Maximum length input line  */
#define MAX_INPUT 41
#endif

#ifndef NO_CONNX                           /*  Do we use connection crossing */
#define CONNX                              /* statistics or not?             */
#endif

#define DEF_SIGMAX 0.5                     /*  Set some defaults  */
#define DEF_SIGMIN -0.5
#define BIAS       1.0

/*  Macro to determine error index  */
#define ERROR_INDEX( SSDiff, sDev, num )  ( sqrt( SSDiff / num ) / sDev )

/*  Node types  */
typedef enum {
  UNDEFINED,
  VARIED,
  SIGMOID,
  ASIGMOID,
  VARSIGMOID,
  GAUSSIAN,
  LINEAR
  } node_t;

/*  Training algorithms  */
typedef enum {
  CASCOR,
  CASCADE2
  } algo_t;

/*  Method of determining network error  */
typedef enum {
  INDEX,
  BITS
  } error_t;

/*  Training statuses  */
typedef enum {
  TRAINING,
  TIMEOUT,
  STAGNANT,
  WIN,
  LOSS
  } status_t;


/*  ERROR_DATA_T
    Contains error information on network performance  */
typedef struct {
  int   bits;          /*  Number of incorrect bits  */
  float index,         /*  Computed error index  */
        sumSqDiffs,    /*  The Sum of the Square Differences  */
        sumSqError,    /*  The Sum of the Square Errors (using EPrime)  */
        *errors,       /*  The error at each output  */
        *tempErrors,   /*  Temporary error vector in case of no cache  */
        *sumErr;       /*  The sum of the error at each output  */
} error_data_t;


/*  LAYER_INFO_T
    Contains training data for a single layer of the network.  Instances are
    constructed for the output, candidate input and candidate output layers. */
typedef struct {
  float shrinkFactor,  /*  This is related to mu.  See [1]                   */
        **weights,     /*  Only used for candidate layers                    */
        **deltas,      /*  The previous weight changes                       */
        **slopes,      /*  The slope of the error function at this point     */
        **pSlopes;     /*  The previous value of the slope at this point     */
} layer_info_t;


/*  TRAIN_DATA_T
    Transient network data.  This information is used for training the network
    but is not otherwise necessary for prediction.  This structure is
    generally built as training is about to begin.                           */
typedef struct {
  int          candBest,        /*  The candidate with the best score        */
               cachePts;        /*  The number of points in the cache        */
  float        outScaledEps,    /*  The scaled value of the output epsilon   */
               candBestScore,   /*  The score of the best unit               */
               *candScores,     /*  The scores of the candidate units        */
               *candValues,     /*  The activation values of the candidates  */
               *candSumVals,    /*  The sum of the activation values         */
               **candCorr,      /*  The candidates' covariance               */
               **candPrevCorr,  /*  The previous values of the covariance    */
               *candPrevValues, /*  RCC.  The previous candidate activations */
               **candDVdW,      /*  RCC.  Derivitive of the value with       */
                                /* respect to the weight.                    */
               **valCache,      /*  Cached activation values.  Speeds up     */
                                /* training considerably                     */
               **errCache;      /*  Cached error values.                     */
  node_t       *candTypes;      /*  The activation types of each candidate   */
  layer_info_t candIn,          /*  Training information on the inputs to    */
                                /* the candidates                            */
               candOut,         /*  Training information on the outputs from */
                                /* the candidate units                       */
               output;          /*  Training information for the network     */
                                /* outputs                                   */
} train_data_t;


/*  UPDATE_PARMS_T
    These are parameters that are associated with the weight updates of a
    specific layer.  Accordingly, instances of this structure exist for the
    output, candidate in and candidate out layers.                           */
typedef struct {
  float epsilon,   /*  Learning rate parameter.  Higher rates can decrease   */
                   /* training time, but may cause learning to go unstable   */
        mu,        /*  Maximum step size parameter as described by Fahlman   */
                   /* in the Quickprop paper [1].  Usually not worth tuning  */
        decay;     /*  Weight decay.  Causes weights to decay towards zero.  */
                   /* If you get monstrous weights, set this to ~0.0001 or   */
                   /* less (it doesn't take much).                           */
} update_parms_t;


/*  CYCLE_PARMS_T
    These parameters are specific to a specific part of the training cycle.
    Therefore, one of these structures exist for both the output training
    and candidate training phases.                                           */
typedef struct {
  int   epochs,          /*  The number of training epochs to perform in     */
                         /* each phase before a TIMEOUT is declared.  In     */
                         /* general, a TIMEOUT should never be declared.     */
        patience;        /*  The number of epochs without significant change */
                         /* before the training is declared STAGNANT         */
  float changeThreshold; /*  The relative size of change required to be      */
                         /* considered 'significant'                         */
} cycle_parms_t;


/*  TRAIN_PARM_T
    This is the main structure used to contain training parameters.  All that
    is necessary for network training is contained herein.                   */
typedef struct  {
  int            maxNewUnits,        /*  The maximum number of units to add  */
                                     /* to the network being trained         */
                 validationPatience, /*  The number of training cycles to    */
                                     /* perform without improvement in       */
                                     /* cross-validation generalization      */
                 Ncand;              /*  Number of candidates in the         */
                                     /* training pool                        */
  float          outPrimeOffset,     /*  Amount to offset the error prime    */
                                     /* when training outputs.  See [1]      */
                                     /* for details of why this helps        */
                 weightRange,        /*  The maximum variance of random      */
                                     /* weights from zero                    */
                 indexThreshold,     /*  The maximum Error Index that is     */
                                     /* considered a victory                 */
                 scoreThreshold,     /*  The maximum fractional variance of  */
                                     /* a unit from its goal to be           */
                                     /* considered correct                   */
                 sigMax,             /*  Maximum value of VARSIGMOID units   */
                 sigMin;             /*  Minimum value of VARSIGMOID units   */
  boolean        overshootOK,        /*  Ok to overshoot the desired goal?   */
                 useCache,           /*  Is value and error cache in use?    */
                 test,               /*  Test the network after training?    */
                 validate,           /*  Cross-validate the network during   */
                                     /* training?                            */
                 recurrent;          /*  Train a recurrent network?          */
  node_t         candType;           /*  Type of candidate to comprise pool  */
  algo_t         algorithm;          /*  Network architecture to use         */
  error_t        errorMeasure;       /*  Measure that determines success     */
  update_parms_t candInUpdate,       /*  Parameters for candidates inputs    */
                 candOutUpdate,      /*  Parameters for candidates outputs   */
                 outputUpdate;       /*  Parameters for network outputs      */
  cycle_parms_t  candidateParm,      /*  Candidate phase parameters          */
                 outputParm;         /*  Output phase parameters             */
}  train_parm_t;


/*  NET_T
    This is the network data structure.  This structure contains all necessary
    information about the network.  Using the information contained within this
    structure, feedforward prediction is possible.                           */
typedef struct net_type {
  char            *name,          /*  Name of the network                    */
                  *filename,      /*  Filename the network is stored in      */
                  **descript;     /*  Description of the network. Not used.  */
  int             epochsTrained,  /*  Total number of epochs this network    */
                                  /* has been trained                        */
                  Nunits,         /*  Number of units in the network.  This  */
                                  /* includes inputs, hidden units and the   */
                                  /* bias unit, but not the outputs          */
                  Ninputs,        /*  Number of input units to the network   */
                  Noutputs,       /*  Number of ouputs from the network      */
                  NhiddenUnits,   /*  Current number of hidden units         */
                  maxNewUnits;    /*  Maximum number of hidden units that    */
                                  /* have yet to be added                    */
  float           *values,        /*  Unit activation values                 */
                  *tempValues,    /*  Temp float vector to be used when the  */
                                  /* cache is not in use                     */
                  **weights,      /*  Interior weights.  Weights to outputs  */
                                  /* not included.                           */
                  *outValues,     /*  Activation levels of the outputs       */
                  **outWeights,   /*  Weights to the outputs                 */
                  sigmoidMax,     /*  Maximum value of a VARSIGMOID          */
                  sigmoidMin;     /*  Minimum value of a VARSIGMOID          */
  boolean         recurrent;      /*  Is this net recurrent?                 */
  cvrt_t          *inputMap,      /*  Map from tokens to raw inputs          */
                  *outputMap;     /*  Maps from raw outputs to tokens        */
  node_t          *unitTypes,     /*  Types for the interior units           */
                  *outputTypes;   /*  Types of the outputs                   */
  struct net_type *next;
} net_t;


/*  DF_T 
    This structure is simply a container for a data file, so that the parsed
    file can be linked into a linked list.                                   */
typedef struct df_type {
  data_file_t    *data;
  struct df_type *next;
} df_t;


/*  PARM_VAR_T
    These are enumerations of the various types of data stored in the table
    located in 'interface.c'.  These enumerations help the data be interpreted
    correctly.                                                               */
typedef enum { INT,      /*  Integer value                                   */
	       FLOAT,    /*  Floating point value                            */
	       BOOLEAN,  /*  Boolean value                                   */
	       NODE,     /*  Node type (i.e. Sigmoid, Gaussian, etc.)        */
	       ALGO,     /*  Algorithm type (Cascor/Cascade-2)               */
	       ERR,      /*  Error type (Bits/Index)                         */
	       FUNC      /*  A function's address                            */
	     } parm_var_t;


/*  PARM_T
    This is an entry in the parameter table located in 'interface.c'.        */
typedef struct {
  char       *name;      /*  Display name of the parameter                   */
  parm_var_t type;       /*  Type of parameter (see above)                   */
  void       *ptr;       /*  Pointer to the location where the paramter is   */
                         /* stored                                           */
  boolean    modWRun;    /*  Safe to modify this parameter during a run?     */
} parm_t;


/*  TRIAL_RESULT_T
    This stores information on the results of training the network.          */
typedef struct {
  status_t endStatus;    /*  Ending status from training                     */
  int      bits,         /*  Number of error bits during last epoch          */
           Nepochs,      /*  Number of epochs we trained this time through   */
           connx,        /*  Number of connection crossings                  */
           time,         /*  Training time                                   */
           Nvictories,   /*  Number of victories achieved                    */
           Nunits;       /*  Number of units in the network                  */
  float    perCorrect,   /*  Percent of training outputs correct             */
           index,        /*  Error index after last epoch                    */
           sumSqDiffs,   /*  Sum of the Square of the Differences            */
           sumSqError;   /*  Sum of the Square of the Errors                 */
} trial_result_t;


/*  cascade.c  */

trial_result_t train_net          ( net_t *, train_parm_t *, data_file_t *,
				    int );
trial_result_t test_net           ( net_t *, data_set_t * );
void           set_globals        ( net_t *, train_parm_t *, train_data_t *,
				    data_file_t *, error_data_t * );
status_t       train_outputs      ( void );
void           output_epoch       ( void );
status_t       validation_epoch   ( float *, float ***, int *, int *, 
				    boolean );
void           adjust_weights     ( void );
void           adjust_ci_weights  ( void );
void           adjust_co_weights  ( void );
void           install_cand       ( int, boolean );

/*  cascor.c  */

status_t     cascor_train_cand           ( void );
void         cascor_correlation_epoch    ( void );
void         cascor_compute_correlations ( boolean );
void         cascor_cand_epoch           ( void );
void         cascor_compute_slopes       ( boolean );
void         cascor_adjust_correlations  ( void );

/*  cascade2.c  */

status_t     c2_train_cand               ( void );
void         c2_cand_epoch               ( void );
void         c2_compute_slopes           ( float *, boolean );
void         c2_find_best_cand           ( void );

/*  util.c  */

void         forward_pass       ( float *, boolean );
void         compute_outputs    ( void );
void         compute_error      ( float *, boolean, boolean, boolean, float );
void         quickprop          ( float *, float *, float *, float *,
			          float, float, float, float );
float        activation         ( node_t, float );
float        activation_prime   ( node_t, float, float );
float        output_prime       ( node_t, float );
float        random_weight      ( float );
void         sync               ( net_t *, data_file_t * );

net_t        *select_net        ( char * );
void         add_net            ( net_t * );

data_file_t  *select_data       ( char * );
void         add_data_file      ( data_file_t * );

char         *ntoa              ( node_t );
char         *altoa             ( algo_t );
char         *etoa              ( error_t );
char         *stoa              ( status_t );

node_t       aton               ( char * );
algo_t       atoal              ( char * );
error_t      atoe               ( char * );

/*  init.c  */

net_t        *build_net         ( char *, int, int, int, float, float, float,
				  boolean );
void         realloc_net        ( net_t *, int );
void         init_net           ( net_t *, float );
train_parm_t *build_parm        ( void );
train_data_t *build_train_data  ( net_t *, train_parm_t *, int );
void         free_train_data    ( train_data_t **, net_t *, train_parm_t * );
void         init_cand          ( train_data_t *, int, int, int, boolean,
				  float, node_t );
error_data_t *build_error_data  ( net_t * );
void         free_error_data    ( error_data_t ** );
void         init_error         ( error_data_t *, int );

/*  cache.c  */

boolean      build_cache        ( int, int, int, float ***, float *** );
void         free_cache         ( float ***, float ***, int );
void         compute_cache      ( int, data_set_t *, float ** );
void         recompute_cache    ( int, net_t *, data_set_t *, float ** );

/*  interface.c  */

void         cli                ( boolean );
boolean      process_command    ( boolean, char *, char *, char * );
int          find_key           ( char * );
void         display_parm       ( parm_t );
void         set_parm           ( boolean, parm_t, char *, char * );
void         set_parmtable      ( train_parm_t * );

void         train              ( char *, char * );
void         test               ( char *, char * );
void         predict            ( char *, char * );
void         quit               ( char *, char * );
void         list_parms         ( char *, char * );
void         list_data          ( char *, char * );
void         list_nets          ( char *, char * );
void         run_trials         ( char *, char * );
void         load_script        ( char *, char * );
void         save_script        ( char *, char * );
void         save_net           ( char *, char * );
void         load_net           ( char *, char * );
void         resize_net         ( char *, char * );
void         kill_data          ( char *, char * );
void         kill_net           ( char *, char * );
void         inspect_net        ( char *, char * );
void         inspect_data       ( char *, char * );
void         load_data          ( char *, char * );
void         sync_net           ( char *, char * );

void         print_cvrt         ( cvrt_t * );
cvrt_t       read_map           ( FILE *, char * );

void         trap_ctrl_c        ( int );
void         handle_interrupt   ( train_data_t *, int );

/* display.c */

void         display_banner            ( void );
void         display_begin_trial       ( int, time_t );
void         display_trainout_results  ( net_t *, error_data_t *, 
					 status_t, error_t );
void         display_validate_results  ( trial_result_t, error_t, float, int );
void         display_traincand_results ( net_t *, train_data_t *, status_t ); 
void         display_trial_results     ( trial_result_t, int, boolean, 
					 error_t, int, time_t );
void         display_run_results       ( trial_result_t, int, error_t );
void         display_test_results      ( trial_result_t );

/* query.c */

void         query_net                 ( char *, char * );
void         raw_input                 ( char * );
void         token_input               ( char * );

#endif

