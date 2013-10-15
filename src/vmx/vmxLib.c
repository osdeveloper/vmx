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

/* vmxLib.c - Task scheduling library */

#define NO_SWAPLIB
#define NO_MUTEX
#define NO_WDLIB
#define NO_UNPENDHDL

#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <util/qFifoLib.h>
#include <vmx/private/kernLibP.h>
#include <vmx/kernLib.h>
#include <vmx/kernQLib.h>
#include <vmx/taskLib.h>
#ifndef NO_WDLIB
#include <vmx/wdLib.h>
#endif
#include <vmx/vmxLib.h>
#include <vmx/vmxLib.h>

/******************************************************************************
 * vmxSpawn - Add a new task, initiallly in suspended state
 *
 * RETURNS: N/A
 */

void vmxSpawn(
    TCB_ID tcbId
    )
{
    /* Activate task */
    Q_PUT(&kernActiveQ, &tcbId->activeNode, FIFO_KEY_TAIL);
}

/******************************************************************************
 * vmxDelete - Remove a task
 *
 * RETURNS: OK or ERROR
 */

STATUS vmxDelete(
    TCB_ID tcbId
    )
{
    u_int16_t mask;
    int i;
    BOOL status = OK;

    /* If task is ready, remove it from ready queue */
    if (tcbId->status == TASK_READY)
    {
        Q_REMOVE(&kernReadyQ, tcbId);
    }
    else
    {
        /* If task peding flag is set, remove it from pending queue */
        if (tcbId->status & TASK_PEND)
        {
            status = vmxPendQRemove(tcbId);
        }

        /* If task delay flag is set, remove it from tick queue */
        if (tcbId->status & TASK_DELAY)
        {
            Q_REMOVE(&kernTickQ, &tcbId->tickNode);
        }
    }

#ifndef NO_SWAPLIB
    /* Disconnect to all swap hooks */
    for (i = 0, mask = tcbId->swapInMask; mask != 0; i++, mask = mask << 1)
    {
        if (mask & 0x8000)
        {
            taskSwapReference[i]--;
        }
    }

    for (i = 0, mask = tcbId->swapOutMask; mask != 0; i++, mask = mask << 1)
    {
        if (mask & 0x8000)
        {
            taskSwapReference[i]--;
        }
    }
#endif

    /* Deactivate task */
    Q_REMOVE(&kernActiveQ, &tcbId->activeNode);
    tcbId->status = TASK_DEAD;

    return status;
}

/******************************************************************************
 * vmxSuspend - Suspend a task
 *
 * RETURNS: N/A
 */

void vmxSuspend(
    TCB_ID tcbId
    )
{
    /* If task is ready, remove it from ready queue */
    if (tcbId->status == TASK_READY)
    {
        Q_REMOVE(&kernReadyQ, tcbId);
    }

    /* Set suspended flag */
    tcbId->status |= TASK_SUSPEND;
}

/******************************************************************************
 * vmxResume - Resume a suspended task
 *
 * RETURNS: N/A
 */

void vmxResume(
    TCB_ID tcbId
    )
{
    /* If task is suspended, put it on ready queue */
    if (tcbId->status == TASK_SUSPEND)
    {
        Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
    }

    /* Reset suspended flag */
    tcbId->status &= ~TASK_SUSPEND;
}

/******************************************************************************
 * vmxTickAnnounce - Make kernel time pass
 *
 * RETURNS: N/A
 */

