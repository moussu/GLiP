#include <stdlib.h>

#include "olsr_constants.h"

#define CASE(E)                                 \
  case E:                                       \
  return #E

const char*
olsr_neighbor_type_str(neighbor_type_t t)
{
  static char buf[32];
  switch (t)
  {
    CASE(NOT_NEIGH);
    CASE(SYM_NEIGH);
    CASE(MPR_NEIGH);
    default:
      printf("ERROR:%d", t);
      exit(1);
      return buf;
  }
}

const char*
olsr_link_type_str(link_type_t t)
{
  static char buf[32];
  switch (t)
  {
    CASE(UNSPEC_LINK);
    CASE(ASYM_LINK);
    CASE(SYM_LINK);
    CASE(LOST_LINK);
    default:
      printf("ERROR:%d", t);
      exit(1);
      return buf;
  }
}

const char*
olsr_willingness_str(willingness_t w)
{
  static char buf[32];
  switch (w)
  {
    CASE(WILL_NEVER);
    CASE(WILL_LOW);
    CASE(WILL_DEFAULT);
    CASE(WILL_HIGH);
    CASE(WILL_ALWAYS);
    default:
      printf("ERROR:%d", w);
      exit(1);
      return buf;
  }
}

const char*
olsr_link_status_str(link_status_t s)
{
  static char buf[32];
  switch (s)
  {
    CASE(NOT_SYM);
    CASE(SYM);
    default:
      printf("ERROR:%d", s);
      exit(1);
      return buf;
  }
}
