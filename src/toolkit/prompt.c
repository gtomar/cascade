#include <stdio.h>
#include "toolkit.h"

boolean prompt_yn ( char *prompt, boolean def )
{
  char response;

  if  ( def )
    printf ("%s (Yn)? ", prompt);
  else
    printf ("%s (yN)? ", prompt);
  fflush( stdout );
  response = getchar( );

  if  ( (response == 'Y') || (response == 'y') )
    return TRUE;
  if  ( (response == 'N') || (response =='n') )
    return FALSE;
  return def;
}

boolean prompt_yesno ( char *prompt )
{
  char response[4];

  while ( TRUE )  {
    printf ("%s (yes or no)? ", prompt );
    fgets  ( response, 4, stdin );
    if  ( !strncasecmp( response, "yes", 3 ) )
      return TRUE;
    if  ( !strncasecmp( response, "no", 2 ) )
      return FALSE;
  }
}


