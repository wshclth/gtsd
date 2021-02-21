#ifndef GTSD_INCLUDE_SELFSIMILARITY_FEATURES_H
#define GTSD_INCLUDE_SELFSIMILARITY_FEATURES_H

/* memory allocators and misc functions */
#include <stdlib.h>

/* STACK_ functions for logging  and CHECK_ALLOC */
#include <info/info.h>
#include <info/ptrcheck.h>

/* sqrt */
#include <math.h>

/* memcpy */
#include <string.h>

/* Holds meta data about the features and the normalized features */
typedef struct
{
  /* the number of elements in a single feature */
  size_t feature_size;

  /* the number of features */
  size_t num_features;

  /*
   * a 2D array of size `num_features x feature_size` that holds the normalized
   * features
   */
  double **features;
} features_t;

/*
 * Generates a feature set from the timeseries (ts). This function expect
 * _ret to have set the feature_size and num_features before passing to this
 * function.
 */
int selfsimilarity_genfeatures(const double *ts, features_t *_ret);

#endif
