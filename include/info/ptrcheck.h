#ifndef GTSD_SRC_PTRCHECK_H
#define GTSD_SRC_PTRCHECK_H

#include <info/info.h>

#define CHECK_ALLOC(VAR, SIZE)\
  do {\
    if (VAR == NULL)\
    {\
      STACK_ERROR("(m)(c)alloc failed to allocate %lu bytes", SIZE);\
      VAR = NULL;\
      return 0;\
    }\
  } while (0)

#endif
