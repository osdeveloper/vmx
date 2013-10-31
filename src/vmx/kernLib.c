/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2013 Surplus Users Ham Society
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
 */

/* kernLib.c - Kernel library*/

#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/taskArchLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <util/qPriBmpLib.h>
#include <util/qFifoLib.h>
#include <vmx/taskLib.h>
#include <vmx/workQLib.h>
#include <vmx/vmxLib.h>
#include <vmx/kernLib.h>

#define INCLUDE_CONSTANT_RDY_Q

/* Globals */
BOOL kernelInitialized           = FALSE;
BOOL kernelState                 = FALSE;
BOOL kernRoundRobin              = FALSE;
unsigned kernRoundRobinTimeSlice = 0;
TCB_ID taskIdCurrent             = NULL;
Q_HEAD kernActiveQ               = {NULL, 0, 0 ,NULL};
Q_HEAD kernTickQ;
Q_HEAD readyQHead;

/* Locals */
#ifdef INCLUDE_CONSTANT_RDY_Q
LOCAL DL_LIST kernReadyLst[256];
LOCAL unsigned kernReadyBmp[8];
#endif

/******************************************************************************
 * kernelInit - Initialize kernel
 *
 * RETURNS: N/A
 */

void kernInit(
    FUNCPTR rootTask
    )
{
    int level;
    int rootTaskId, idleTaskId;

    /* Initialize kernel work queue */
    workQLibInit();

    /* Initialize queues */
    qInit(&kernActiveQ, qFifoClassId);
    qInit(&kernTickQ, qPrioClassId);
#ifdef INCLUDE_CONSTANT_RDY_Q
    qInit(&readyQHead, qPriBmpClassId, 256, kernReadyLst, kernReadyBmp);
#else
    qInit(&readyQHead, qPrioClassId);
#endif

    /* Initialize variables */
    kernelState             = FALSE;
    kernRoundRobin          = FALSE;
    kernRoundRobinTimeSlice = 0;

    /* Create and start root task */
    rootTaskId = taskCreat(
        "rootTask",
        0,
        0,
        DEFAULT_STACK_SIZE,
        rootTask,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        );
    vmxResume((TCB_ID) rootTaskId);

    idleTaskId = taskCreat(
        "idleTask",
        255,
        0,
        DEFAULT_STACK_SIZE,
        (FUNCPTR) taskIdle,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        );
    vmxResume((TCB_ID) idleTaskId);

    taskIdCurrent = (TCB_ID) rootTaskId;
    INT_LOCK(level);
    intConnectDefault(TIMER_INTERRUPT_NUM, vmxTickAnnounce, NULL);
    vmxTaskContextLoad();
    INT_UNLOCK(level);
}

/******************************************************************************
 * kernTimeSlice - Enable/Disable round robin task scheduling
 *
 * RETURNS: OK
 */

STATUS kernelTimeSlice(
    unsigned ticks
    )
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

    return OK;
}

