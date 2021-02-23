#ifndef GTSD_INCLUDE_TSGEN_TSGEN_H
#define GTSD_INCLUDE_TSGEN_TSGEN_H

#include <stdlib.h>
#include <stdint.h>
#include <info/info.h>
#include <info/ptrcheck.h>

#include "randf.h"
#include "audio.h"
#include "financial.h"

enum GENERATOR_FUNCTION
{
  RANDOM_WALK = 0,
  LOAD_FILE = 1
};

/*
 * Loads a file. See documentation for file format specifications.
 */
int tsgen_loadfile(const char *file_name, double **_data, size_t *_len);

#endif
