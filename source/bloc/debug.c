#include <stdio.h>
#include "debug.h"

int debug = 0;
int indent = 0;

void
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
