#ifndef GTSD_INCLUDE_INFO_INFO_H
#define GTSD_INCLUDE_INFO_INFO_H

#include <stdio.h>

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define STACK_TRACE(LEVEL) \
  _stack_trace(LEVEL, __FILE_NAME__, __func__, __LINE__)

void _stack_trace(int level, const char *file, const char *func,
    int line);

#endif
