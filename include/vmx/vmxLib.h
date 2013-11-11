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

/* vmxLib.h - Task scheduling library */

#ifndef _vmxLib_h
#define _vmxLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* includes */

#include <util/qLib.h>
#include <vmx/taskLib.h>
#include <vmx/wdLib.h>

/* defines */

#define VMX_TIMEOUT   1    /* unpend handler called due to a timeout */

/* imports */

/******************************************************************************
 * vmxSpawn - Add a new task, initiallly in suspended state
 *
 * RETURNS: N/A
 */

void vmxSpawn(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxDelete - Remove a task
 *
 * RETURNS: OK or ERROR
 */

STATUS vmxDelete(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxSuspend - Suspend a task
 *
 * RETURNS: N/A
 */

void vmxSuspend(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxResume - Resume a suspended task
 *
 * RETURNS: N/A
 */

void vmxResume(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxTickAnnounce - Make kernel time pass
 *
 * RETURNS: N/A
 */

void vmxTickAnnounce(
    void
    );

/******************************************************************************
 * vmxDelay - Put a task to sleep
 *
 * RETURNS: OK
 */

STATUS vmxDelay(
    unsigned timeout
    );

/******************************************************************************
 * vmxUndelay - Wake up a sleeping task
 *
 * RETURNS: OK
 */

STATUS vmxUndelay(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxPrioritySet - Set task priority
 *
 * RETURNS: N/A
 */

void vmxPrioritySet(
    TCB_ID tcbId,
    unsigned priority
    );

/******************************************************************************
 * vmxSemDelete - Delete a semaphore
 *
 * RETURNS: N/A
 */

void vmxSemDelete(
    SEM_ID semId
    );

/******************************************************************************
 * vmxPendQGet - Take next task on pend queue
 *
 * RETURNS: N/A
 */

void vmxPendQGet(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * vmxReadyQPut - Put task on ready queue
 *
 * RETURNS: N/A
 */

void vmxReadyQPut(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxReadyQRemove - Remove current pending task from ready queue
 *
 * RETURNS: N/A
 */

void vmxReadyQRemove(
    Q_HEAD *pQHead,
    unsigned timeout
    );

/******************************************************************************
 * vmxPendQFlush - Flush all tasks on pending queue to make them ready
 *
 * RETURNS: N/A
 */

void vmxPendQFlush(
    Q_HEAD *pQHead
    );

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
    );

/******************************************************************************
 * vmxPendQPut - Put a task on pending queue
 *
 * RETURNS: OK, or ERROR
 */

STATUS vmxPendQPut(
    Q_HEAD *pQHead,
    unsigned timeout
    );

/******************************************************************************
 * vmxPendQRemove - Remove a task from pending queue
 *
 * RETURNS: OK, or ERROR
 */

STATUS vmxPendQRemove(
    TCB_ID tcbId
    );

/******************************************************************************
 * vmxPendQTerminate - Flush all tasks on pending queue and make the ready
 *
 * RETURNS: N/A
 */

void vmxPendQTerminate(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * vmxWdStart - Start watchdog timer
 *
 * RETURNS: OK
 */

STATUS vmxWdStart(
    WDOG_ID wdId,
    unsigned timeout
    );

/******************************************************************************
 * vmxWdCancel - Cancel watchdog timer
 *
 * RETURNS: N/A
 */

void vmxWdCancel(
    WDOG_ID wdId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _vmxLib_h */

