#include <tsgen/randf.h>

double
randf(void)
{
  return (rand() - RAND_HALF) / RAND_HALF;
}
