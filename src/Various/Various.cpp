#include "Various.h"


long GCD_Function ( long n1, long n2)
{
  long i,result;
  for(i=1;i<=n1 && i<=n2;i++)
  {
    if(n1%i==0 && n2%i==0)result = i;  
  }
  return result;     
}