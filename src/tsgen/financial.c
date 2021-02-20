#include <tsgen/financial.h>

int
financial_randomwalk(size_t steps, double start, double **_walk)
{

  (*_walk) = calloc(steps, sizeof(double));
  CHECK_ALLOC(*_walk, sizeof(double) * steps);

  /* set the initial walk value */
  (*_walk)[0] = start;
  for (size_t step = 1; step < steps; ++step)
  {
    (*_walk)[step] = (*_walk)[step-1] + randf();
  }

  return 1;
}
