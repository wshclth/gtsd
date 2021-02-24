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

/* sysconf */
#include <unistd.h>

/* threading library for CPU concurrency */
#include <pthread.h>

/* acos */
#include <math.h>

/* opencl gpu */
#include <CL/cl.h>

#include "features.h"

/*
 * Generates and writes the selfsimilarity matrix out to disk gevn a length and
 * and the timeseries to work with.
 */
int selfsimilarity_genmatrix(size_t len, double *ts, const char *out);

int selfsimilarity_genmatrix_gpu(size_t len, double *ts, const char *out,
    size_t feature_width);

#endif
