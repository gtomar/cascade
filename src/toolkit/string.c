#include <ctype.h>
#include <string.h>
#include "toolkit.h"

void str_addch( char *str, char newCh )
{
  int begin = 0,
      mid,
      end,
      i;

  end = strlen( str );
  while ( begin <= end )  {
    mid = (begin+end)/2;
    if ( newCh < str[mid] )
      end = mid-1;
    else if ( newCh == str[mid] )
      return;
    else
      begin = mid+1;
  }

  for  ( i = strlen( str ) ; i >= mid ; i-- )
    str [i+1] = str[i];
  str[mid] = newCh;
}

char *str_or ( char *str, char *strPrime )
{
  int  len,
       lenPrime,
       i;
  char *fn = "String OR";

  if ( (lenPrime = strcspn( strPrime, str )) == 0 )
    return str;
  len      = strlen( str );  
  str      = (char *)realloc_mem( str, len+lenPrime+1, sizeof( char ), fn );
  
  for  ( i = 0 ; i < strlen( strPrime ) ; i++ )
    str_addch( str, strPrime[i] );

  return str;
}

char *str_order ( char *str )
{
  int  i,j,h,len;
  char val;

  len = strlen( str );
  for ( h = 1 ; h <= len/9 ; h = 3*h+1 );
  for ( ; h > 0 ; h /= 3 )
    for ( i = h ; i < len ; i++ )  {
      val = str[i];
      j   = i;
      while ( (j >= h) && (str[j-h] > val) )  {
	str[j] =  str[j-h];
	j      -= h;
      }
      str[j] = val;
    }

  val = str[len];
  for  ( i = len-1; i >= 0 ; i-- )
    if  ( str[i] == val )  {
      for  ( j = i ; j < len ; j++ )
	str[j] = str[j+1];
      len--;
    } else
      val = str[i];

  return str;
}
