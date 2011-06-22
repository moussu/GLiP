/*
 * main.c
 *
 * Creates the tasks and launch the scheduler
 *
 * Copyright 2011 - Benjamin Bonny    <benjamin.bonny@gmail.com>,
 *                  Cédric Le Ninivin <cedriclen@gmail.com>,
 *                  Guillaume Normand <guillaume.normand.gn@gmail.com>
 *
 * All rights reserved.
 * MB Led
 * Telecom ParisTech - ELECINF344/ELECINF381
 *
 * This file is part of MB Led Project.
 *
 * MB Led Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MB Led Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MB Led Project.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

// FreeRTOS and STM32 features
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <croutine.h>

#include "olsr/olsr.h"
#include "comm/simulator.h"

int main (int argc, char** argv)
{
  const int id = simulator_init(atoi(argv[1]));
  const address_t main_address = id << 2;
  olsr_init(main_address);

  // Start scheduler and tasks
  vTaskStartScheduler();

  for(;;)
  {}

  return 0;
}

void vApplicationStackOverflowHook( void )
{
  /* This will get called if an overflow is detected in the stack of a task.
     Inspect pxCurrentTCB to see which was the offending task. */
  for( ;; )
  {}
}

void vMainQueueSendPassed( void )
{
}

void vApplicationIdleHook( void )
{
	/* The co-routines are executed in the idle task using the idle task hook. */
	//vCoRoutineSchedule();	/* Comment this out if not using Co-routines. */

#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
		/* Makes the process more agreeable when using the Posix simulator. */
		xTimeToSleep.tv_sec = 1;
		xTimeToSleep.tv_nsec = 0;
		nanosleep( &xTimeToSleep, &xTimeSlept );
#endif
}