void vmxTickAnnounce(
    void
    )
{
    Q_NODE *pNode;
    TCB_ID tcbId;
#ifndef NO_WDLIB
    WDOG_ID wdId;
    FUNCPTR wdFunc;
    ARG wdArg;
#endif
    STATUS status;

    /* Increase time counters */
    kernTicks++;
    kernAbsTicks++;

    /* Advance time queue */
    Q_ADVANCE(&kernTickQ);

    while ((pNode = (Q_NODE *) Q_EXPIRED(&kernTickQ)) != NULL)
    {
        tcbId = (TCB_ID) ((int) pNode - OFFSET(TCB, tickNode));

        /* If this is a task */
        if (tcbId->objCore.pObjClass == taskClassId)
        {
            tcbId->status &= ~TASK_DELAY;          /* Clear delay flag */
            if (tcbId->status == TASK_READY)       /* Timeout */
            {
                taskRetValueSet(tcbId, OK);
            }
            else if (tcbId->status & TASK_PEND)
            {
                /* Remove from pend queue */
                status = vmxPendQRemove (tcbId);   /* Remove from pend Q. */
                if (status == ERROR)               /* If returned ERROR */
                {
                    tcbId->errorStatus = ERROR;    /* set task [errorStatus]. */
                }

                taskRetValueSet (tcbId, ERROR);    /* Task timed-out. */
                tcbId->errno = S_objLib_TIMEOUT;   /* Set errno */

#ifndef NO_UNPENDHDL
                /*
                 * If the task removed from the pend queue has a custom
                 * 'unpend' handler, call it.
                 */
                if (tcbId->objUnpendHandler != NULL)
                {
                    tcbId->objUnpendHandler (tcbId, VMX_TIMEOUT);
                }
#endif
            }

            /* If task is ready now, add it to ready queue */
            if (tcbId->status == TASK_READY)
            {
                Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
            }
        }
#ifndef NO_WDLIB
        else {
            /* Convert to watchdog id */
            wdId = (WDOG_ID) ((int) pNode - OFFSET(WDOG, tickNode) );

            /* Process */
            wdId->status = WDOG_OUT_OF_Q;
            wdFunc = wdId->wdFunc;
            wdArg = wdId->wdArg;

            /* Call watchdog function */
            intCnt++;
            if (wdId->dfrStartCount == 0)
            {
                (*wdFunc)(wdArg);
            }
            intCnt--;

            /* Do kernel work */
            kernQDoWork();
        }
#endif
    }

    /* Perform periodic task switching in round robin mode */
    if ((kernRoundRobin) &&
        (taskIdCurrent->lockCount == 0) &&
        (taskIdCurrent->status == TASK_READY) &&
        (++taskIdCurrent->timeSlice >= kernRoundRobinTimeSlice))
    {
        taskIdCurrent->timeSlice = 0;
        Q_REMOVE(&kernReadyQ, taskIdCurrent);
        Q_PUT(&kernReadyQ, taskIdCurrent, taskIdCurrent->priority);
    }
}

/******************************************************************************
 * vmxDelay - Put a task to sleep
 *
 * RETURNS: OK
 */

STATUS vmxDelay(
    unsigned timeout
    )
{
    /* Remove from running tasks queue */
    Q_REMOVE(&kernReadyQ, taskIdCurrent);

    /* Will kernel timer overflow ? */
    if ((unsigned) (kernTicks + timeout) < kernTicks)
    {
        Q_OFFSET(&kernTickQ, ~kernTicks + 1);
        kernTicks = 0;
    }

    /* Put task on delay queue */
    Q_PUT(&kernTickQ, &taskIdCurrent->tickNode, timeout + kernTicks);

    /* Update status to delayed */
    taskIdCurrent->status |= TASK_DELAY;

    return OK;
}

/******************************************************************************
 * vmxUndelay - Wake up a sleeping task
 *
 * RETURNS: OK
 */

STATUS vmxUndelay(
    TCB_ID tcbId
    )
{
    STATUS status;

    /* If not in delayed mode, just return */
    if ((tcbId->status & TASK_DELAY) == 0)
    {
        status = OK;
    }
    else
    {
        /* Remove delay flag */
        tcbId->status &= ~TASK_DELAY;

        /* Remove from tick queue */
        Q_REMOVE(&kernTickQ, &tcbId->priority);
        tcbId->errorStatus = TASK_UNDELAYED;

        /* Put on ready queue */
        if (tcbId->status == TASK_READY)
        {
            Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
        }

        status = OK;
    }

    return status;
}

/******************************************************************************
 * vmxPrioritySet - Set task priority
 *
 * RETURNS: N/A
 */

void vmxPrioritySet(
    TCB_ID tcbId,
    unsigned priority
    )
{
    /* Update priotity */
    tcbId->priority = priority;

    /* Update correct in queue */
    if (tcbId->status == TASK_READY)
    {
        Q_MOVE(&kernReadyQ, tcbId, priority);
    }
    else if (tcbId->status & TASK_PEND)
    {
        Q_MOVE(tcbId->pPendQ, tcbId, priority);
    }
}

/******************************************************************************
 * vmxSemDelete - Delete a semaphore
 *
 * RETURNS: N/A
 */

void vmxSemDelete(
    SEM_ID semId
    )
{
    TCB_ID pOwner;

    /* End pending queue */
    vmxPendQTerminate(&semId->qHead);

    /* Get owner */
    pOwner = SEM_OWNER_GET(semId);

#ifndef NO_MUTEX
    /* Special for mutex */
    if (semId->semType == SEM_TYPE_MUTEX)
    {
        /* Has it been taken */
        if (pOwner != NULL)
        {
            /* Valid task pending */
            if (OBJ_VERIFY(pOwner, taskClassId) == OK)
            {
                /* Is it delete safe */
                if ((semId->options & SEM_DELETE_SAFE) &&
                    (--pOwner->safeCount == 0) &&
                    (Q_FIRST(&pOwner->safetyQ) != NULL))
                {
                    vmxPendQFlush(&pOwner->safetyQ);
                }
            }
        }
    }
#endif
}

