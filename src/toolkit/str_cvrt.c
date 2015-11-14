#include <string.h>
#include "toolkit.h"

boolean isint ( char *value )
{
  int len;

  if  ( (value[0] == '+') || (value[0] == '-') )
    value++;  
  len = strlen( value );
  if  ( (len == 0) || (strspn( value, "0123456789" ) != len) )
    return FALSE;
  return TRUE;
}

boolean isfloat ( char *value )
{
  int     len;

  if  ( (value[0] == '+') || (value[0] == '-') )
    value++;
  len = strlen( value );
  if  ( (len == 0) || (strspn( value, "0123456789.E" ) != len) )
    return FALSE;
  if  ( (strspn( value, "." ) > 1) || (strspn( value, "E" ) > 1)  )
    return FALSE;

  return TRUE;
}
  
boolean isboolean ( char *value )
{
  static char *valid[6] = { "true", "false", "yes", "no", "on", "off" };
  int         i;

  for  ( i = 0 ; i < 6 ; i++ )
    if  ( !strcasecmp( value, valid[i] ) )
      return TRUE;
  return FALSE;
}

char *btoa  ( int mode, boolean value )
{
  switch  ( mode )  {
    case TRUE_FALSE:  if ( value ) return "True"; else return "False";
    case YES_NO:      if ( value ) return "Yes"; else return "No";
    case ON_OFF:      if ( value ) return "On"; else return "Off";
    default:          return "Illegal Mode";
    }
}

boolean atob  ( char *value )
{
  if  ( !strcasecmp( value, "true" ) || !strcasecmp( value, "yes" ) ||
        !strcasecmp( value, "on" ) )
    return TRUE;
  return FALSE;
}
