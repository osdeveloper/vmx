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

/* vmxLib.c - Task scheduling library */

#include <stdlib.h>
#include <vmx.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <util/qFifoLib.h>
#include <vmx/logLib.h>
#include <vmx/kernLib.h>
#include <vmx/taskLib.h>
#include <vmx/vmxLib.h>

/* Imports */
IMPORT void taskRetValueSet(TCB_ID pTcb, int val);
IMPORT BOOL kernState;
IMPORT BOOL kernRoundRobin;
IMPORT unsigned kernRoundRobinTimeSlice;
IMPORT TCB_ID kernCurrTaskId;		/* Running task */
IMPORT Q_HEAD kernActiveQ;		/* Active tasks: TCB.activeNode */
IMPORT Q_HEAD kernTickQ;		/* Delayed tasks: TCB.tickNode */
IMPORT Q_HEAD kernReadyQ;		/* Ready tasks: TCB.qNode */
IMPORT volatile unsigned kernTicks;
IMPORT volatile unsigned kernAbsTicks;
IMPORT TCB_ID kernCurrTaskId;

/******************************************************************************
* vmxSpawn - Add a new task, initiallly in suspended state
*
* RETURNS: N/A
******************************************************************************/

void vmxSpawn(TCB_ID pTcb)
{
  logString("vmxSpawn() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Activate task */
  Q_PUT(&kernActiveQ, &pTcb->activeNode, FIFO_KEY_TAIL);
}

/******************************************************************************
* vmxDelete - Remove a task
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS vmxDelete(TCB_ID pTcb)
{
  BOOL status = OK;

  logString("vmxDelete() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* If task is ready, remove it from ready queue */
  if (pTcb->status == TASK_READY)
  {
    Q_REMOVE(&kernReadyQ, pTcb);
  }
  else
  {

    /* If task peding flag is set, remove it from pending queue */
    if (pTcb->status & TASK_PEND)
      status = vmxPendQRemove(pTcb);

    /* If task delay flag is set, remove it from tick queue */
    if (pTcb->status & TASK_DELAY)
      Q_REMOVE(&kernTickQ, &pTcb->tickNode);

  }

  /* Deactivate task */
  Q_REMOVE(&kernActiveQ, &pTcb->activeNode);
  pTcb->status = TASK_DEAD;

  return(status);
}

/******************************************************************************
* vmxSuspend - Suspend a task
*
* RETURNS: N/A
******************************************************************************/

