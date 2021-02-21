#ifndef GTSD_INCLUDE_SELFSIMILARITY_SELFSIMILARITY_H
#define GTSD_INCLUDE_SELFSIMILARITY_SELFSIMILARITY_H

/* allocators and various functions */
#include <stdlib.h>

/* cstring functions */
#include <string.h>

/* exposes uint32_t etc.. */
#include <stdint.h>

/* STACK_* functions */
#include <info/ptrcheck.h>

/* clock_t */
#include <time.h>

#include "features.h"

/*
 * Generates and writes the selfsimilarity matrix out to disk gevn a length and
 * and the timeseries to work with.
 */
int selfsimilarity_genmatrix(size_t len, double *ts, const char *out);

#endif
