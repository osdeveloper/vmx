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

/* taskLib.c - Task handeling library */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/kernArchLib.h>
#include <vmx/logLib.h>
#include <vmx/classLib.h>
#include <vmx/objLib.h>
#include <vmx/memPartLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <vmx/workQLib.h>
#include <vmx/private/kernLibP.h>
#include <vmx/taskLib.h>
#include <vmx/vmxLib.h>
#include <vmx/sigLib.h>

/* Defines */
#define TASK_EXTRA_BYTES       16
/* Locals */
LOCAL OBJ_CLASS taskClass;
LOCAL BOOL taskLibInstalled = FALSE;

/* Globals */
CLASS_ID taskClassId = &taskClass;

/******************************************************************************
 * taskLibInit - Initialize task library
 *
 * RETURNS: OK or ERROR
 */

STATUS taskLibInit(
    void
    )
{
    STATUS status;

    if (taskLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        if (classInit(
                taskClassId,
                STACK_ROUND_UP(sizeof(TCB) + TASK_EXTRA_BYTES),
                OFFSET(TCB, objCore),
                memSysPartId,
                (FUNCPTR) taskCreat,
                (FUNCPTR) taskInit,
                (FUNCPTR) taskDestroy
                ) != OK)
        {
            status = ERROR;
        }
        else
        {
            taskLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * taskSpawn - Create a new task start it
 *
 * RETURNS: taskId or 0
 */

int taskSpawn(
    const char *name,
    unsigned priority,
    int options,
    unsigned stackSize,
    FUNCPTR func,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8,
    ARG arg9
    )
{
    int taskId;

    /* Create task */
    taskId = taskCreat(
               name,
               priority,
               options,
               stackSize,
               func,
               arg0,
               arg1,
               arg2,
               arg3,
               arg4,
               arg5,
               arg6,
               arg7,
               arg8,
               arg9
               );
    if (taskId != 0)
    {
        /* Then start it */
        taskActivate((TCB_ID) taskId);
    }

    return taskId;
}

/******************************************************************************
 * taskCreat - Create a new task without starting it
 *
 * RETURNS: taskId or 0
 */

int taskCreat(
    const char *name,
    unsigned priority,
    int options,
    unsigned stackSize,
    FUNCPTR func,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8,
    ARG arg9
    )
{
    TCB_ID tcbId;
    char *pTaskMem;
    char *pStackBase;

    /* Check if task lib is installed */
    if (taskLibInstalled != TRUE)
    {
        errnoSet(S_taskLib_NOT_INSTALLED);
        tcbId = NULL;
    }
    else
    {
        /* Round up stack size */
        ROUND_UP(stackSize, _STACK_ALIGN_SIZE);

        /* Allocate new TCB plus stack */
        pTaskMem = objAllocPad(
                       taskClassId,
                       (unsigned) stackSize,
                       (void **) NULL
                       );
        if (pTaskMem == NULL)
        {
            tcbId = NULL;
        }
        else
        {
            /* Setup stack vars */
#if (_STACK_DIR == _STACK_GROWS_DOWN)
            pStackBase = pTaskMem + stackSize;
            tcbId = (TCB_ID) pStackBase;
#else /* _STACK_GROWS_UP */
            tcbId = (TCB_ID) (pTaskMem + 16);
            pStackBase = STACK_ROUND_UP(
                             pTaskMem + TASK_EXTRA_BYTES + sizeof(TCB)
                             );
#endif /* _STACK_DIR */

            /* Initialize task */
            if (taskInit(
                    tcbId,
                    name,
                    priority,
                    options,
                    pStackBase,
                    stackSize,
                    func,
                    arg0,
                    arg1,
                    arg2,
                    arg3,
                    arg4,
                    arg5,
                    arg6,
                    arg7,
                    arg8,
                    arg9
                    ) != OK)
            {
                objFree(taskClassId, tcbId);
            }
        }
    }

    return (int) tcbId;
}

/******************************************************************************
 * taskInit - Initialize task
 *
 * RETURNS: OK or ERROR
 */
STATUS taskInit(
    TCB_ID tcbId,
    const char *name,
    unsigned priority,
    int options,
    char *pStackBase,
    unsigned stackSize,
    FUNCPTR func,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8,
    ARG arg9
    )
{
    static unsigned new_id;
    STATUS status;
    ARG args[MAX_TASK_ARGS];

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        /* Check if task lib is installed */
        if (taskLibInstalled != TRUE)
        {
            errnoSet(S_taskLib_NOT_INSTALLED);
            status = ERROR;
        }
        else
        {
            /* Copy args to array */
            args[0] = arg0;
            args[1] = arg1;
            args[2] = arg2;
            args[3] = arg3;
            args[4] = arg4;
            args[5] = arg5;
            args[6] = arg6;
            args[7] = arg7;
            args[8] = arg8;
            args[9] = arg9;

            /* Set unique id */
            tcbId->id = new_id++;

            /* Copy name */
            strcpy(tcbId->name, name);

            /* Set initial status */
            tcbId->status = TASK_SUSPEND;
            tcbId->lockCount = 0;
            tcbId->priority = priority;
            tcbId->options = options;

            /* Time slice counter */
            tcbId->timeSlice = 0;

            /* Pendig queue, used by semaphores */
            tcbId->pPendQ = NULL;

            /* Initialize safety */
            tcbId->safeCount = 0;
            qInit(
                &tcbId->safetyQ,
                qPrioClassId,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );

            /* Task entry point */
            tcbId->entry = func;

            /* Setup error status */
            tcbId->errorStatus = OK;
            tcbId->exitCode = 0;

            /* Setup stack */
            tcbId->stackDepth = stackSize;
            tcbId->pStackBase = pStackBase;
            tcbId->pStackLimit = tcbId->pStackBase + stackSize * _STACK_DIR;

            /* Initialize architecutre depedent stuff */
            taskRegsInit(tcbId, pStackBase);

            /* Push args on task stack */
            taskArgSet(tcbId, pStackBase, args);

            /* Object core */
            objCoreInit(&tcbId->objCore, taskClassId);

            /* Start task */
            vmxSpawn(tcbId);
            status = OK;
        }
    }

    return status;
}

/**************************************************************
* taskDelete - Remove task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskDelete(TCB_ID pTcb)
{
  return (taskDestroy(pTcb, TRUE, WAIT_FOREVER, FALSE));
}

/**************************************************************
* taskDeleteForce - Remove task forcevly
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskDeleteForce(TCB_ID pTcb)
{
  return (taskDestroy(pTcb, TRUE, WAIT_FOREVER, TRUE));
}

/**************************************************************
* taskTerminate - Terminate task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskTerminate(TCB_ID pTcb)
{
  return (taskDestroy(pTcb, FALSE, WAIT_FOREVER, FALSE));
}

/**************************************************************
* taskDestroy - Kill task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskDestroy(TCB_ID pTcb,
                   BOOL freeStack,
                   unsigned timeout,
                   BOOL forceDestroy)
{
  STATUS status;
  int level;
  TCB_ID tcbId;

  logString("taskDestroy() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Null will commit suicide */
  if (pTcb == NULL)
    tcbId = taskIdCurrent;
  else
    tcbId = pTcb;

  /* Check if not NULL */
  if (tcbId == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(tcbId))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Loop here if kernel restarts */
  do {

    /* Lock interrupts */
    INT_LOCK(level);

    /* Check id */
    if (taskIdVerify(tcbId) != OK)
    {
      logString("ERROR - Trying to destroy a non task object",
                LOG_TASK_LIB,
                LOG_LEVEL_ERROR);

      /* Unlock interrupts */
      INT_UNLOCK(level);

      return(ERROR);
    }

    /* Block here for safe and running locked tasks */
    while   ( (tcbId->safeCount > 0) ||
            ( (tcbId->status == TASK_READY) && (tcbId->lockCount > 0) )
          )
    {
      /* Enter kernel mode */
      kernelState = TRUE;

      /* Unlock interrupts */
      INT_UNLOCK(level);

      /* Check if force deletion, or suicide */
      if (forceDestroy || (tcbId == taskIdCurrent))
      {

        /* Remove protections */
        tcbId->safeCount = 0;
        tcbId->lockCount = 0;

        /* Check if flush of safety queue is needed */
        if (Q_FIRST(&tcbId->safetyQ) != NULL)
          vmxPendQFlush(&tcbId->safetyQ);

        /* Exit trough kernel */
        kernExit();
      }
      else
      {
        /* Not forced deletion or suicide */

        /* Put task on safe queue */
        if (vmxPendQPut(&tcbId->safetyQ, timeout) != OK)
        {
          /* Exit trough kernel */
          kernExit();

          logString("ERROR - Trying to destroy a non task object",
                    LOG_TASK_LIB,
                    LOG_LEVEL_ERROR);

          return(ERROR);
        }

      } /* End else forced or suicide */

      /* Exit troug kernel */
      status = kernExit();

      /* Check if unsuccessfull */
      if (status == ERROR)
      {
          logString("ERROR - Kernel error",
                    LOG_TASK_LIB,
                    LOG_LEVEL_ERROR);

          return(ERROR);
      }

      /* Lock interrupts */
      INT_LOCK(level);

      /* Now verify class id again */
      if (taskIdVerify(tcbId) != OK)
      {
        logString("ERROR - Destroy got a non task object",
                  LOG_TASK_LIB,
                  LOG_LEVEL_ERROR);

        /* Unlock interrupts */
        INT_UNLOCK(level);

        return(ERROR);
      }

    } /* End while blocked by safety */

    /* Now only one cadidate is selected for deletion */

    /* Make myself safe */
    taskSafe();

    /* Protet deletion cadidate */
    tcbId->safeCount++;

     /* Check if not suicide */
     if (tcbId != taskIdCurrent)
     {
       /* Enter kernel mode */
       kernelState = TRUE;

       /* Unlock interrupts */
       INT_UNLOCK(level);

       /* Suspend victim */
       vmxSuspend(tcbId);

       /* Exit trough kernel */
       kernExit();
    }
    else
    {
       /* Unlock interrupts */
       INT_UNLOCK(level);
    }

    /* Lock task */
    taskLock();

    /* If dealloc and options dealloc stack */
    if ( freeStack && (tcbId->options & TASK_OPTIONS_DEALLOC_STACK) )
    {
#if     (_STACK_DIR == _STACK_GROWS_DOWN)
      objFree(taskClassId, tcbId->pStackLimit);
#else
      objFree(taskClassId, tbcId - 16);
#endif
    }

    /* Lock interrupts */
    INT_LOCK(level);

    /* Invalidate id */
    objCoreTerminate(&tcbId->objCore);

    /* Enter kernel mode */
    kernelState = TRUE;

    /* Unlock interrupts */
    INT_UNLOCK(level);

    /* Delete task */
    status = vmxDelete(tcbId);

    /* Check if safe quque needs to be flushed */
    if (Q_FIRST(&tcbId->safetyQ) != NULL)
      vmxPendQFlush(&tcbId->safetyQ);

    /* Exit trough kernel */
    kernExit();

    /* Unprotect */
    taskUnlock();
    taskUnsafe();

  /* Try again, kernel restart */
  } while (0);


  return(OK);
}

/**************************************************************
* taskActivate - Activate task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskActivate(TCB_ID pTcb)
{
  logString("taskActivate() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  return(taskResume(pTcb));
}

/**************************************************************
* taskSuspend - Suspend a task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskSuspend(TCB_ID pTcb)
{
  logString("taskSuspend() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(pTcb))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if in kernel mode */
  if (kernelState == TRUE)
  {
    /* Add to kernel queue */
    workQAdd1 ((FUNCPTR) vmxSuspend, (ARG) pTcb);

    return(OK);
  }

  /* Enter kernel mode */
  kernelState = TRUE;

  /* Suspend task */
  vmxSuspend(pTcb);

  /* Exit trough kernel */
  kernExit();

  return(OK);
}

