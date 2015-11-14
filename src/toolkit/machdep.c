#ifdef HPUX

double ceil ( double src )
{
  int isrc;

  isrc = (int)src;

  if  ( (double)isrc == src )  return src;
  if  ( src < 0.0 ) return (double)isrc;
  return (double)(isrc + 1.0);
}

double fmod ( double x, double y )
{
  double result = x / y;
  int    ires;
  
