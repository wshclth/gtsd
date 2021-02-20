#ifndef GTSD_INCLUDE_INFO_INFO_H
#define GTSD_INCLUDE_INFO_INFO_H

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

/* exposes printf */
#include <stdio.h>

/* exposes variadic arguments */
#include <stdarg.h>

/* exposes all the ansi color tables to make output pretty */
#include "ansicolors.h"

#define STACK_FUNC_HEADER \
  const char *file, const char *func, int line

#define STACK_TRACE() \
  _stack_trace(__FILE_NAME__, __func__, __LINE__)

#define STACK_ERROR(fmt, ...) \
  _stack_error(__FILE_NAME__, __func__, __LINE__, fmt, __VA_ARGS__)

#define STACK_INFO(fmt, ...) \
  _stack_info(__FILE_NAME__, __func__, __LINE__, fmt, __VA_ARGS__)

void _stack_trace(STACK_FUNC_HEADER);
void _stack_error(STACK_FUNC_HEADER, char *fmt, ...);
void _stack_info(STACK_FUNC_HEADER, char *fmt, ...);

#endif
