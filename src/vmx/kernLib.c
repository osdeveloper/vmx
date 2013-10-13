/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* kernLib.c - Kernel */

#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/taskArchLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <util/qFifoLib.h>
#include <vmx/taskLib.h>
#include <vmx/kernLib.h>
#include <vmx/kernQLib.h>
#include <vmx/vmxLib.h>
#include <vmx/kernHookLib.h>

/* Imports */
IMPORT void kernTaskLoadContext(void);

/* Globals */
BOOL kernState = FALSE;
BOOL kernRoundRobin = FALSE;
unsigned kernRoundRobinTimeSlice = 0;
TCB_ID kernCurrTaskId = NULL;
Q_HEAD kernActiveQ = {NULL, 0, 0 ,NULL};
Q_HEAD kernTickQ;
Q_HEAD kernReadyQ;
volatile unsigned kernTicks = 0;
volatile unsigned kernAbsTicks = 0;
FUNCPTR kernCreateHooks[MAX_KERNEL_CREATE_HOOKS + 1];
FUNCPTR kernSwitchHooks[MAX_KERNEL_SWITCH_HOOKS + 1];
FUNCPTR kernDeleteHooks[MAX_KERNEL_DELETE_HOOKS + 1];
FUNCPTR kernSwapHooks[MAX_KERNEL_SWAP_HOOKS + 1];
int kernSwapReference[MAX_KERNEL_SWAP_HOOKS + 1];

/******************************************************************************
* kernInit - Initialize kernel
*
* RETURNS: N/A
******************************************************************************/

void kernInit(FUNCPTR rootTask)
{
  int level;
  TCB_ID rootTcb, idleTcb;

  /* Initialize kernel work queue */
  kernQLibInit();

  /* Initialize queues */
  qInit(&kernActiveQ, qFifoClassId);
  qInit(&kernTickQ, qPrioClassId);
  qInit(&kernReadyQ, qPrioClassId);

  /* Initialize variables */
   kernState = FALSE;
   kernRoundRobin = FALSE;
   kernRoundRobinTimeSlice = 0;
   kernTicks = 0;
   kernAbsTicks = 0;

  /* Initialize kernel extension library */
  kernHookLibInit();

  /* Initialize root task */
  rootTcb = taskCreate("rootTask", 0, 0,
		       DEFAULT_STACK_SIZE, rootTask,
		       0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  vmxResume(rootTcb);

  idleTcb = taskCreate("idleTask", 255, 0,
		       DEFAULT_STACK_SIZE, (FUNCPTR) taskIdle,
		       0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  vmxResume(idleTcb);

  kernCurrTaskId = rootTcb;

  INT_LOCK(level);

  intConnectFunction(TIMER_INTERRUPT_NUM, vmxTickAnnounce, NULL);
  kernTaskLoadContext();

  INT_UNLOCK(level);
}

/******************************************************************************
* kernTimeSlice - Enable/Disable round robin task scheduling
*
* RETURNS: N/A
******************************************************************************/

STATUS kernTimeSlice(unsigned ticks)
{
  /* 0 turns round robin off */
  if (ticks == 0)
  {
    kernRoundRobin = FALSE;
  }
  else
  {
    kernRoundRobinTimeSlice = ticks;
    kernRoundRobin = TRUE;
  }

  return(OK);
}

