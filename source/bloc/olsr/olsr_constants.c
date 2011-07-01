#include <stdlib.h>

#include "olsr_constants.h"
#include "olsr_message.h"

#define CASE(E)                                \
  case E:                                      \
  return #E

#define DEFAULT_(Var)                           \
  sprintf(buf, message, Var);                   \
  ERROR("ERROR:%d", Var);                       \
  return buf

#define DEFAULT(Var)                            \
  default:                                      \
  DEFAULT_(Var)

static const char* message = "ERROR:%d";

const char*
olsr_neighbor_type_str(neighbor_type_t t)
{
  static char buf[32] __attribute__((unused));
  switch (t)
  {
    CASE(NOT_NEIGH);
    CASE(SYM_NEIGH);
    CASE(MPR_NEIGH);
    DEFAULT(t);
  }
}

const char*
olsr_link_type_str(link_type_t t)
{
  static char buf[32] __attribute__((unused));
  switch (t)
  {
    CASE(UNSPEC_LINK);
    CASE(ASYM_LINK);
    CASE(SYM_LINK);
    CASE(LOST_LINK);
    DEFAULT(t);
  }
}

const char*
olsr_willingness_str(willingness_t w)
{
  static char buf[32] __attribute__((unused));
  switch (w)
  {
    CASE(WILL_NEVER);
    CASE(WILL_LOW);
    CASE(WILL_DEFAULT);
    CASE(WILL_HIGH);
    CASE(WILL_ALWAYS);
    DEFAULT(w);
  }
}

const char*
olsr_link_status_str(link_status_t s)
{
  static char buf[32] __attribute__((unused));
  switch (s)
  {
    CASE(NOT_SYM);
    CASE(SYM);
    DEFAULT(s);
  }
}

const char*
olsr_message_type_str(message_type_t t)
{
  static char buf[32] __attribute__((unused));
  switch (t)
  {
    CASE(HELLO_MESSAGE);
    CASE(TC_MESSAGE);
    CASE(MID_MESSAGE);
    CASE(HNA_MESSAGE);
    DEFAULT(t);
  }
}

const char*
olsr_iface_str(interface_t iface)
{
  static char buf[32] __attribute__((unused));
  switch (iface)
  {
    case NORTH_IFACE:
      return "N";
      break;
    case WEST_IFACE:
      return "W";
      break;
    case SOUTH_IFACE:
      return "S";
      break;
    case EAST_IFACE:
      return "E";
      break;
    default:
      DEFAULT_(iface);
  }
}