/******************************************************************************
 * vmxPendQGet - Take next task on pend queue
 *
 * RETURNS: N/A
 */

void vmxPendQGet(
    Q_HEAD *pQHead
    )
{
    /* Get task on queue */
    TCB_ID tcbId = (TCB_ID) Q_GET(pQHead);

    tcbId->status &= ~TASK_PEND;   /* Clear pending flag */
    taskRetValueSet(tcbId, OK);    /* Set return value */

    if (tcbId->status & TASK_DELAY)
    {
        tcbId->status &= ~TASK_DELAY;                /* Clear delay flag */
        Q_REMOVE(&kernTickQ, &tcbId->tickNode);      /* Remove from queue */
    }

    /* Check if task is ready */
    if (tcbId->status == TASK_READY)
    {
        Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
    }
}

/******************************************************************************
 * vmxReadyQPut - Put task on ready queue
 *
 * RETURNS: N/A
 */

void vmxReadyQPut(
    TCB_ID tcbId
    )
{
    tcbId->status &= ~TASK_PEND;   /* Clear pending flag */
    taskRetValueSet(tcbId, OK);    /* Set return value */

    if (tcbId->status & TASK_DELAY)
    {
        tcbId->status &= ~TASK_DELAY;                /* Clear delay flag */
        Q_REMOVE(&kernTickQ, &tcbId->tickNode);      /* Remove from queue */
    }

    /* Check if task is ready */
    if (tcbId->status == TASK_READY)
    {
        Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
    }
}

/******************************************************************************
 * vmxReadyQRemove - Remove current pending task from ready queue
 *
 * RETURNS: N/A
 */

void vmxReadyQRemove(
    Q_HEAD *pQHead,
    unsigned timeout
    )
{
    /* Remove from ready queue */
    Q_REMOVE(&kernReadyQ, taskIdCurrent);

    /* Update status */
    taskIdCurrent->status |= TASK_PEND;

    /* Set task pending queue */
    taskIdCurrent->pPendQ = pQHead;

    if (timeout != WAIT_FOREVER)
    {
        /* Check timer overflow */
        if ((unsigned)(kernTicks + timeout) < kernTicks)
        {
            Q_OFFSET(&kernTickQ, ~kernTicks + 1);
            kernTicks = 0;
        }

        /* Put on tick queue */
        Q_PUT(&kernTickQ, &taskIdCurrent->tickNode, timeout + kernTicks);
        taskIdCurrent->status |= TASK_DELAY;
    }
}

/******************************************************************************
 * vmxPendQFlush - Flush all tasks on pending queue to make them ready
 *
 * RETURNS: N/A
 */

void vmxPendQFlush(
    Q_HEAD *pQHead
    )
{
    TCB_ID tcbId;

    /* Get all tasks */
    while ((tcbId = (TCB_ID) Q_GET(pQHead)) != NULL)
    {
        tcbId->status &= ~TASK_PEND; /* Clear pending flag */
        taskRetValueSet(tcbId, OK);  /* Set return value */

        if (tcbId->status & TASK_DELAY)
        {
            tcbId->status &= ~TASK_DELAY;              /* Clear delay flag */
            Q_REMOVE(&kernTickQ, &tcbId->tickNode);    /* Remove from queue */
        }

        /* Check if task is ready */
        if (tcbId->status == TASK_READY)
        {
            Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
        }
    }
}

/******************************************************************************
 * vmxPendQWithHandlerPut - put a task on pending queue and set unpend handler
 *
 * This routine adds a task wtih a custom 'unpend' handler to the specified
 * pending queue.  In the event that the task times-out, is deleted, or for
 * some other reason does not obtain the resource for which it is pending, it
 * will call the 'unpend' handler.  It is assumed that the parameter <timeout>
 * will never be WAIT_NONE.
 *
 * RETURNS: N/A
 */

