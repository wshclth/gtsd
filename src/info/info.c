#include <info/info.h>

void
_stack_trace(STACK_FUNC_HEADER)
{
  flockfile(stdout);
  printf("->   | %s:%s:%d\n", file, func, line);
  fflush(stdout);
  funlockfile(stdout);
}

void __attribute__((__format__(__printf__, 4, 0)))
_stack_error(STACK_FUNC_HEADER, char *fmt, ...)
{
  flockfile(stdout);
  printf(RED "error: " RESET "%s:%s:%d\n", file, func, line);
  printf(    "     | " RED);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf(RESET "\n");

  funlockfile(stdout);
}

void __attribute__((__format__(__printf__, 4, 0)))
_stack_info(STACK_FUNC_HEADER, char *fmt, ...)
{
  flockfile(stdout);
  printf(WHT "info: " RESET "%s:%s:%d ", file, func, line);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");

  funlockfile(stdout);
}