/**************************************************************
* taskResume - Resume task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskResume(TCB_ID pTcb)
{
  logString("taskResume() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(pTcb))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Put on queue if in kernel mode */
  if (kernelState == TRUE)
  {
    /* Put on kernel queue */
    workQAdd1((FUNCPTR) vmxResume, (ARG) pTcb);

    return(OK);
  }

  /* Enter kernel mode */
  kernelState = TRUE;

  /* Resume task */
  vmxResume(pTcb);

  /* Exit kernel mode */
  kernExit();

  return(OK);
}

/**************************************************************
* taskDelay - Put a task to sleep
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskDelay(unsigned timeout)
{
  STATUS status;

  logString("taskDelay() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (taskIdCurrent == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Enter kernel mode */
  kernelState = TRUE;

  /* If no wait, then just reinsert it */
  if (timeout == WAIT_NONE)
  {
    Q_REMOVE(&kernReadyQ, taskIdCurrent);
    Q_PUT(&kernReadyQ, taskIdCurrent, taskIdCurrent->priority);
  }
  /* Put to sleep */
  else
  {
    vmxDelay(timeout);
  }

  /* Exit trough kernel, and check for error */
  if ( (status = kernExit()) == SIG_RESTART)
  {
    status = ERROR;
  }

  return(status);
}

