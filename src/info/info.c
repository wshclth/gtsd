#include <info/info.h>

void
_stack_trace(int level, const char *file, const char *func, int line)
{
  flockfile(stdout);
  if (level != 0)
  {
    printf("-> %.*s %s:%s:%d\n", 2*level, " ", file, func, line);
  }
  else
  {
    printf("-> %s:%s:%d\n", file, func, line);
  }
  fflush(stdout);
  funlockfile(stdout);
}
