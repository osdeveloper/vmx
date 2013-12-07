/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 - 2009 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* netLib.c - Network interface library */

#include <arch/intArchLib.h>
#include <net/netLib.h>
#include <vmx/semLib.h>
#include <vmx/taskLib.h>
#include <util/qLib.h>
#include <util/rngLib.h>

/* Locals */
LOCAL SEMAPHORE netTaskSem;
LOCAL RING_ID netWorkRing;

/* Globlas */
BOOL netLibInstalled = FALSE;
int netTaskId;
int netTaskPriority = 50;
int netTaskOptions = TASK_OPTIONS_UNBREAKABLE;
int netTaskStackSize = 10000;
SEM_ID netTaskSemId = &netTaskSem;

VOIDFUNCPTR rtMissMsgHook = (VOIDFUNCPTR) NULL;
VOIDFUNCPTR rtIfaceMsgHook = (VOIDFUNCPTR) NULL;
VOIDFUNCPTR rtNewAddrMsgHook = (VOIDFUNCPTR) NULL;

/******************************************************************************
* netLibInit - Initialize network library
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS netLibInit(void)
{
  if (netLibInstalled == TRUE) {

    /* Return error if task not initialized */
    if (netTaskId == (int) NULL)
      return ERROR;

    return OK;
  }

  /* Mark as initialized */
  netLibInstalled = TRUE;

  /* Create network ring buffer */
  netWorkRing = rngCreate(NET_LIB_RING_SIZE);
  if (netWorkRing == NULL)
    return ERROR;

  /* Initialize network semaphore */
  semBInit(netTaskSemId, SEM_Q_PRIORITY, SEM_EMPTY);

  /* Start network task */
  netTaskId = taskSpawn("tNetTask", netTaskPriority,
			netTaskOptions, netTaskStackSize,
			(FUNCPTR) netTask,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  if (netTaskId == (int) NULL)
    return ERROR;

  return OK;
}

/******************************************************************************
* netTask - Network task
*
* RETURNS: N/A
******************************************************************************/

void netTask(void)
{
  NET_JOB_NODE node;

  /* Endless loop */
  while (1) {

    /* Wait for wakeup call */
    semTake(netTaskSemId, WAIT_FOREVER);

    /* Process work list */
    while ( rngIsEmpty(netWorkRing) == FALSE ) {

      /* Get work node, kill task if error */
      if ( rngBufGet(netWorkRing, (char *) &node,
		     sizeof(node)) != sizeof(node) )
        exit(1);

      /* Do work */
      ( *(node.func) ) ( node.args[0], node.args[1], node.args[2],
			 node.args[3], node.args[4] );

    } /* End process work list */

  } /* End endless loop */

}

/******************************************************************************
* netJobAdd - Add work to network task
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS netJobAdd(FUNCPTR func, ARG param0, ARG param1, ARG param2,
		 	       ARG param3, ARG param4)
{
  int level, ringPut;
  NET_JOB_NODE node;

  /* Copy node */
  node.func = func;
  node.args[0] = param0;
  node.args[1] = param1;
  node.args[2] = param2;
  node.args[3] = param3;
  node.args[4] = param4;

  /* Lock interrputs */
  INT_LOCK(level);

  /* Put job on ring buffer */
  ringPut = rngBufPut(netWorkRing, (char *) &node, sizeof(node));

  /* Unlock interrupts */
  INT_UNLOCK(level);

  if ( ringPut != sizeof(node) )
    return ERROR;

  /* Wake up network task */
  semGive(netTaskSemId);

  return OK;
}

