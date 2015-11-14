#include "toolkit.h"

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
