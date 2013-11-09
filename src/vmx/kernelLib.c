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
#include <arch/vmxArchLib.h>
#include <arch/taskArchLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <util/qPriBmpLib.h>
#include <util/qFifoLib.h>
#include <vmx/taskLib.h>
#include <vmx/workQLib.h>
#include <vmx/vmxLib.h>
#include <vmx/memPartLib.h>
#include <vmx/kernelLib.h>
#include <vmx/private/kernelLibP.h>

#define TCB_SIZE                ((unsigned)STACK_ROUND_UP(sizeof(TCB)))
#define MEM_BLOCK_HEADER_SIZE   ((unsigned)STACK_ROUND_UP(sizeof(BLOCK_HEADER)))
#define MEM_FREE_BLOCK_SIZE     ((unsigned)STACK_ROUND_UP(sizeof(FREE_BLOCK)))
#define MEM_BASE_BLOCK_SIZE     (MEM_BLOCK_HEADER_SIZE+MEM_FREE_BLOCK_SIZE)
#define MEM_END_BLOCK_SIZE      (MEM_BLOCK_HEADER_SIZE)
#define MEM_TOTAL_BLOCK_SIZE    ((2*MEM_BLOCK_HEADER_SIZE)+MEM_FREE_BLOCK_SIZE)

/* Globals */
BOOL      kernelState       = FALSE;
BOOL      roundRobinOn      = FALSE;
unsigned  roundRobinSlice   = 0;

char     *pRootMemStart     = NULL;
unsigned  rootMemNBytes     = 0;
int       rootTaskId        = 0;

char     *pExcStackBase     = NULL;
char     *pExcStackEnd      = NULL;

TCB_ID    taskIdCurrent     = NULL;
Q_HEAD    activeQHead       = {NULL, 0, 0 ,NULL};
Q_HEAD    tickQHead;
Q_HEAD    readyQHead;

/* Locals */
#ifdef INCLUDE_CONSTANT_RDY_Q
LOCAL      DL_LIST kernReadyLst[256];
LOCAL      unsigned kernReadyBmp[8];
#endif
LOCAL char kernelVer[]   = "vmx pre-alpha";

/******************************************************************************
 * kernelInit - Initialize kernel
 *
 * RETURNS: N/A
 */

void kernelInit(
    FUNCPTR rootFunc,
    unsigned rootMemSize,
    char *pMemPoolStart,
    char *pMemPoolEnd,
    unsigned excStackSize,
    int lockOutLevel
    )
{
    union
    {
        char align[8];
        TCB initTcb;
    } tcbAligned;

    TCB_ID tcbId;
    unsigned rootStackSize, memPoolSize;
    char *pRootStackBase;
    int level;

    /* Align input parameters */
    pMemPoolStart = (char *) STACK_ROUND_UP(pMemPoolStart);
    pMemPoolEnd = (char *) STACK_ROUND_UP(pMemPoolEnd);
    excStackSize = STACK_ROUND_UP(excStackSize);

    /* Setup global root task memory byte count */
    rootMemNBytes = STACK_ROUND_UP(rootMemSize);

    /* Setup interrupt lock level */
    intLockLevelSet(lockOutLevel);

    /* Initialize variables */
    kernelState     = FALSE;
    roundRobinOn    = FALSE;
    roundRobinSlice = 0;

#if (_STACK_DIR == _STACK_GROWS_DOWN)

    /* Setup interrupt stack at bottom of memory pool */
    pExcStackBase = pMemPoolStart + excStackSize;
    pExcStackEnd  = pMemPoolStart;

    /* Fill stack with 0xee */
    memset(pExcStackEnd, 0xee, excStackSize);

    /* Memory will start after interrupt stack */
    pMemPoolStart = pExcStackBase;

#else /* _STACK_DIR == _STACK_GROWS_UP */

    /* Setup interrupt stack at bottom of memory pool */
    pExcStackBase = pMemPoolStart;
    pExcStackEnd = pMemPoolStart + excStackSize;

    /* Fill stack with 0xee */
    memset(pExcStackBase, 0xee, excStackSize);

    /* Memory will start after interrupt stack */
    pMemPoolStart = pExcStackEnd;

#endif /* _STACK_DIR */

    /* Set stack exception stack */
    intStackSet(pExcStackBase);

    /*
     * The root task stack and TCB will be located at the end of the memory pool
     * Where the TCB will be just above the stack.
     * One memory block must be left alone at the end and at the beginning
     * of the memory pool.
     */
    rootStackSize = rootMemNBytes - TCB_SIZE - MEM_TOTAL_BLOCK_SIZE;
    pRootMemStart = pMemPoolEnd - rootMemNBytes;

#if (_STACK_DIR == _STACK_GROWS_DOWN)

    pRootStackBase = pRootMemStart + rootStackSize + MEM_BASE_BLOCK_SIZE;
    tcbId = (TCB_ID) pRootStackBase;

#else /* _STACK_DIR == _STACK_GROWS_UP */

  tcbId = (TCB_ID) (pRootMemStart + MEM_BASE_BLOCK_SIZE);
  pRootStackBase = pRootMemStart + TCB_SIZE + MEM_BASE_BLOCK_SIZE;

#endif /* _STACK_DIR */

    /*
     * Initialize the root task with taskIdCurrent as NULL.
     * Since the kernel ready queue will be empty no task
     * switch will occur.
     */

    taskIdCurrent = NULL;
    memset(&tcbAligned, 0, sizeof(TCB));

    /* Memory pool will lose the memory which is reserved for the root task */
    memPoolSize = (unsigned) ((int) pRootMemStart - (int) pMemPoolStart);

    /* Initialize the root task */
    taskInit(
        tcbId,                                  /* TCB */
        "tRootTask",                            /* Name */
        0,                                      /* Priority */
        /* TASK_OPTIONS_DEALLOC_STACK | */
        TASK_OPTIONS_UNBREAKABLE,               /* Options */
        pRootStackBase,                         /* Stack base */
        (unsigned) rootStackSize,               /* Stack size */
        (FUNCPTR) rootFunc,                     /* Func */
        (ARG) pMemPoolStart,                    /* arg0 */
        (ARG) memPoolSize,                      /* arg1 */
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0
        );

    /* Store root tasks id */
    rootTaskId = (int) tcbId;

    /* Temp storage for taskIdCurrent, later it points to next ready task */
    taskIdCurrent = &tcbAligned.initTcb;

    /* Start the root task */
    taskActivate(rootTaskId);
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

/******************************************************************************
 * kernelVersion - Return a string with kernel version
 *
 * RETURNS: N/A
 */

char* kernelVersion(
    void
    )
{
    return kernelVer;
}