void vmxSuspend(TCB_ID pTcb)
{
  logString("vmxSuspend() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* If task is ready, remove it from ready queue */
  if (pTcb->status == TASK_READY)
    Q_REMOVE(&kernReadyQ, pTcb);

  /* Set suspended flag */
  pTcb->status |= TASK_SUSPEND;
}

/******************************************************************************
* vmxResume - Resume a suspended task
*
* RETURNS: N/A
******************************************************************************/

void vmxResume(TCB_ID pTcb)
{
  logString("vmxResume() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* If task is suspended, put it on ready queue */
  if (pTcb->status == TASK_SUSPEND)
    Q_PUT(&kernReadyQ, pTcb, pTcb->priority);

  /* Reset suspended flag */
  pTcb->status &= ~TASK_SUSPEND;
}

/******************************************************************************
* vmxTickAnnounce - Make kernel time pass
*
* RETURNS: N/A
******************************************************************************/

void vmxTickAnnounce(void)
{
  Q_NODE *pNode;
  TCB_ID pTcb;
  STATUS status;

  logString("vmxTickAnnounce() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Increase time counters */
  kernTicks++;
  kernAbsTicks++;

  /* Advance time queue */
  Q_ADVANCE(&kernTickQ);

  while ((pNode = (Q_NODE *) Q_EXPIRED(&kernTickQ)) != NULL)
  {
    pTcb = (TCB_ID) ((int)pNode - OFFSET(TCB, tickNode));

    logStringAndInteger("A task timed out",
                         pTcb->id,
                         LOG_VMX_LIB,
                         LOG_LEVEL_INFO);

    /* Check class */
    if (pTcb->objCore.pObjClass == taskClassId)
    {
      pTcb->status &= ~TASK_DELAY;		/* Clear delay flag */

      if (pTcb->status == TASK_READY)		/* Timeout */
      {
        logStringAndInteger("Task was ready",
                            pTcb->id,
                            LOG_VMX_LIB,
                            LOG_LEVEL_INFO);

        taskRetValueSet(pTcb, OK);
      }
      else if (pTcb->status & TASK_PEND)
      {

        logStringAndInteger("Task was pending",
                            pTcb->id,
                            LOG_VMX_LIB,
                            LOG_LEVEL_INFO);

        /* Remove from pendig queue */
        status = vmxPendQRemove(pTcb);

        logStringAndInteger("Removed pending task with status",
                            status,
                            LOG_VMX_LIB,
                            LOG_LEVEL_INFO);

	/* Check return status */
	switch(status)
        {
          case ERROR :
	    taskRetValueSet(pTcb, ERROR);
	    pTcb->errorStatus = ERROR;
	  break;

	  default:
	    taskRetValueSet(pTcb, OK);
	  break;
        }

      }

      logStringAndInteger("Putting task on ready queue",
                           pTcb->id,
                           LOG_VMX_LIB,
                           LOG_LEVEL_INFO);

      /* If task is ready now, add it to ready queue */
      Q_PUT(&kernReadyQ, pTcb, pTcb->priority);
    }
  }

  /* Perform periodic task switching in round robin mode */
  if ((kernRoundRobin) &&
      (kernCurrTaskId->lockCount == 0) &&
      (kernCurrTaskId->status == TASK_READY) &&
      (++kernCurrTaskId->timeSlice >= kernRoundRobinTimeSlice))
  {
    kernCurrTaskId->timeSlice = 0;
    Q_REMOVE(&kernReadyQ, kernCurrTaskId);
    Q_PUT(&kernReadyQ, kernCurrTaskId, kernCurrTaskId->priority);
  }
}

/******************************************************************************
* vmxDelay - Put a task to sleep
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS vmxDelay(unsigned timeout)
{
  logString("vmxDelay() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Remove from running tasks queue */
  Q_REMOVE(&kernReadyQ, kernCurrTaskId);

  /* Will kernel timer overflow ? */
  if ((unsigned) (kernTicks + timeout) < kernTicks)
  {
    Q_OFFSET(&kernTickQ, ~kernTicks + 1);
    kernTicks = 0;
  }

  /* Put task on delay queue */
  Q_PUT(&kernTickQ, &kernCurrTaskId->tickNode, timeout + kernTicks);

  /* Update status to delayed */
  kernCurrTaskId->status |= TASK_DELAY;

  return(OK);
}

/******************************************************************************
* vmxUndelay - Wake up a sleeping task
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS vmxUndelay(TCB_ID pTcb)
{
  logString("vmxUndelay() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* If not in delayed mode, just return */
  if ((pTcb->status & TASK_DELAY) == 0)
    return(OK);

  /* Remove delay flag */
  pTcb->status &= ~TASK_DELAY;

  /* Remove from tick queue */
  Q_REMOVE(&kernTickQ, &pTcb->priority);
  pTcb->errorStatus = TASK_UNDELAYED;

  /* Put on ready queue */
  if (pTcb->status == TASK_READY)
    Q_PUT(&kernReadyQ, pTcb, pTcb->priority);

  return(OK);
}

/******************************************************************************
* vmxPriorityNormalSet - Set normal task priority
*
* RETURNS: N/A
******************************************************************************/

void vmxPriorityNormalSet(TCB_ID pTcb, unsigned priority)
{
  logString("vmxPriorityNormalSet() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  pTcb->priorityNormal = priority;

  /* Update task priority */
  vmxPrioritySet(pTcb, priority);
}

/******************************************************************************
* vmxPrioritySet - Set task priority
*
* RETURNS: N/A
******************************************************************************/

void vmxPrioritySet(TCB_ID pTcb, unsigned priority)
{
  logString("vmxPrioritySet() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Check mutex and current priority */
  if ( (pTcb->priorityMutexCount == 0) && (pTcb->priority < priority) )
  {
    /* Update priotity */
    pTcb->priority = priority;

    /* Update correct in queue */
    if (pTcb->status == TASK_READY)
      Q_MOVE(&kernReadyQ, pTcb, priority);
    else if (pTcb->status & TASK_PEND)
      Q_MOVE(pTcb->pPendQ, pTcb, priority);

    return;
  }

  /* Priority was to low */
  while (pTcb->priority > priority)
  {

    /* Update priority */
    pTcb->priority = priority;

    /* Update in correct queue */
    if (pTcb->status == TASK_READY)
    {
      Q_MOVE(&kernReadyQ, pTcb, priority);
    }
    else if (pTcb->status & TASK_PEND)
    {
      Q_MOVE(pTcb->pPendQ, pTcb, priority);

      if (pTcb->priorityMutex != NULL)
        pTcb = SEM_OWNER_GET(pTcb->priorityMutex);
    }
  }
}

/******************************************************************************
* vmxPendQGet - Take next task on pend queue
*
* RETURNS: N/A
******************************************************************************/

void vmxPendQGet(Q_HEAD *pQHead)
{
  logString("vmxPendQGet() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Get task on queue */
  TCB_ID pTcb = (TCB_ID) Q_GET(pQHead);

  pTcb->status &= ~TASK_PEND;	/* Clear pending flag */
  taskRetValueSet(pTcb, OK);	/* Set return value */

  if (pTcb->status & TASK_DELAY)
  {
    logStringAndInteger("Removing task from time queue",
                        pTcb->id,
                        LOG_VMX_LIB,
                        LOG_LEVEL_INFO);

    pTcb->status &= ~TASK_DELAY;		/* Clear delay flag */
    Q_REMOVE(&kernTickQ, &pTcb->tickNode);	/* Remove from queue */
  }

  /* Check if task is ready */
  if (pTcb->status == TASK_READY)
  {
    logStringAndInteger("Putting task on ready queue",
                        pTcb->id,
                        LOG_VMX_LIB,
                        LOG_LEVEL_INFO);
    Q_PUT(&kernReadyQ, pTcb, pTcb->priority);
  }
}

/******************************************************************************
* vmxReadyQPut - Put task on ready queue
*
* RETURNS: N/A
******************************************************************************/

void vmxReadyQPut(TCB_ID pTcb)
{
  logString("vmxReadyQPut() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  pTcb->status &= ~TASK_PEND;	/* Clear pending flag */
  taskRetValueSet(pTcb, OK);	/* Set return value */

  if (pTcb->status & TASK_DELAY)
  {
    pTcb->status &= ~TASK_DELAY;		/* Clear delay flag */
    Q_REMOVE(&kernTickQ, &pTcb->tickNode);	/* Remove from queue */
  }

  /* Check if task is ready */
  if (pTcb->status == TASK_READY)
    Q_PUT(&kernReadyQ, pTcb, pTcb->priority);
}

/******************************************************************************
* vmxReadyQRemove - Remove current pending task from ready queue
*
* RETURNS: N/A
******************************************************************************/

void vmxReadyQRemove(Q_HEAD *pQHead, unsigned timeout)
{
  logString("vmxReadyQRemove() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Remove from ready queue */
  Q_REMOVE(&kernReadyQ, kernCurrTaskId);

  /* Update status */
  kernCurrTaskId->status |= TASK_PEND;

  /* Set task pendig queue */
  kernCurrTaskId->pPendQ = pQHead;

  if (timeout != WAIT_FOREVER)
  {
    /* Check timer overflow */
    if ((unsigned)(kernTicks + timeout) < kernTicks)
    {
      Q_OFFSET(&kernTickQ, ~kernTicks + 1);
      kernTicks = 0;
    }

    /* Put on tick queue */
    Q_PUT(&kernTickQ, &kernCurrTaskId->tickNode, timeout + kernTicks);
    kernCurrTaskId->status |= TASK_DELAY;
  }
}

/******************************************************************************
* vmxPendQFlush - Flush all tasks on pending queue to make them ready
*
* RETURNS: N/A
******************************************************************************/

void vmxPendQFlush(Q_HEAD *pQHead)
{
  TCB_ID pTcb;

  logString("vmxPendQFlush() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Get all tasks */
  while ((pTcb = (TCB_ID) Q_GET(pQHead)) != NULL)
  {
    pTcb->status &= ~TASK_PEND;	/* Clear pending flag */
    taskRetValueSet(pTcb, OK);	/* Set return value */

    if (pTcb->status & TASK_DELAY)
    {
      pTcb->status &= ~TASK_DELAY;		/* Clear delay flag */
      Q_REMOVE(&kernTickQ, &pTcb->tickNode);	/* Remove from queue */
    }

    /* Check if task is ready */
    if (pTcb->status == TASK_READY)
      Q_PUT(&kernReadyQ, pTcb, pTcb->priority);
  }
}

/******************************************************************************
* vmxPendQPut - Put a task on pending queue
*
* RETURNS: OK, or ERROR
******************************************************************************/

STATUS vmxPendQPut(Q_HEAD *pQHead, unsigned timeout)
{
  logString("vmxPendQPut() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  logStringAndInteger("Removing task from ready queue",
                      kernCurrTaskId->id,
                      LOG_VMX_LIB,
                      LOG_LEVEL_INFO);

  /* Remove from ready queue */
  Q_REMOVE(&kernReadyQ, kernCurrTaskId);

  /* Update status */
  kernCurrTaskId->pPendQ = pQHead;

  logStringAndInteger("Put task on semaphore pending queue",
                      kernCurrTaskId->id,
                      LOG_VMX_LIB,
                      LOG_LEVEL_INFO);

  /* Put on task pendig queue */
  Q_PUT(pQHead, kernCurrTaskId, kernCurrTaskId->priority);

  if (timeout != WAIT_FOREVER)
  {
    /* Check timer overflow */
    if ((unsigned)(kernTicks + timeout) < kernTicks)
    {
      Q_OFFSET(&kernTickQ, ~kernTicks + 1);
      kernTicks = 0;
    }

    logStringAndInteger("Put task on time queue",
                        kernCurrTaskId->id,
                        LOG_VMX_LIB,
                        LOG_LEVEL_INFO);

    /* Put on tick queue */
    Q_PUT(&kernTickQ, &kernCurrTaskId->tickNode, timeout + kernTicks);
    kernCurrTaskId->status |= TASK_DELAY;
  }

  return(OK);
}

/******************************************************************************
* vmxPendQRemove - Remove a task from pending queue
*
* RETURNS: OK, or ERROR
******************************************************************************/

STATUS vmxPendQRemove(TCB_ID pTcb)
{
  STATUS status;

  logString("vmxPendQRemove() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  status = (STATUS) Q_REMOVE(pTcb->pPendQ, pTcb);

  pTcb->status &= ~TASK_PEND;	/* Clear pending flag */
  pTcb->priorityMutex = NULL;

  if (pTcb->status & TASK_DELAY)
  {
    pTcb->status &= ~TASK_DELAY;		/* Clear delay flag */
    Q_REMOVE(&kernTickQ, &pTcb->tickNode);	/* Remove from queue */
  }

  return(status);
}

/******************************************************************************
* vmxPendQTerminate - Flush all tasks on pending queue and make the ready
*
* RETURNS: N/A
******************************************************************************/

void vmxPendQTerminate(Q_HEAD *pQHead)
{
  TCB_ID pTcb;

  logString("vmxPendQTerminate() called:",
            LOG_VMX_LIB,
            LOG_LEVEL_CALLS);

  /* Get all tasks */
  while ((pTcb = (TCB_ID) Q_GET(pQHead)) != NULL)
  {
    pTcb->status &= ~TASK_PEND;	/* Clear pending flag */
    pTcb->priorityMutex = NULL;
    pTcb->errorStatus = TASK_DELETED;

    /* Return error */
    taskRetValueSet(pTcb, ERROR);

    if (pTcb->status & TASK_DELAY)
    {
      pTcb->status &= ~TASK_DELAY;		/* Clear delay flag */
      Q_REMOVE(&kernTickQ, &pTcb->tickNode);	/* Remove from queue */
    }

    /* Check if task is ready */
    if (pTcb->status == TASK_READY)
      Q_PUT(&kernReadyQ, pTcb, pTcb->priority);
  }
}

