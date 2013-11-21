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

#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/vmxArchLib.h>
#include <arch/taskArchLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <vmx/vmxLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/classLib.h>
#include <os/objLib.h>
#include <os/sigLib.h>
#include <os/memPartLib.h>
#include <os/private/taskHookLibP.h>
#include <os/taskHookLib.h>
#include <vmx/taskLib.h>

/* Defines */
#define TASK_EXTRA_BYTES                16

/* Locals */
LOCAL OBJ_CLASS taskClass;
LOCAL BOOL      taskLibInstalled = FALSE;
LOCAL int       namelessCount = 0;

/* Globals */
CLASS_ID taskClassId          = &taskClass;
char     namelessPrefix[]     = "t";
char     restartTaskName[]    = "tRestart";
int      restartTaskPriority  = 0;
int      restartTaskStackSize = 6000;
int      restartTaskOptions   = TASK_OPTIONS_UNBREAKABLE |
                                TASK_OPTIONS_NO_STACK_FILL;

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
        if (classInit(taskClassId,
                STACK_ROUND_UP(sizeof(TCB) + TASK_EXTRA_BYTES),
                OFFSET(TCB, objCore),
                memSysPartId,
                (FUNCPTR) taskCreat,
                (FUNCPTR) taskInit,
                (FUNCPTR) taskDestroy
                ) != OK)
        {
            /* errno set by classInit() */
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
 * taskSpawn - Create a new task and start it
 *
 * RETURNS: Integer taskId or zero
 */

int taskSpawn(
    const char *name,
    unsigned    priority,
    int         options,
    unsigned    stackSize,
    FUNCPTR     func,
    ARG         arg0,
    ARG         arg1,
    ARG         arg2,
    ARG         arg3,
    ARG         arg4,
    ARG         arg5,
    ARG         arg6,
    ARG         arg7,
    ARG         arg8,
    ARG         arg9
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
        taskActivate(taskId);
    }

    return taskId;
}

/******************************************************************************
 * taskCreat - Create a new task without starting it
 *
 * RETURNS: Integer taskId or or zero
 */

