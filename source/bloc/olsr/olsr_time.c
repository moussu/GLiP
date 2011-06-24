#include <FreeRTOS.h>
#include <task.h>
#include "olsr_time.h"

olsr_time_t
olsr_get_current_time()
{
  return xTaskGetTickCount();
}
