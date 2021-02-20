#ifndef GTSD_INCLUDE_TSGEN_FINANCIAL_H
#define GTSD_INCLUDE_TSGEN_FINANCIAL_H

/* exposes standard fixed size variables */
#include <stddef.h>

/* exposes allocators and other misc function */
#include <stdlib.h>

/* exposes pointer chcking */
#include <info/ptrcheck.h>

#include "randf.h"

/*
 * Generate a random walk with the given number of steps and a starting point.
 *
 * @param steps the number of times to walk
 * @param start the starting vluae of the walk
 * @param _walk a place to allocate the number of doubles and place the random
 *        walk in.
 */
int financial_randomwalk(size_t steps, double start, double **_walk);

#endif
