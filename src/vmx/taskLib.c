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
#include <arch/vmxArchLib.h>
#include <vmx/logLib.h>
#include <vmx/classLib.h>
#include <vmx/objLib.h>
#include <vmx/memPartLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>
#include <vmx/workQLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/taskLib.h>
#include <vmx/vmxLib.h>
#include <vmx/sigLib.h>

/* Defines */
#define TASK_EXTRA_BYTES       16

/* Locals */
LOCAL OBJ_CLASS taskClass;
LOCAL BOOL taskLibInstalled  = FALSE;
LOCAL char restartTaskName[] = "tRestart";
int restartTaskPriority      = 0;
int restartTaskStackSize     = 6000;
int restartTaskOptions       = TASK_OPTIONS_UNBREAKABLE |
                               TASK_OPTIONS_NO_STACK_FILL;

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
        taskActivate(taskId);
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
        STACK_ROUND_UP(stackSize);

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
    int i, len;
    char *taskName;
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

            /* Initialize standard file desriptors */
            for (i = 0; i < 3; i++)
            {
               tcbId->taskStd[i]   = i;
               tcbId->taskStdFp[i] = NULL;
            }

            /* Object core */
            objCoreInit(&tcbId->objCore, taskClassId);

            /* Get stack space for task name */
            len = strlen(name) + 1;
            taskName = (char *) taskStackAllot((int) tcbId, len);
            if (taskName == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Set task name */
                strcpy(taskName, name);
                tcbId->name = taskName;

                /* Start task */
                vmxSpawn(tcbId);
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * taskDelete - Remove task
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
    int taskId,
    BOOL freeStack,
    unsigned timeout,
    BOOL forceDestroy
    )
{
    STATUS status;
    int level;
    TCB_ID tcbId;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        tcbId = taskTcb(taskId);
        if (tcbId == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Loop here if status is SIG_RESTART */
            do
            {
                /* Lock interrupts */
                INT_LOCK(level);

                /* Verify that it is a task */
                if (TASK_ID_VERIFY(tcbId) != OK)
                {
                    /* Unlock interrupts */
                    INT_UNLOCK(level);

                    status = ERROR;
                }
                else
                {
                    /* Block here for safe and running locked tasks */
                    while ((tcbId->safeCount > 0) ||
                           ((tcbId->status == TASK_READY) &&
                            (tcbId->lockCount > 0)))
                    {
                        /* Enter kernel mode */
                        kernelState = TRUE;

                        /* Unlock interrupts */
                        INT_UNLOCK(level);

                        /* Check if force deletion, or suicide */
                        if ((forceDestroy == TRUE) || (tcbId == taskIdCurrent))
                        {
                            /* Remove protections */
                            tcbId->safeCount = 0;
                            tcbId->lockCount = 0;

                            /* Check if flush of safety queue is needed */
                            if (Q_FIRST(&tcbId->safetyQ) != NULL)
                            {
                                vmxPendQFlush(&tcbId->safetyQ);
                            }

                            /* Exit trough kernel */
                            vmxExit();
                            status = OK;
                        }
                        else
                        {
                            /* Not forced deletion or suicide */

                            /* Put task on safe queue */
                            if (vmxPendQPut(&tcbId->safetyQ, timeout) != OK)
                            {
                                /* Exit trough kernel */
                                vmxExit();

                                /* Invalid timeout */
                                errnoSet(S_taskLib_INVALID_TIMEOUT);
                                status = ERROR;
                            }
                            else
                            {
                                /* Exit trough kernel */
                                status = vmxExit();
                                if (status == ERROR)
                                {
                                    /* timer set errno to S_objLib_TIMEOUT */
                                    status = ERROR;
                                }
                                else
                                {
                                    /* Lock interrupts */
                                    INT_LOCK(level);

                                    /* Verify that it still is a valid task */
                                    if (TASK_ID_VERIFY(tcbId) != OK)
                                    {
                                        /* Unlock interrupts */
                                        INT_UNLOCK(level);

                                        status = ERROR;
                                    }
                                }
                            }
                        }
                    }
                }
            } while (status == SIG_RESTART);

            if (status == OK)
            {
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

                /* Lock task */
                taskLock();

                /* If dealloc and options dealloc stack */
                if ((freeStack == TRUE) &&
                    (tcbId->options & TASK_OPTIONS_DEALLOC_STACK))
                {
#if (_STACK_DIR == _STACK_GROWS_DOWN)
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
                {
                    vmxPendQFlush(&tcbId->safetyQ);
                }

                /* Exit trough kernel */
                vmxExit();

                /* Unprotect */
                taskUnlock();
                taskUnsafe();
            }
        }
    }

    return status;
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

    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
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

    if (taskId == 0)
    {
        /* Task must already be running */
        status = OK;
    }
    else
    {
        tcbId = (TCB_ID) taskId;
        if (TASK_ID_VERIFY(tcbId) != OK)
        {
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
            vmxDelay(timeout);
        }

        /* Exit trough kernel, and check for error */
        if ((status = vmxExit()) == SIG_RESTART)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
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
    int taskId,
    unsigned priority
    )
{
    STATUS status;
    TCB_ID tcbId;

    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
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
    int taskId,
    unsigned *priority
    )
{
    STATUS status;
    TCB_ID tcbId;

    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
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
    STATUS status;
    TCB_ID tcbId;
    char *name, *rename;
    int len;
    unsigned priority;
    int options;
    char *pStackBase;
    unsigned stackSize;
    FUNCPTR entry;
    ARG args[MAX_TASK_ARGS];

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        /* If self restart */
        if ((taskId == 0) || (taskId == (int) taskIdCurrent))
        {
            /* Task must be safe */
            while (taskIdCurrent->safeCount > 0)
            {
                taskSafe();
            }

            /* Spawn a task that will restart this task */
            taskSpawn(
                restartTaskName,
                restartTaskPriority,
                restartTaskOptions,
                restartTaskStackSize,
                taskRestart,
                (ARG) taskIdCurrent,
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

            /* Block until task restarts */
            while (1)
            {
                taskSuspend(0);
            }
            status = OK;
        }
        else
        {
            tcbId = taskTcb(taskId);
            if (tcbId == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Copy task data */
                priority = tcbId->priority;
                options = tcbId->options;
                entry = tcbId->entry;
                pStackBase = tcbId->pStackBase;
                stackSize = (tcbId->pStackLimit - tcbId->pStackBase) *
                               _STACK_DIR;
                taskArgGet(tcbId, pStackBase, args);

                /* Copy name if needed */
                name = tcbId->name;
                rename = NULL;
                if (name != NULL)
                {
                    len = strlen(name) + 1;
                    rename = (char *) malloc(len);
                    if (rename != NULL)
                    {
                        strcpy(rename, name);
                    }
                    name = rename;
                }

                /* Prevent deletion */
                taskSafe();

                if (taskTerminate((int) tcbId) != OK)
                {
                    taskUnsafe();
                    status = ERROR;
                }
                else
                {
                    /* Initialize task with same data */
                    status = taskInit(
                        tcbId,
                        name,
                        priority,
                        options,
                        pStackBase,
                        stackSize,
                        entry,
                        args[0],
                        args[1],
                        args[2],
                        args[3],
                        args[4],
                        args[5],
                        args[6],
                        args[7],
                        args[8],
                        args[9]
                        );
                }
                if (status == OK)
                {
                    /* Start the new task */
                    status = taskActivate((int) tcbId);
                    if (status == OK)
                    {
                        /* Make me mortal */
                        taskUnsafe();
                    }
                }

                /* Free rename memory if allocated */
                if (rename != NULL)
                {
                    free(rename);
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * taskExit - Exit from task
 *
 * RETURNS: N/A
 */

void taskExit(
    int exitCode
    )
{
    /* Store exit code to current task */
    taskIdCurrent->exitCode = exitCode;

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
    /* Increase task lock count */
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
    /* Check if state is chaged and decrease task lock count */
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
    /* Increase task safe counter */
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
    /* Check if state is chaged and decrease task safe counter */
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
 * taskIdSelf - Get current task id
 *
 * RETURNS: Task id
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
 * RETURNS: Task control block id or NULL
 */

TCB_ID taskTcb(
    int taskId
    )
{
    TCB_ID tcbId;

    /* Zero means the calling task id */
    if (taskId == 0)
    {
        tcbId = taskIdCurrent;
    }
    else
    {
        tcbId = (TCB_ID) taskId;
    }

    /* Verify that it is actually a task */
    if (TASK_ID_VERIFY(tcbId) != OK)
    {
        tcbId = NULL;
    }

    return(tcbId);
}

/******************************************************************************
 * taskStackAllot - Allot memory from callers stack
 *
 * RETURNS: Pointer to memory or NULL
 */

void* taskStackAllot(
    int taskId,
    unsigned size
    )
{
    TCB_ID tcbId;
    char *pStackPrev;
    char *pData;

    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        pData = NULL;
    }
    else
    {
        /* Round up */
        STACK_ROUND_UP(size);

        /* Check if size is ok */
        if (size > (tcbId->pStackLimit - tcbId->pStackBase) * _STACK_DIR)
        {
            errnoSet(S_taskLib_STACK_OVERFLOW);
            pData = NULL;
        }
        else
        {
            /* Store old stack end */
            pStackPrev = tcbId->pStackLimit;

#if     (_STACK_DIR == _STACK_GROWS_DOWN)
            tcbId->pStackLimit += size;
            pData = pStackPrev;
#else
            tcbId->pStackLimit -= size;
            pData = tcbId->pStackLimit;
#endif
        }
    }

    return pData;
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

    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskIdle - An idle task
 *
 * RETURNS: return value
 */

int taskIdle(
    void
    )
{
    for (;;);

    /* Should never get here */
    return 0;
}

