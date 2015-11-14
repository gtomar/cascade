#include <stdio.h>
#include "toolkit.h"

void main()
{
  char *s1 = "sadfdsa",
       *s2 = "adfs",
       *s3 = "jkl;";

  printf ("Original: %s, %s, %s\n",s1,s2,s3);
  printf ("Ordered:%s, %s, %s\n",
	  str_order( s1 ), str_order( s2 ), str_order( s3 ) );
  s1 = str_or ( s1, s2 );
  printf ("s1+s2: %s\n",s1);
  s1 = str_or ( s1, s3 );
  printf ("s1+s2+s3: %s\n",s1);
}
