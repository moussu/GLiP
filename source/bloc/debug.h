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

# define PRINT(S, Color, ...)                                           \
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
#  define DEBUG_PRINT(S, Color, ...)            \
  PRINT(S, Color, ##__VA_ARGS__)
# else
#  define DEBUG_PRINT(S, Color, ...)
# endif

//# define DEBUG_SERVER(S, ...)        DEBUG_PRINT(S, LPINK,   ##__VA_ARGS__)
//# define DEBUG_SEND(S, ...)          DEBUG_PRINT(S, LBLUE,   ##__VA_ARGS__)
//# define DEBUG_RECEIVE(S, ...)       DEBUG_PRINT(S, LGREEN,  ##__VA_ARGS__)
//# define DEBUG_HELLO(S, ...)         DEBUG_PRINT(S, LPURPLE, ##__VA_ARGS__)
//# define DEBUG_APPLI_SET(S, ...)     DEBUG_PRINT(S, LPINK,   ##__VA_ARGS__)
//# define DEBUG_APPLI(S, ...)         DEBUG_PRINT(S, LPINK,   ##__VA_ARGS__)
//# define DEBUG_LINK(S, ...)          DEBUG_PRINT(S, PURPLE,  ##__VA_ARGS__)
//# define DEBUG_MPR(S, ...)           DEBUG_PRINT(S, BLUE,    ##__VA_ARGS__)
//# define DEBUG_MS(S, ...)            DEBUG_PRINT(S, RED,     ##__VA_ARGS__)
//# define DEBUG_NEIGHBOR(S, ...)      DEBUG_PRINT(S, YELLOW,  ##__VA_ARGS__)
//# define DEBUG_NEIGHBOR2(S, ...)     DEBUG_PRINT(S, PINK,    ##__VA_ARGS__)
//# define DEBUG_DUPLICATE(S, ...)     DEBUG_PRINT(S, GREEN,   ##__VA_ARGS__)
//# define DEBUG_TOPOLOGY(S, ...)      DEBUG_PRINT(S, GRAY,    ##__VA_ARGS__)
//# define DEBUG_LINK_SET(S, ...)      DEBUG_PRINT(S, PURPLE,  ##__VA_ARGS__)
//# define DEBUG_MPR_SET(S, ...)       DEBUG_PRINT(S, BLUE,    ##__VA_ARGS__)
//# define DEBUG_MS_SET(S, ...)        DEBUG_PRINT(S, RED,     ##__VA_ARGS__)
//# define DEBUG_NEIGHBOR_SET(S, ...)  DEBUG_PRINT(S, YELLOW,  ##__VA_ARGS__)
//# define DEBUG_NEIGHBOR2_SET(S, ...) DEBUG_PRINT(S, PINK,    ##__VA_ARGS__)
//# define DEBUG_DUPLICATE_SET(S, ...) DEBUG_PRINT(S, GREEN,   ##__VA_ARGS__)
//# define DEBUG_TOPOLOGY_SET(S, ...)  DEBUG_PRINT(S, GRAY,    ##__VA_ARGS__)
//# define DEBUG_ROUTING_TABLE(S, ...) DEBUG_PRINT(S, GRAY,    ##__VA_ARGS__)

# ifndef DEBUG_SERVER
#  define DEBUG_SERVER(S, ...)
# endif
# ifndef DEBUG_SEND
#  define DEBUG_SEND(S, ...)
# endif
# ifndef DEBUG_RECEIVE
#  define DEBUG_RECEIVE(S, ...)
# endif
# ifndef DEBUG_HELLO
#  define DEBUG_HELLO(S, ...)
# endif
# ifndef DEBUG_APPLI_SET
#  define DEBUG_APPLI_SET(S, ...)
# endif
# ifndef DEBUG_APPLI
#  define DEBUG_APPLI(S, ...)
# endif
# ifndef DEBUG_LINK
#  define DEBUG_LINK(S, ...)
# endif
# ifndef DEBUG_MPR
#  define DEBUG_MPR(S, ...)
# endif
# ifndef DEBUG_MS
#  define DEBUG_MS(S, ...)
# endif
# ifndef DEBUG_NEIGHBOR
#  define DEBUG_NEIGHBOR(S, ...)
# endif
# ifndef DEBUG_NEIGHBOR2
#  define DEBUG_NEIGHBOR2(S, ...)
# endif
# ifndef DEBUG_DUPLICATE
#  define DEBUG_DUPLICATE(S, ...)
# endif
# ifndef DEBUG_TOPOLOGY
#  define DEBUG_TOPOLOGY(S, ...)
# endif
# ifndef DEBUG_LINK_SET
#  define DEBUG_LINK_SET(S, ...)
# endif
# ifndef DEBUG_MPR_SET
#  define DEBUG_MPR_SET(S, ...)
# endif
# ifndef DEBUG_MS_SET
#  define DEBUG_MS_SET(S, ...)
# endif
# ifndef DEBUG_NEIGHBOR_SET
#  define DEBUG_NEIGHBOR_SET(S, ...)
# endif
# ifndef DEBUG_NEIGHBOR2_SET
#  define DEBUG_NEIGHBOR2_SET(S, ...)
# endif
# ifndef DEBUG_DUPLICATE_SET
#  define DEBUG_DUPLICATE_SET(S, ...)
# endif
# ifndef DEBUG_TOPOLOGY_SET
#  define DEBUG_TOPOLOGY_SET(S, ...)
# endif
# ifndef DEBUG_ROUTING_TABLE
#  define DEBUG_ROUTING_TABLE(S, ...)
# endif

# ifdef WARNINGS
#  define WARNING(S, ...)                       \
  PRINT(S, LYELLOW, ##__VA_ARGS__)
# else
#  define WARNING(S, ...)
# endif

# ifdef ERRORS
#  define ERROR(S, ...)                                         \
  {                                                             \
    PRINT(S, LRED, ##__VA_ARGS__);                              \
    fflush(stdout);                                             \
    abort();                                                    \
  }
# else
#  define ERROR(S, ...)
# endif

#endif