/**************************************************************
* taskUndelay - Wake up a sleeping task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskUndelay(TCB_ID pTcb)
{
  logString("taskUndelay() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(pTcb))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Put on queue if in kernel mode */
  if (kernelState == TRUE)
  {
    /* Put on kernel queue */
    workQAdd1((FUNCPTR) vmxUndelay, (ARG) pTcb);

    return(OK);
  }

  /* Enter kernel mode */
  kernelState = TRUE;

  /* Resume task */
  vmxUndelay(pTcb);

  /* Exit kernel mode */
  kernExit();

  return(OK);
}

/**************************************************************
* taskPrioritySet - Change task priority
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskPrioritySet(TCB_ID pTcb, unsigned priority)
{
  logString("taskPrioritySet() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(pTcb))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if in kernel mode */
  if (kernelState == TRUE)
  {
    /* Add work to kernel */
    workQAdd2 ((FUNCPTR) vmxPrioritySet, (ARG) pTcb, (ARG) priority);

    return(OK);
  }

  /* Enter kernel mode */
  kernelState = TRUE;

  /* Set priority */
  vmxPrioritySet(pTcb, priority);

  /* Exit trough kernel */
  kernExit();

  return(OK);
}

/**************************************************************
* taskPriorityGet - Get task priority
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskPriorityGet(TCB_ID pTcb, unsigned *priority)
{
  logString("taskPriorityGet() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(pTcb))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Store priority */
  *priority = pTcb->priority;

  return(OK);
}