void vmxPendQWithHandlerPut(
    Q_HEAD *pQHead,              /* ptr to Q on which to pend */
    unsigned timeout,            /* timeout with which to pend */
    void (*handler)(void*, int), /* custom 'unpend' handler */
    void *pObj,                  /* ptr to object upon which to pend */
    int info                     /* custom info to pass to handler */
    )
{
    Q_REMOVE (&kernReadyQ, taskIdCurrent);   /* Remove from ready queue */

    taskIdCurrent->status |= TASK_PEND;      /* Update status */

    taskIdCurrent->pPendQ = pQHead;          /* Set task pending queue */

#ifndef NO_UNPENDHDL
    taskIdCurrent->objUnpendHandler = handler;
    taskIdCurrent->pObj = pObj;
    taskIdCurrent->objInfo = info;
#endif

    /* Put on task pending queue */
    Q_PUT (pQHead, taskIdCurrent, taskIdCurrent->priority);

    if (timeout != WAIT_FOREVER)
    {
        /* Check timer overflow */
        if ((unsigned) (kernTicks + timeout) < kernTicks)
        {
            Q_OFFSET (&kernTickQ, ~kernTicks + 1);
            kernTicks = 0;
        }

        /* Put on tick queue */
        Q_PUT (&kernTickQ, &taskIdCurrent->tickNode, timeout + kernTicks);
        taskIdCurrent->status |= TASK_DELAY;
    }
}

/******************************************************************************
 * vmxPendQPut - Put a task on pending queue
 *
 * RETURNS: OK, or ERROR
 */

STATUS vmxPendQPut(
    Q_HEAD *pQHead,
    unsigned timeout
    )
{
    STATUS status;

    if (timeout == WAIT_NONE)
    {
        status = ERROR;
    }

    vmxPendQWithHandlerPut (pQHead, timeout, NULL, NULL, 0);
    status = OK;

    return status;
}

/******************************************************************************
 * vmxPendQRemove - Remove a task from pending queue
 *
 * RETURNS: OK, or ERROR
 */

STATUS vmxPendQRemove(
    TCB_ID tcbId
    )
{
    STATUS status;

    status = (STATUS) Q_REMOVE(tcbId->pPendQ, tcbId);
    tcbId->status &= ~TASK_PEND;   /* Clear pending flag */

    if (tcbId->status & TASK_DELAY)
    {
        tcbId->status &= ~TASK_DELAY;                /* Clear delay flag */
        Q_REMOVE(&kernTickQ, &tcbId->tickNode);      /* Remove from queue */
    }

    return status;
}

/******************************************************************************
 * vmxPendQTerminate - Flush all tasks on pending queue and make the ready
 *
 * RETURNS: N/A
 */

void vmxPendQTerminate(
    Q_HEAD *pQHead
    )
{
    TCB_ID tcbId;

    /* Get all tasks */
    while ((tcbId = (TCB_ID) Q_GET(pQHead)) != NULL)
    {
        tcbId->status &= ~TASK_PEND; /* Clear pending flag */
        tcbId->errorStatus = TASK_DELETED;

        /* Return error */
        taskRetValueSet(tcbId, ERROR);

        if (tcbId->status & TASK_DELAY)
        {
            tcbId->status &= ~TASK_DELAY;              /* Clear delay flag */
            Q_REMOVE(&kernTickQ, &tcbId->tickNode);    /* Remove from queue */
        }

        /* Check if task is ready */
        if (tcbId->status == TASK_READY)
        {
            Q_PUT(&kernReadyQ, tcbId, tcbId->priority);
        }
    }
}

#ifndef NO_WDLIB
/******************************************************************************
 * vmxWdStart - Start watchdog timer
 *
 * RETURNS: OK
 */

STATUS vmxWdStart(
    WDOG_ID wdId,
    unsigned timeout
    )
{
    int level;
    int dfrStartCount;

    /* Get defer start count */
    INT_LOCK(level);
    dfrStartCount = --wdId->dfrStartCount;
    INT_UNLOCK(level);

    if (dfrStartCount == 0) {
        /* If overflow */
        if ((unsigned) (kernTicks + timeout) < kernTicks)
        {
            Q_OFFSET(&kernTickQ, ~kernTicks + 1);
            kernTicks = 0;
        }

        if (wdId->status == WDOG_IN_Q)
        {
            Q_MOVE(&kernTickQ, &wdId->tickNode, timeout + kernTicks);
        }
        else
        {
            Q_PUT(&kernTickQ, &wdId->tickNode, timeout + kernTicks);
        }

        /* Mark as in queue */
        wdId->status = WDOG_IN_Q;
  }

  return OK;
}

/******************************************************************************
 * vmxWdCancel - Cancel watchdog timer
 *
 * RETURNS: N/A
 */

void vmxWdCancel(
    WDOG_ID wdId
    )
{

    /* If in queue */
    if (wdId->status == WDOG_IN_Q)
    {
        Q_REMOVE(&kernTickQ, &wdId->tickNode);
        wdId->status = WDOG_OUT_OF_Q;
    }
}
#endif

