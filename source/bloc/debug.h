#ifndef DEBUG_H
# define DEBUG_H

# include <stdio.h>

typedef enum
{
  YELLOW,
  BLUE,
  GREEN,
  RED,
  WHITE,
} color_t;

extern int debug;
extern int indent;

inline void
textcolor(color_t color)
{
  switch (color)
  {
    case YELLOW:
      printf("[33;01;33m");
      break;
    case BLUE:
      printf("[33;01;36m");
      break;
    case GREEN:
      printf("[33;01;32m");
      break;
    case RED:
      printf("[33;01;31m");
      break;
    default:
    case WHITE:
      printf("[0m");
      break;
  }
  fflush(stdout);
}

# ifdef DEBUG
#  define DEBUG_PRINT(S, Color, ...)                 \
  {                                                  \
    textcolor(Color);                                \
    for (int __i__ = 0; __i__ < indent; __i__++)     \
      printf("    ");                                \
    printf(S "\n", ##__VA_ARGS__);                   \
    textcolor(WHITE);                                \
  }

# define DEBUG_INC                              \
  indent++

# define DEBUG_DEC                              \
  indent--

#  define DEBUG_SERVER(S, ...)                  \
  DEBUG_PRINT(S, YELLOW, ##__VA_ARGS__)

#  define DEBUG_SEND(S, ...)                    \
  DEBUG_PRINT(S, BLUE, ##__VA_ARGS__)

#  define DEBUG_RECEIVE(S, ...)                 \
  DEBUG_PRINT(S, GREEN, ##__VA_ARGS__)

#  define DEBUG_HELLO(S, ...)                    \
  DEBUG_PRINT(S, RED, ##__VA_ARGS__)

# else
#  define DEBUG_PRINT(S, ...)
#  define DEBUG_SERVER(S, ...)
#  define DEBUG_SEND(S, ...)
#  define DEBUG_RECEIVE(S, ...)
#  define DEBUG_HELLO(S, ...)
# endif

#endif
