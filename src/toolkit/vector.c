#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"

double *vector_copy  ( double *dest, double *src, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = src[i];

  return dest;
}

double *vector_add  ( double *dest, double *src1, double *src2, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = src1[i] + src2[i];

  return dest;
}

double *vector_sub  ( double *dest, double *src1, double *src2, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = src1[i] - src2[i];

  return dest;
}

double *vector_multF  ( double *dest, double *src, double operand, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = src[i] * operand;

  return dest;
}

double *vector_divF  ( double *dest, double *src, double operand, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = src[i] / operand;

  return dest;
}

double *vector_negate  ( double *dest, double *src, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = -src[i];

  return dest;
}

double vector_dot  ( double *src1, double *src2, int len )
{
  int    i;
  double retVal = 0.0;

  for  ( i = 0 ; i < len ; i++ )
    retVal += src1[i] * src2[i];

  return retVal;
}

double vector_len  ( double *src, int len )
{
  return  sqrt( vector_dot( src, src, len ) );
}

void vector_print  ( double *src, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    printf ( "%f\t", src[i] );
  printf ("\n");
}

double *vector_random  ( double *dest, double range, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    dest[i] = range * (((double)(random( )%1000))/500.0 - 1.0);

  return dest;
}

double *vector_zero  ( double *dest, int length )
{
  int i;

  for  ( i = 0 ; i < length ; i++ )
    dest[i] = 0.0;

  return dest;
}

double vector_sum  ( double *src, int length )
{
  int    i;
  double sum = 0.0;

  for  ( i = 0 ; i < length ; i++ )
    sum += src[i];

  return sum;
}


/*  Return the index of the maximum element of a vector of doubles.  If two or
    more elements have a maximum value, the first of these is returned.
    */

int vector_max_index  ( double *src, int length )
{
  int max = 0,
      i;

  for  ( i = 1 ; i < length ; i++ )
    if  ( src[i] > src[max] )
      max = i;

  return max;
}


/*  Return the index of the minimum element of a vector of doubles.  If two or
    more elements have a minimum value, the first of these is returned.
    */

int vector_min_index  ( double *src, int length )
{
  int min = 0,
      i;

  for  ( i = 1 ; i < length ; i++ )
    if  ( src[i] < src[min] )
      min = i;
  
  return min;
}


/*  Set all the elements of a vector to a certain value
 */

double *vector_set  ( double *dest, double val, int length )
{
  int i;

  for  ( i = 0 ; i < length ; i++ )
    dest[i] = val;

  return dest;
}
