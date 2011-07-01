#include <stdio.h>
#include "debug.h"

int __debug_id__ = 0;
int __debug_indent__ = 0;

void
textcolor(color_t color)
{
  switch (color)
  {
    case GRAY:
      printf("[33;01;30m");
      break;
    case RED:
      printf("[33;01;31m");
      break;
    case GREEN:
      printf("[33;01;32m");
      break;
    case YELLOW:
      printf("[33;01;33m");
      break;
    case PURPLE:
      printf("[33;01;34m");
      break;
    case PINK:
      printf("[33;01;35m");
      break;
    case BLUE:
      printf("[33;01;36m");
      break;
    case LRED:
      printf("[33;00;31m");
      break;
    case LGREEN:
      printf("[33;00;32m");
      break;
    case LYELLOW:
      printf("[33;00;33m");
      break;
    case LPURPLE:
      printf("[33;00;34m");
      break;
    case LPINK:
      printf("[33;00;35m");
      break;
    case LBLUE:
      printf("[33;00;36m");
      break;
    default:
    case WHITE:
      printf("[0m");
      break;
  }
  fflush(stdout);
}
