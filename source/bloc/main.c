#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// FreeRTOS and STM32 features
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <croutine.h>

#include "olsr/olsr.h"
#include "olsr/olsr_packet.h"
#include "olsr/olsr_message.h"
#include "comm/simulator.h"

int
main (int argc, char** argv)
{
  const int id = simulator_init(atoi(argv[1]), olsr_receive_callback);
  const address_t main_address = id << 2;
  DEBUG_SET_ID(id);
  DEBUG_SERVER("MAX_PACKET_SIZE is          %d", (int)MAX_PACKET_SIZE);
  DEBUG_SERVER("MAX_PACKET_CONTENT_SIZE is  %d", (int)MAX_PACKET_CONTENT_SIZE);
  DEBUG_SERVER("MAX_PACKET_MESSAGES is      %d", (int)MAX_PACKET_MESSAGES);
  DEBUG_SERVER("MAX_MESSAGE_SIZE is         %d", (int)MAX_MESSAGE_SIZE);
  DEBUG_SERVER("MAX_MESSAGE_CONTENT_SIZE is %d", (int)MAX_MESSAGE_CONTENT_SIZE);
  DEBUG_SERVER("main address is 0x%x", main_address);
  olsr_init(main_address);

  // Start scheduler and tasks
  vTaskStartScheduler();

  for (;;) {}

  return 0;
}

void
vApplicationStackOverflowHook(void)
{
  /* This will get called if an overflow is detected in the stack of a task.
     Inspect pxCurrentTCB to see which was the offending task. */
  for( ;; )
  {}
}

void
vMainQueueSendPassed(void)
{
}

void
vApplicationIdleHook(void)
{
  /* The co-routines are executed in the idle task using the idle task hook. */
  //vCoRoutineSchedule(); /* Comment this out if not using Co-routines. */

#ifdef __GCC_POSIX__
  struct timespec xTimeToSleep, xTimeSlept;
  /* Makes the process more agreeable when using the Posix simulator. */
  xTimeToSleep.tv_sec = 1;
  xTimeToSleep.tv_nsec = 0;
  nanosleep( &xTimeToSleep, &xTimeSlept );
#endif
}
