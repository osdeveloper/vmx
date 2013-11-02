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
BOOL kernelInitialized   = FALSE;
BOOL kernelState         = FALSE;
BOOL roundRobinOn        = FALSE;
unsigned roundRobinSlice = 0;

char  *excStackBase      = NULL;
char  *excStackEnd       = NULL;

TCB_ID taskIdCurrent     = NULL;
Q_HEAD kernActiveQ       = {NULL, 0, 0 ,NULL};
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

void kernelInit(
    FUNCPTR rootFunc,
    char *pMemPoolStart,
    char *pMemPoolEnd,
    unsigned excStackSize
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


    /* Align input parameters */
    pMemPoolStart = (char *) STACK_ROUND_UP(pMemPoolStart);
    pMemPoolEnd = (char *) STACK_ROUND_UP(pMemPoolEnd);
    excStackSize = STACK_ROUND_UP(excStackSize);

    /* Initialize variables */
    kernelState     = FALSE;
    roundRobinOn    = FALSE;
    roundRobinSlice = 0;
#if (_STACK_DIR == _STACK_GROWS_DOWN)

    /* Setup interrupt stack at bottom of memory pool */
    excStackBase = pMemPoolStart + excStackSize;
    excStackEnd  = pMemPoolStart;

    /* Fill stack with 0xee */
    //memset(pKernExcStkEnd, 0xee, excStackSize);

    /* Memory will start after interrupt stack */
    pMemPoolStart = excStackBase;

#else /* _STACK_DIR == _STACK_GROWS_UP */

    /* Setup interrupt stack at bottom of memory pool */
    excStackBase = pMemPoolStart;
    excStackkEnd = pMemPoolStart + excStackSize;

    /* Fill stack with 0xee */
    //memset(excStackBase, 0xee, excStackSize);

    /* Memory will start after interrupt stack */
    pMemPoolStart = excStackEnd;

#endif /* _STACK_DIR */

    /* Create and start root task */
    rootTaskId = taskCreat(
        "rootTask",
        0,
        0,
        DEFAULT_STACK_SIZE,
        rootFunc,
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
 * kernelTimeSlice - Enable/Disable round robin task scheduling
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
        roundRobinOn = FALSE;
    }
    else
    {
        roundRobinSlice = ticks;
        roundRobinOn = TRUE;
    }

    return OK;
}

