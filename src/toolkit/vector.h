#ifndef VECTOR
#define VECTOR

/*  vector.c  */

double *vector_copy      ( double *, double *, int );
double *vector_add       ( double *, double *, double *, int );
double *vector_sub       ( double *, double *, double *, int );
double *vector_multF     ( double *, double *, double, int );
double *vector_divF      ( double *, double *, double, int );
double *vector_negate    ( double *, double *, int );
double vector_dot        ( double *, double *, int );
double vector_len        ( double *, int );
void   vector_print      ( double *, int );
double *vector_random    ( double *, double, int );
double *vector_zero      ( double *, int );
double vector_sum        ( double *, int );
int    vector_max_index  ( double *, int );
int    vector_min_index  ( double *, int );
double *vector_set       ( double *, double, int );

/*  vectori.c  */

int  *vectori_zero        ( int *, int );
void vectori_print        ( int *, int );
int  vectori_sum          ( int *, int );
int  *vectori_plus_scan   ( int *, int *, int );
int  *vectori_scan        ( int *, int *, int (*)(int, int), int );

#endif
