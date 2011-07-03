#ifndef DEBUG_H
# define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include "dashes.h"
#include "olsr_time.h"

typedef enum
{
  WHITE = 0,
  GRAY, YELLOW, BLUE, GREEN, RED, PURPLE, PINK,
  LYELLOW, LBLUE, LGREEN, LRED, LPURPLE, LPINK,
} color_t;

extern int __debug_id__;
extern int __debug_indent__;

void textcolor(color_t color);

# define PRINT_(S, Color, ...)                                          \
  {                                                                     \
    printf("[%d]{%010d} ", __debug_id__, olsr_get_current_time());      \
    for (int __i__ = 0; __i__ < __debug_indent__; __i__++)              \
      printf("    ");                                                   \
    textcolor(Color);                                                   \
    printf(S "\n", ##__VA_ARGS__);                                      \
    textcolor(WHITE);                                                   \
  }

#  define DEBUG_SET_ID(Id)                      \
  __debug_id__ = Id

# define DEBUG_INC                              \
  __debug_indent__++

# define DEBUG_DEC                              \
  __debug_indent__--

# ifdef DEBUG

#  define PRINT(S, Color, ...)                                          \
    PRINT_(S, Color, ##__VA_ARGS__);                                    \

#  define DEBUG_PRINT(S, Color, ...)            \
  PRINT(S, Color, ##__VA_ARGS__)

# else
#  define PRINT(S, Color, ...)                  \
  PRINT_(S, Color, ##__VA_ARGS__)
#  define DEBUG_PRINT(S, Color, ...)
# endif

# define DEBUG_SERVER(S, ...)                   \
  DEBUG_PRINT(S, LPINK, ##__VA_ARGS__)

# define DEBUG_SEND(S, ...)                     \
  DEBUG_PRINT(S, LBLUE, ##__VA_ARGS__)

# define DEBUG_RECEIVE(S, ...)                  \
  DEBUG_PRINT(S, LGREEN, ##__VA_ARGS__)

# define DEBUG_HELLO(S, ...)                    \
  DEBUG_PRINT(S, LPURPLE, ##__VA_ARGS__)

# define DEBUG_APPLI(S, ...)                    \
  DEBUG_PRINT(S, LPINK, ##__VA_ARGS__)

# define DEBUG_LINK(S, ...)                     \
  DEBUG_PRINT(S, PURPLE, ##__VA_ARGS__)

# define DEBUG_MPR(S, ...)                      \
  DEBUG_PRINT(S, BLUE, ##__VA_ARGS__)

# define DEBUG_MS(S, ...)                       \
  DEBUG_PRINT(S, RED, ##__VA_ARGS__)

# define DEBUG_NEIGHBOR(S, ...)                 \
  DEBUG_PRINT(S, YELLOW, ##__VA_ARGS__)

# define DEBUG_NEIGHBOR2(S, ...)                \
  DEBUG_PRINT(S, PINK, ##__VA_ARGS__)

# define DEBUG_DUPLICATE(S, ...)                \
  DEBUG_PRINT(S, GREEN, ##__VA_ARGS__)

# define DEBUG_TOPOLOGY(S, ...)                 \
  DEBUG_PRINT(S, GRAY, ##__VA_ARGS__)

# define DEBUG_LINK_SET(S, ...)                 \
  DEBUG_PRINT(S, PURPLE, ##__VA_ARGS__)

# define DEBUG_MPR_SET(S, ...)                  \
  DEBUG_PRINT(S, BLUE, ##__VA_ARGS__)

# define DEBUG_MS_SET(S, ...)                   \
  DEBUG_PRINT(S, RED, ##__VA_ARGS__)

# define DEBUG_NEIGHBOR_SET(S, ...)             \
  DEBUG_PRINT(S, YELLOW, ##__VA_ARGS__)

# define DEBUG_NEIGHBOR2_SET(S, ...)            \
  DEBUG_PRINT(S, PINK, ##__VA_ARGS__)

# define DEBUG_DUPLICATE_SET(S, ...)            \
  DEBUG_PRINT(S, GREEN, ##__VA_ARGS__)

# define DEBUG_TOPOLOGY_SET(S, ...)             \
  DEBUG_PRINT(S, GRAY, ##__VA_ARGS__)

# define DEBUG_ROUTING_TABLE(S, ...)            \
  DEBUG_PRINT(S, GRAY, ##__VA_ARGS__)

# ifdef WARNINGS
#  define WARNING(S, ...)                       \
  PRINT(S, LYELLOW, ##__VA_ARGS__)
# else
#  define WARNING(S, ...)
# endif

# ifdef ERRORS
#  define ERROR(S, ...)                          \
  PRINT(S, LRED, ##__VA_ARGS__);                 \
  abort()
# else
#  define ERROR(S, ...)
# endif

#endif
