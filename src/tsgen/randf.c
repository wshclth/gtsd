#include <tsgen/randf.h>

double
randf()
{
  return (rand() - RAND_HALF) / RAND_HALF;
}
