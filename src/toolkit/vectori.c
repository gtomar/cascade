#include <stdlib.h>

#include "vector.h"

int *
vectori_zero  ( int *src, int len )
{
  int i;

  for  ( i = 0 ; i < len ; i++ )
    src[i] = 0;

  return src;
}

void
vectori_print  ( int *src, int len )
{
  int i;
  
  for  ( i = 0 ; i < len ; i++ )
    printf ("%d\t", src[i] );
  printf ("\n");
}

int
vectori_sum  ( int *src, int len )
{
  int sum = 0,
      i;

  for  ( i = 0 ; i < len ; i++ )
    sum += src[i];

  return sum;
}

int *
vectori_plus_scan ( int *dest, int *src, int len )
{
  int i;

  if  ( len > 0 )
    dest[0] = src[0];
  for  ( i = 1 ; i < len ; i++ )
    dest[i] = dest[i-1] + src[i];

  return dest;
}

int *
vectori_scan ( int *dest, int *src, int (*func)(int, int), int len )
{
  int i;

  if  ( len > 0 )
    dest[0] = src[0];
  for  ( i = 1 ; i < len ; i++ )
    dest[i] = (*func)(dest[i-1], src[i]);

  return dest;
}
