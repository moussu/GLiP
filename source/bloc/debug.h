#ifndef DEBUG_H
# define DEBUG_H

#include <stdio.h>
#include "dashes.h"

typedef enum
{
  WHITE = 0,
  GRAY, YELLOW, BLUE, GREEN, RED, PURPLE, PINK,
  LYELLOW, LBLUE, LGREEN, LRED, LPURPLE, LPINK,
} color_t;

extern int debug;
extern int indent;

void textcolor(color_t color);

# ifdef DEBUG
#  define DEBUG_SET_ID(Id)                           \
  debug = Id

#  define DEBUG_PRINT(S, Color, ...)                 \
  {                                                  \
    printf("[%d] ", debug);                          \
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
  DEBUG_PRINT(S, LYELLOW, ##__VA_ARGS__)

#  define DEBUG_SEND(S, ...)                    \

//  DEBUG_PRINT(S, LBLUE, ##__VA_ARGS__)

#  define DEBUG_RECEIVE(S, ...)                 \
  DEBUG_PRINT(S, LGREEN, ##__VA_ARGS__)

#  define DEBUG_HELLO(S, ...)                   \

//  DEBUG_PRINT(S, LRED, ##__VA_ARGS__)

#  define DEBUG_SET(S, ...)                     \
  DEBUG_PRINT(S, LPURPLE, ##__VA_ARGS__)

#  define DEBUG_LINK(S, ...)                    \

//  DEBUG_PRINT(S, PURPLE, ##__VA_ARGS__)

#  define DEBUG_MPR(S, ...)                     \

//  DEBUG_PRINT(S, BLUE, ##__VA_ARGS__)

#  define DEBUG_MS(S, ...)                      \
  DEBUG_PRINT(S, RED, ##__VA_ARGS__)

#  define DEBUG_NEIGHBOR(S, ...)                \
  DEBUG_PRINT(S, YELLOW, ##__VA_ARGS__)

#  define DEBUG_NEIGHBOR2(S, ...)                \
  DEBUG_PRINT(S, PINK, ##__VA_ARGS__)

#  define DEBUG_DUPLICATE(S, ...)               \
  DEBUG_PRINT(S, GREEN, ##__VA_ARGS__)

#  define DEBUG_TOPOLOGY(S, ...)                \
  DEBUG_PRINT(S, GRAY, ##__VA_ARGS__)

#  define MPR(S, ...)                                  \
  DEBUG_MPR(S, ##__VA_ARGS__);                         \
  DEBUG_INC;                                           \
  olsr_neighbor_set_print();                           \
  olsr_N_set_print();                                  \
  olsr_neighbor2_set_print();                          \
  olsr_N2_set_print();                                 \
  olsr_mpr_set_print();                                \
  DEBUG_DEC;

# else
#  define DEBUG_SET_ID(Id)
#  define DEBUG_PRINT(S, ...)
#  define DEBUG_SERVER(S, ...)
#  define DEBUG_SEND(S, ...)
#  define DEBUG_RECEIVE(S, ...)
#  define DEBUG_HELLO(S, ...)
#  define DEBUG_SET(S, ...)
#  define DEBUG_LINK(S, ...)
#  define DEBUG_MPR(S, ...)
#  define DEBUG_MS(S, ...)
#  define DEBUG_NEIGHBOR(S, ...)
#  define DEBUG_NEIGHBOR2(S, ...)
#  define DEBUG_DUPLICATE(S, ...)
#  define DEBUG_TOPOLOGY(S, ...)
# endif

#endif