/**************************************************************
* taskRestart - Restart a task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskRestart(TCB_ID pTcb)
{
  char name[MAX_TASK_NAME_LEN];
  unsigned priority;
  int options;
  char *pStackBase;
  unsigned stackDepth;
  FUNCPTR entry;
  ARG args[MAX_TASK_ARGS];
  STATUS status;
  int len;

  logString("taskRestart() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(pTcb))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Discard self restart */
  if (pTcb == taskIdCurrent)
  {
    logString("ERROR - Self restart not supported",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Copy task data */
  priority = pTcb->priority;
  options = pTcb->options;
  entry = pTcb->entry;
  pStackBase = pTcb->pStackBase;
  stackDepth = (pTcb->pStackLimit - pTcb->pStackBase) * _STACK_DIR;
  taskArgGet(pTcb, pStackBase, args);

  /* Copy name */
  memcpy(name, pTcb->name, strlen(pTcb->name));

  /* Prevent deletion */
  taskSafe();

  if (taskTerminate(pTcb) != OK)
  {
    taskUnsafe();
    logString("ERROR - Task restart failed, becase task won't terminate",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Initialize task with same data */
  status = taskInit(pTcb, name, priority, options, pStackBase,
                    stackDepth, entry, args[0], args[1], args[2],
                    args[3], args[4], args[5], args[6], args[7],
                    args[8], args[9]);
  if (status != OK)
  {
    logString("ERROR - Unable to initialize task again",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* And start it */
  status = taskActivate(pTcb);
  if (status != OK)
  {
    logString("ERROR - Unable to start task again",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Make me mortal */
  taskUnsafe();

  return(OK);
}

/**************************************************************
* taskExit - Exit from task
*
* RETURNS: N/A
**************************************************************/

void taskExit(int code)
{
  logString("taskExit() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (taskIdCurrent == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return;
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return;
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return;
  }

  /* Store return code */
  taskIdCurrent->exitCode = code;

  /* Lock task */
  taskLock();

  /* Destroy task */
  taskDestroy(NULL, TRUE, WAIT_FOREVER, FALSE);

  /* Unlock task */
  taskUnlock();
}

/**************************************************************
* taskLock - Prevent task from beeing switched out
*
* RETURNS: N/A
**************************************************************/

STATUS taskLock(void)
{
  logString("taskLock() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (taskIdCurrent == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  taskIdCurrent->lockCount++;

  return(OK);
}

/**************************************************************
* taskUnlock - Allow task to be switched out
*
* RETURNS: N/A
**************************************************************/

STATUS taskUnlock(void)
{
  logString("taskUnlock() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (taskIdCurrent == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if state is chaged */
  if ( (taskIdCurrent->lockCount > 0) && (--taskIdCurrent->lockCount == 0) )
  {
    /* Enter kernel mode */
    kernelState = TRUE;

    if (Q_FIRST(&taskIdCurrent->safetyQ) != NULL)
      vmxPendQFlush(&taskIdCurrent->safetyQ);

    /* Exit trough kernel */
    kernExit();
  }
  return(OK);
}

/**************************************************************
* taskSafe - Make safe from deletion
*
* RETURNS: Ok or ERROR
**************************************************************/

STATUS taskSafe(void)
{
  logString("taskSafe() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (taskIdCurrent == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  taskIdCurrent->safeCount++;

  return(OK);
}

/**************************************************************
* taskUnsafe - Make unsafe from deletion
*
* RETURNS: Ok or ERROR
**************************************************************/

STATUS taskUnsafe(void)
{
  logString("taskUnsafe() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (taskIdCurrent == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if task lib is installed */
  if (taskLibInstalled == FALSE)
  {
    logString("ERROR - Task lib not installed",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Check if state is chaged */
  if ( (taskIdCurrent->safeCount > 0) && (--taskIdCurrent->safeCount == 0) )
  {
    /* Enter kernel mode */
    kernelState = TRUE;

    if (Q_FIRST(&taskIdCurrent->safetyQ) != NULL)
      vmxPendQFlush(&taskIdCurrent->safetyQ);

    /* Exit trough kernel */
    kernExit();
  }

  return(OK);
}

/**************************************************************
* taskIdSelf - Get my own TCB
*
* RETURNS: TCB_ID
**************************************************************/

TCB_ID taskIdSelf(void)
{
  logString("taskIdSelf() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  return(taskIdCurrent);
}

/**************************************************************
* taskTcb - Get TCB
*
* RETURNS: TCB_ID to task, or to runnig task if NULL as arg
**************************************************************/

TCB_ID taskTcb(TCB_ID pTcb)
{
  logString("taskTcb() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* If NULL, return current task id */
  if (pTcb == NULL) return(taskIdCurrent);

  /* Verify that it is actually a task */
  if (TASK_ID_VERIFY(taskIdCurrent))
  {
    logString("ERROR - Non task object",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(NULL);
  }

  /* Else trivial */
  return(pTcb);
}

/**************************************************************
* taskStackAllot - Allot memory from callers stack
*
* RETURNS: Pointer to memory, or NULL
**************************************************************/

void *taskStackAllot(TCB_ID pTcb, unsigned size)
{
  char *pStackPrev;

  logString("taskStackAllot() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  /* Check if not NULL */
  if (pTcb == NULL)
  {
    logString("ERROR - Null task",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(NULL);
  }

  /* Round up */
  ROUND_UP(size, _STACK_ALIGN_SIZE);

  /* Check if size is ok */
  if (size > (pTcb->pStackLimit - pTcb->pStackBase) * _STACK_DIR)
  {
    logString("ERROR - Stack allot request to big",
              LOG_TASK_LIB,
              LOG_LEVEL_ERROR);
    return(NULL);
  }

  /* Store old stack end */
  pStackPrev = pTcb->pStackLimit;

#if     (_STACK_DIR == _STACK_GROWS_DOWN)
  pTcb->pStackLimit += size;
  return( (void *) pStackPrev);
#else
  pTcb->pStackLimit -+ size;
  return( (void *) pTcb->pStackLimit);
#endif
}

/**************************************************************
* taskIdVerify - Verify that this is actually a task
*
* RETURNS: OK or ERROR
**************************************************************/

STATUS taskIdVerify(TCB_ID pTcb)
{
  logString("taskIdVerify() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  return(TASK_ID_VERIFY(pTcb));
}

/**************************************************************
* taskIdle - An idle task
*
* RETURNS: return value
**************************************************************/

int taskIdle(void)
{
  logString("taskIdle() called:",
            LOG_TASK_LIB,
            LOG_LEVEL_CALLS);

  for (;;);

  return(0);
}