int taskCreat(
    const char *name,
    unsigned    priority,
    int         options,
    unsigned    stackSize,
    FUNCPTR     func,
    ARG         arg0,
    ARG         arg1,
    ARG         arg2,
    ARG         arg3,
    ARG         arg4,
    ARG         arg5,
    ARG         arg6,
    ARG         arg7,
    ARG         arg8,
    ARG         arg9
    )
{
    static char digits[] = "0123456789";

    TCB_ID  tcbId;
    char   *pTaskMem;
    char   *pStackBase;
    char    newName[20];
    int     value;
    int     nBytes;
    int     nPreBytes;
    char   *pBufStart;
    char   *pBufEnd;
    char    ch;

    /* Check if task lib is installed */
    if (taskLibInstalled != TRUE)
    {
        errnoSet(S_taskLib_NOT_INSTALLED);
        tcbId = NULL;
    }
    else
    {
        /* If NULL name create default name for task */
        if (name == NULL)
        {
            strcpy(newName, namelessPrefix);
            nBytes    = strlen(newName);
            nPreBytes = nBytes;
            value     = ++namelessCount;

            /* Do while value is non-zero */
            do
            {
                newName[nBytes++] = digits[value % 10];
                value /= 10;
            } while(value != 0);

            /* Calculate start/end positions in name */
            pBufStart = newName + nPreBytes;
            pBufEnd   = newName + nBytes - 1;

            /* While startbuffer lt. end buffer */
            while (pBufStart < pBufEnd)
            {
                ch         = *pBufStart;
                *pBufStart = *pBufEnd;
                *pBufEnd   = ch;
                pBufStart++;
                pBufEnd--;
            }

            /* Terminate string */
            newName[nBytes] = EOS;

            /* Use this a the name for the task */
            name = newName;
        }

        /* Round up stack size */
        stackSize = STACK_ROUND_UP(stackSize);

        /* Allocate new TCB plus stack */
        pTaskMem = objAllocPad(
                       taskClassId,
                       (unsigned) stackSize,
                       (void **) NULL
                       );
        if (pTaskMem == NULL)
        {
            /* errno set by objAllocPad() */
            tcbId = NULL;
        }
        else
        {
            /* Setup stack vars */

#if (_STACK_DIR == _STACK_GROWS_DOWN)
            pStackBase = pTaskMem + stackSize;
            tcbId      = (TCB_ID) pStackBase;
#else /* _STACK_GROWS_UP */
            tcbId      = (TCB_ID) (pTaskMem + TASK_EXTRA_BYTES);
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
                /* errno set by taskInit() */
                objFree(taskClassId, tcbId);
                tcbId = NULL;
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
    TCB_ID      tcbId,
    const char *name,
    unsigned    priority,
    int         options,
    char       *pStackBase,
    unsigned    stackSize,
    FUNCPTR     func,
    ARG         arg0,
    ARG         arg1,
    ARG         arg2,
    ARG         arg3,
    ARG         arg4,
    ARG         arg5,
    ARG         arg6,
    ARG         arg7,
    ARG         arg8,
    ARG         arg9
    )
{
    static unsigned new_id;

    STATUS status;
    int    i;
    int    len;
    char  *taskName;
    ARG    args[MAX_TASK_ARGS];

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

            /* Task entry point */
            tcbId->entry = func;

            /* Setup errno */
            tcbId->errno = 0;

            /* Set unique id */
            tcbId->id = new_id++;

            /* Set initial status as suspended */
            tcbId->status    = TASK_SUSPEND;
            tcbId->lockCount = 0;

            /* Zero swap mask */
            tcbId->swapInMask  = 0;
            tcbId->swapOutMask = 0;

            /* Name and options */
            tcbId->priority = priority;
            tcbId->options  = options;

            /* Round robin time slice */
            tcbId->timeSlice   = 0;

            /* Pending queue, used by semaphores */
            tcbId->pPendQ = NULL;

            /* Zero unpend callback */
            tcbId->objUnpendHandler = NULL;
            tcbId->pObj             = NULL;
            tcbId->objInfo          = 0;

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
                (ARG) 0);

            /* Setup error status and exit code */
            tcbId->errorStatus = OK;
            tcbId->exitCode    = 0;

#ifdef TASKVAR
            /* Zero task extensions */
            tcbId->pTaskVar = NULL;
#endif

#ifdef SIGNAL
            /* Zero signals and exception info */
            tcbId->pSignalInfo = NULL;
#endif

            /* Task variables */
            tcbId->pTaskVar = NULL;

            /* Exception info */
            tcbId->pExcRegSet    = NULL;
            tcbId->excInfo.valid = 0;

            /* Setup stack */
            tcbId->pStackBase  = pStackBase;
            tcbId->pStackLimit = tcbId->pStackBase + stackSize * _STACK_DIR;
            tcbId->pStackEnd   = tcbId->pStackLimit;

            if ((options & TASK_OPTIONS_NO_STACK_FILL) == 0)
            {
#if (_STACK_DIR == _STACK_GROWS_DOWN)
                memset(tcbId->pStackLimit, 0xee, stackSize);
#else /* _STACK_GROWS_UP */
                memset(tcbId->stackBase, 0xee, stackSize);
#endif /* _STACK_DIR */
            }

            /* Initialize standard file desriptors */
            for (i = 0; i < 3; i++)
            {
                tcbId->taskStd[i]   = i;
                tcbId->taskStdFp[i] = NULL;
            }

            /* Environment */
            tcbId->ppEnviron = NULL;

            /* Initialize architecutre depedent stuff */
            taskRegsInit(tcbId, pStackBase);

            /* Push args on task stack */
            taskArgSet(tcbId, pStackBase, args);

            /* Object core */
            objCoreInit(&tcbId->objCore, taskClassId);

            /* Copy name if not unnamed */
            if (name != NULL)
            {
                len = strlen(name) + 1;

                taskName = (char *) taskStackAllot((int) tcbId, len);
                if (taskName != NULL)
                {
                    strcpy(taskName, name);
                }

                tcbId->name = taskName;
            }
            else
            {
                tcbId->name = NULL;
            }

            /* Run create hooks */
            for (i = 0; i < MAX_TASK_CREATE_HOOKS; i++)
            {
                if (taskCreateHooks[i] != NULL)
                {
                    (*taskCreateHooks[i])(tcbId);
                }
            }

            /* Set task as active */
            vmxSpawn(tcbId);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * taskDelete - Delete task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDelete(
    int taskId
    )
{
    return taskDestroy(taskId, TRUE, WAIT_FOREVER, FALSE);
}

/******************************************************************************
 * taskDeleteForce - Remove task forced
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDeleteForce(
    int taskId
    )
{
    return taskDestroy(taskId, TRUE, WAIT_FOREVER, TRUE);
}

/******************************************************************************
 * taskTerminate - Terminate task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskTerminate(
    int taskId
    )
{
    return taskDestroy(taskId, FALSE, WAIT_FOREVER, FALSE);
}

/******************************************************************************
 * taskDestroy - Kill task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDestroy(
    int      taskId,
    BOOL     freeStack,
    unsigned timeout,
    BOOL     forceDestroy
    )
{
  STATUS status;
  int i, level;
  TCB_ID tcbId;

  if (INT_RESTRICT() != OK)
  {
    errnoSet (S_intLib_NOT_ISR_CALLABLE);
    return ERROR;
  }

  /* Get task context */
  tcbId = taskTcb(taskId);
  if (tcbId == NULL)
    return ERROR;

  /* If task self destruct and excption lib installed */
  if (tcbId == taskIdCurrent) {

    /* Wait for safe to destroy */
    while (tcbId->safeCount > 0)
      taskUnsafe();

    /* Kill it */
    status = excJobAdd(
        (VOIDFUNCPTR) taskDestroy,
        (ARG) tcbId,
        (ARG) freeStack,
        (ARG) WAIT_NONE,
        (ARG) FALSE
        );

    /* Block here and suspend */
    while(status == OK)
      taskSuspend(0);

  } /* End if task self destruct and exception lib installed */

taskDestroyLoop:

  /* Lock interrupts */
  INT_LOCK(level);

  /* Check id */
  if (TASK_ID_VERIFY(tcbId) != OK)
  {
    /* errno set by taskIdVerify() */

    /* Unlock interrupts */
    INT_UNLOCK(level);

    return ERROR;
  }

#ifdef SIGNAL
  /* Mask all signals */
  if (tcbId->pSignalInfo != NULL)
    tcbId->pSignalInfo->sigt_blocked = 0xffffffff;
#endif

  /* Block here for safe and running locked tasks */
  while ( (tcbId->safeCount > 0) ||
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
      vmxExit();
    }
    else
    {
      /* Not forced deletion or suicide */

      /* Put task on safe queue */
      if (vmxPendQPut(&tcbId->safetyQ, timeout) != OK)
      {
        /* Exit trough kernel */
        vmxExit();

        errnoSet (S_taskLib_INVALID_TIMEOUT);

        return ERROR;
      }

      /* Exit trough kernel */
      status = vmxExit();

      /* Check for restart */
      if (status == SIG_RESTART)
      {
#ifdef SIGNAL
        /* If singnal recalc function exists */
        if (_func_sigTimeoutRecalc != NULL) {

          timeout = ( *_func_sigTimeoutRecalc) (timeout);
          goto taskDestroyLoop;

        } /* End if signal recalc function exists */
#endif
      }

      /* Check if unsuccessful */
      if (status == ERROR)
      {
          /* timer should have set errno to S_objLib_TIMEOUT */
          return ERROR;
      }

    } /* End else forced or suicide */

    /* Lock interrupts */
    INT_LOCK(level);

    /* Now verify class id again */
    if (TASK_ID_VERIFY(tcbId) != OK)
    {
      /* errno set by taskIdVerify() */

      /* Unlock interrupts */
      INT_UNLOCK(level);

      return ERROR;
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
     vmxExit();
  }
  else
  {
     /* Unlock interrupts */
     INT_UNLOCK(level);
  }

  /* Run deletion hooks */
  for (i = 0; i < MAX_TASK_DELETE_HOOKS; i++)
    if (taskDeleteHooks[i] != NULL)
      (*taskDeleteHooks[i])(tcbId);

  /* Lock task */
  taskLock();

  /* If dealloc and options dealloc stack */
  if ( freeStack && (tcbId->options & TASK_OPTIONS_DEALLOC_STACK) ) {

#if (_STACK_DIR == _STACK_GROWS_DOWN)

    objFree(taskClassId, tcbId->pStackEnd);

#else /* _STACK_GROWS_UP */

    objFree(taskClassId, tbcId - TASK_EXTRA_BYTES);

#endif /* _STACK_DIR */

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
  vmxExit();

  /* Unprotect */
  taskUnlock();
  taskUnsafe();

  return OK;
}

/******************************************************************************
 * taskActivate - Activate task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskActivate(
    int taskId
    )
{
    return taskResume(taskId);
}

/******************************************************************************
 * taskSuspend - Suspend a task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSuspend(
    int taskId
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* Get task context */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* TASK_ID_VERIFY() already done by taskTcb() */

        /* Check if in kernel mode */
        if (kernelState == TRUE)
        {
            /* Add to kernel queue */
            workQAdd1((FUNCPTR) vmxSuspend, (ARG) tcbId);
            status = OK;
        }
        else
        {
            /* Enter kernel mode */
            kernelState = TRUE;

            /* Suspend task */
            vmxSuspend(tcbId);

            /* Exit trough kernel */
            vmxExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * taskResume - Resume task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskResume(
    int taskId
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* If try to resume current task */
    if (taskId == 0)
    {
        status = OK;
    }
    else
    {
        tcbId = (TCB_ID) taskId;

        /* Verify that it is actually a task */
        if (TASK_ID_VERIFY(tcbId) != OK)
        {
            /* errno set by TASK_ID_VERIFY() */
            status = ERROR;
        }
        else
        {
            /* Put on queue if in kernel mode */
            if (kernelState == TRUE)
            {
                /* Put on kernel queue */
                workQAdd1((FUNCPTR) vmxResume, (ARG) tcbId);
                status = OK;
            }
            else
            {
                /* Enter kernel mode */
                kernelState = TRUE;

                /* Resume task */
                vmxResume(tcbId);

                /* Exit kernel mode */
                vmxExit();
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * taskDelay - Put a task to sleep
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDelay(
    unsigned timeout
    )
{
    STATUS status;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        /* Enter kernel mode */
        kernelState = TRUE;

        /* If no wait, then just re-insert it */
        if (timeout == WAIT_NONE)
        {
            Q_REMOVE(&readyQHead, taskIdCurrent);
            Q_PUT(&readyQHead, taskIdCurrent, taskIdCurrent->priority);
        }
        else
        {
            /* Put to sleep */
            vmxDelay(timeout);
        }

        /* Exit trough kernel, and check for error */
        if ((status = vmxExit()) == SIG_RESTART)
        {
            status = ERROR;
        }
    }

    return status;
}

/******************************************************************************
 * taskUndelay - Wake up a sleeping task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskUndelay(
    int taskId
    )
{
    STATUS status;
    TCB_ID tcbId;

    tcbId = (TCB_ID) taskId;

    /* Verify that it is actually a task */
    if (TASK_ID_VERIFY(tcbId) != OK)
    {
        /* errno set by TASK_ID_VERIFY() */
        status = ERROR;
    }
    else
    {
        /* Put on queue if in kernel mode */
        if (kernelState == TRUE)
        {
            /* Put on kernel queue */
            workQAdd1((FUNCPTR) vmxUndelay, (ARG) tcbId);
            status = OK;
        }
        else
        {
            /* Enter kernel mode */
            kernelState = TRUE;

            /* Resume task */
            vmxUndelay(tcbId);

            /* Exit kernel mode */
            vmxExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * taskPrioritySet - Change task priority
 *
 * RETURNS: OK or ERROR
 */

STATUS taskPrioritySet(
    int      taskId,
    unsigned priority
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* Get task context */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* TASK_ID_VERIFY() already done by taskTcb() */

        /* Check if in kernel mode */
        if (kernelState == TRUE)
        {
            /* Add work to kernel */
            workQAdd2((FUNCPTR) vmxPrioritySet, (ARG) tcbId, (ARG) priority);
            status = OK;
        }
        else
        {
            /* Enter kernel mode */
            kernelState = TRUE;

            /* Set priority */
            vmxPrioritySet(tcbId, priority);

            /* Exit trough kernel */
            vmxExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * taskPriorityGet - Get task priority
 *
 * RETURNS: OK or ERROR
 */

STATUS taskPriorityGet(
    int       taskId,
    unsigned *priority
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* Get task context */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* TASK_ID_VERIFY() already done by taskTcb() */

        /* Store priority */
        *priority = tcbId->priority;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskRestart - Restart a task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskRestart(
    int taskId
    )
{
  TCB_ID tcbId;
  char *name, *rename;
  int len;
  unsigned priority;
  int options;
  char *pStackBase;
  unsigned stackSize;
  FUNCPTR entry;
  ARG args[MAX_TASK_ARGS];
  STATUS status;

  if (INT_RESTRICT() != OK)
  {
    errnoSet(S_intLib_NOT_ISR_CALLABLE);
    return ERROR;
  }

  /* If self restart */
  if ( (taskId == 0) || (taskId == (int) taskIdCurrent) )
  {

    /* Task must be unsafe */
    while (taskIdCurrent->safeCount > 0)
      taskSafe();

    /* Spawn a task that will restart this task */
    taskSpawn(restartTaskName, restartTaskPriority, restartTaskOptions,
              restartTaskStackSize, taskRestart, (ARG) taskIdCurrent,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0,
              (ARG) 0);

    /* Wait for restart */
    while (1)
      taskSuspend(0);

  } /* End if self restart */

  /* Get task context */
  tcbId = taskTcb(taskId);
  if (tcbId == NULL)
    return ERROR;

  /* TASK_ID_VERIFY() already done by taskTcb() */

  /* Copy task data */
  priority = tcbId->priority;
  options = tcbId->options;
  entry = tcbId->entry;
  pStackBase = tcbId->pStackBase;
  stackSize = (tcbId->pStackEnd - tcbId->pStackBase) * _STACK_DIR;
  taskArgGet(tcbId, pStackBase, args);

  /* Copy name if needed */
  name = tcbId->name;
  rename = NULL;
  if (name != NULL) {
    len = strlen(name) + 1;
    rename = malloc(len);
    if (rename != NULL)
      strcpy(rename, name);
    name = rename;
  }

  /* Prevent deletion */
  taskSafe();

  if (taskTerminate((int) tcbId) != OK)
  {
    taskUnsafe();
    /* errno set by taskTerminate() */
    return ERROR;
  }

  /* Initialize task with same data */
  status = taskInit(tcbId, name, priority, options, pStackBase,
                    stackSize, entry, args[0], args[1], args[2],
                    args[3], args[4], args[5], args[6], args[7],
                    args[8], args[9]);
  if (status != OK)
  {
    /* errno set by taskInit() */
    return ERROR;
  }

  /* And start it */
  status = taskActivate((int) tcbId);
  if (status != OK)
  {
    /* errno set by taskActivate() */
    return ERROR;
  }

  /* Make me mortal */
  taskUnsafe();

  /* Free rename buffer if needed */
  if (rename != NULL)
    free(rename);

  return OK;
}

/******************************************************************************
 * taskExit - Exit from task
 *
 * RETURNS: N/A
 */

void taskExit(
    int code
    )
{
    /* Store return code */
    taskIdCurrent->exitCode = code;

    /* Lock task */
    taskLock();

    /* Destroy task */
    taskDestroy(NULL, TRUE, WAIT_FOREVER, FALSE);

    /* Unlock task */
    taskUnlock();
}

/******************************************************************************
 * taskLock - Prevent task from beeing switched out
 *
 * RETURNS: OK
 */

STATUS taskLock(
    void
    )
{
    taskIdCurrent->lockCount++;

    return OK;
}

/******************************************************************************
 * taskUnlock - Allow task to be switched out
 *
 * RETURNS: OK
 */

STATUS taskUnlock(
    void
    )
{
    /* Check if state is chaged */
    if ((taskIdCurrent->lockCount > 0) && (--taskIdCurrent->lockCount == 0))
    {
        /* Enter kernel mode */
        kernelState = TRUE;

        if (Q_FIRST(&taskIdCurrent->safetyQ) != NULL)
        {
            vmxPendQFlush(&taskIdCurrent->safetyQ);
        }

        /* Exit trough kernel */
        vmxExit();
    }

    return OK;
}

/******************************************************************************
 * taskSafe - Make safe from deletion
 *
 * RETURNS: OK
 */

STATUS taskSafe(
    void
    )
{
    taskIdCurrent->safeCount++;

    return OK;
}

/******************************************************************************
 * taskUnsafe - Make unsafe from deletion
 *
 * RETURNS: OK or ERROR
 */

STATUS taskUnsafe(
    void
    )
{
    /* Check if state is chaged */
    if ((taskIdCurrent->safeCount > 0) && (--taskIdCurrent->safeCount == 0))
    {
        /* Enter kernel mode */
        kernelState = TRUE;

        if (Q_FIRST(&taskIdCurrent->safetyQ) != NULL)
        {
            vmxPendQFlush(&taskIdCurrent->safetyQ);
        }

        /* Exit trough kernel */
        vmxExit();
    }

    return OK;
}

/******************************************************************************
 * taskIdSelf - Get my own TCB
 *
 * RETURNS: Integer taskId
 */

int taskIdSelf(
    void
    )
{
    return (int) taskIdCurrent;
}

/******************************************************************************
 * taskTcb - Get TCB
 *
 * RETURNS: Pointer to task control block or NULL
 */

TCB_ID taskTcb(
    int taskId
    )
{
    TCB_ID tcbId;

    /* If 0, return current task id */
    if (taskId == 0)
    {
        tcbId = taskIdCurrent;
    }
    else
    {
        /* Convert to pointer to TCB struct */
        tcbId = (TCB_ID) taskId;

        /* Verify that it is actually a task */
        if (TASK_ID_VERIFY(tcbId) != OK)
        {
            /* errno set by TASK_ID_VERIFY() */
            tcbId = NULL;
        }
    }

    return tcbId;
}

/******************************************************************************
 * taskStackAllot - Allot memory from callers stack
 *
 * RETURNS: Pointer to memory, or NULL
 */

void* taskStackAllot(
    int      taskId,
    unsigned size
    )
{
    TCB_ID tcbId;
    char  *pStackPrev;
    void  *pNewMem;

    /* Get task context */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        pNewMem = NULL;
    }
    else
    {
        /* TASK_ID_VERIFY() already done by taskTcb() */

        /* Round up */
        size = STACK_ROUND_UP(size);

        /* Check if size is ok */
        if (size > (tcbId->pStackLimit - tcbId->pStackBase) * _STACK_DIR)
        {
            errnoSet(S_taskLib_STACK_OVERFLOW);
            pNewMem = NULL;
        }
        else
        {
#if (_STACK_DIR == _STACK_GROWS_DOWN)
            pStackPrev          = tcbId->pStackLimit;
            tcbId->pStackLimit += size;
            pNewMem = pStackPrev;
#else /* _STACK_GROWS_UP */
            tcbId->pStackLimit -= size;
            pNewMem = tcbId->pStackLimit;
#endif /* _STACK_DIR */
        }
    }

    return pNewMem;
}

/******************************************************************************
 * taskIdVerify - Verify that this is actually a task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskIdVerify(
    int taskId
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* Get task context */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* TASK_ID_VERIFY() already done by taskTcb() */
        status = OK;
    }

    return status;
}

