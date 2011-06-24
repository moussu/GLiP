#include "olsr_constants.h"

#define CASE(E)                                 \
  case E:                                       \
  return #E

const char*
olsr_willingness_str(willingness_t w)
{
  switch (w)
  {
    CASE(WILL_NEVER);
    CASE(WILL_LOW);
    CASE(WILL_DEFAULT);
    CASE(WILL_HIGH);
    CASE(WILL_ALWAYS);
    default:
      return "ERROR";
  }
}

const char*
olsr_link_status_str(link_status_t s)
{
  switch (s)
  {
    CASE(NOT_SYM);
    CASE(SYM);
    default:
      return "ERROR";
  }
}
