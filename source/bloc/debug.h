#ifndef DEBUG_H
# define DEBUG_H

# include <stdio.h>

extern int debug;

# ifdef DEBUG
#  define DEBUG_PRINT(S, ...)                   \
  printf("%d: " S, debug++, __VA_ARGS__)
# else
#  define DEBUG_PRINT(S, ...)
# endif

#endif
