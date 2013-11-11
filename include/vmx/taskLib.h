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

/* taskLib.h - Task scheduling library */

#ifndef _taskLib_h
#define _taskLib_h

#include <tools/moduleNumber.h>

#define S_taskLib_NOT_INSTALLED                 (M_taskLib | 0x0001)
#define S_taskLib_NULL_TASK_ID                  (M_taskLib | 0x0002)
#define S_taskLib_INVALID_TIMEOUT               (M_taskLib | 0x0003)
#define S_taskLib_STACK_OVERFLOW                (M_taskLib | 0x0004)
#define S_taskLib_TASK_HOOK_TABLE_FULL          (M_taskLib | 0x0005)
#define S_taskLib_TASK_HOOK_NOT_FOUND           (M_taskLib | 0x0006)
#define S_taskLib_TASK_SWAP_HOOK_REFERENCED     (M_taskLib | 0x0007)
#define S_taskLib_TASK_SWAP_HOOK_SET            (M_taskLib | 0x0008)
#define S_taskLib_TASK_SWAP_HOOK_CLEAR          (M_taskLib | 0x0009)

#define TASK_READY                      0x00
#define TASK_SUSPEND                    0x01
#define TASK_PEND                       0x02
#define TASK_DELAY                      0x04
#define TASK_DEAD                       0x08

#define NUM_TASK_PRIORITIES             256
#define MAX_TASK_DELAY                  0xffffffff
#define DEFAULT_STACK_SIZE              0x1000

#define MAX_TASK_ARGS                   10

#define TASK_UNDELAYED                  2
#define TASK_DELETED                    3

#define TASK_OPTIONS_SUPERVISOR_MODE    0x0001
#define TASK_OPTIONS_UNBREAKABLE        0x0002
#define TASK_OPTIONS_DEALLOC_STACK      0x0004
#define TASK_OPTIONS_NO_STACK_FILL      0x0100

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/regs.h>
#include <arch/excArchLib.h>
#include <util/qLib.h>
#include <vmx/semLib.h>
#include <os/classLib.h>
#include <os/objLib.h>
#include <os/selectLib.h>
#include <stdio.h>

/* Task control block */
typedef struct taskTCB
{
    /* Queues for sheduler */
    Q_HEAD qNode;                         /* 0x00 */
    Q_HEAD tickNode;                      /* 0x10 */
    Q_HEAD activeNode;                    /* 0x20 */

    /* Stack */
    char *pStackBase;                     /* 0x30 */
    char *pStackLimit;                    /* 0x34 */
    char *pStackEnd;                      /* 0x38 */

    /* Entry point */
    FUNCPTR entry;                        /* 0x3c */

    /* Errno */
    int errno;                            /* 0x40 */

    /* Status */
    unsigned id;                          /* 0x44 */
    unsigned status;                      /* 0x48 */
    unsigned lockCount;                   /* 0x4c */

    /* Kernel hooks */
    u_int16_t swapInMask;                 /* 0x50 */
    u_int16_t swapOutMask;                /* 0x52 */

    /* Registers */
    REG_SET regs;                         /* 0x54 */

    char *name;
    unsigned priority;
    int options;

    /* Timing */
    unsigned timeSlice;

    /* Used with semaphores */
    Q_HEAD *pPendQ;

    /*
     * These next three fields are used to handle the case when the task
     * has timed out, or has been deleted, BEFORE it has obtained the
     * object on which it is pending.  Some objects require special code
     * for this (inversion safe mutex semaphores, read-write semaphores, ...).
     */

    void    (*objUnpendHandler)(void *, int);
    void   *pObj;     /* ptr to object (sem/msgQ/...) on which task pends */
    int     objInfo;  /* info to handler about what to do */

    /* Safety */
    unsigned safeCount;
    Q_HEAD safetyQ;

    /* Error codes */
    int errorStatus;
    int exitCode;

    /* Exception info */
    REG_SET *pExcRegSet;
    EXC_INFO excInfo;

    /* I/O related */
    int   taskStd[3];
    FILE *taskStdFp[3];

    /* Select context */
    SEL_CONTEXT_ID selectContextId;

    /* Class */
    OBJ_CORE objCore;
} __attribute__((packed)) TCB;

/* Exports */
typedef TCB    *TCB_ID;
IMPORT CLASS_ID taskClassId;

/* Macros */

/******************************************************************************
 * TASK_ID_VERIFY - Verify that this is a task object
 *
 * RETURNS: OK or ERROR
 */

#define TASK_ID_VERIFY(tcbId)                                                 \
    (OBJ_VERIFY(tcbId, taskClassId))

/* Functions */

/******************************************************************************
 * taskLibInit - Initialize task library
 *
 * RETURNS: OK or ERROR
 */

STATUS taskLibInit(
    void
    );

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
    );

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
    );

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
    );

/******************************************************************************
 * taskDelete - Remove task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDelete(
    int taskId
    );

/******************************************************************************
 * taskDeleteForce - Remove task forced
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDeleteForce(
    int taskId
    );

/******************************************************************************
 * taskTerminate - Terminate task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskTerminate(
    int taskId
    );

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
    );

/******************************************************************************
 * taskActivate - Activate task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskActivate(
    int taskId
    );

/******************************************************************************
 * taskSuspend - Suspend a task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSuspend(
    int taskId
    );

/******************************************************************************
 * taskResume - Resume task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskResume(
    int taskId
    );

/******************************************************************************
 * taskDelay - Put a task to sleep
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDelay(
    unsigned timeout
    );

/******************************************************************************
 * taskUndelay - Wake up a sleeping task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskUndelay(
    int taskId
    );

/******************************************************************************
 * taskPrioritySet - Change task priority
 *
 * RETURNS: OK or ERROR
 */

STATUS taskPrioritySet(
    int      taskId,
    unsigned priority
    );

/******************************************************************************
 * taskPriorityGet - Get task priority
 *
 * RETURNS: OK or ERROR
 */

STATUS taskPriorityGet(
    int       taskId,
    unsigned *priority
    );

/******************************************************************************
 * taskRestart - Restart a task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskRestart(
    int taskId
    );

/******************************************************************************
 * taskExit - Exit from task
 *
 * RETURNS: N/A
 */

void taskExit(
    int exitCode
    );

/******************************************************************************
 * taskLock - Prevent task from beeing switched out
 *
 * RETURNS: OK
 */

STATUS taskLock(
    void
    );

/******************************************************************************
 * taskUnlock - Allow task to be switched out
 *
 * RETURNS: OK
 */

STATUS taskUnlock(
    void
    );

/******************************************************************************
 * taskSafe - Make safe from deletion
 *
 * RETURNS: OK
 */

STATUS taskSafe(
    void
    );

/******************************************************************************
 * taskUnsafe - Make unsafe from deletion
 *
 * RETURNS: OK or ERROR
 */

STATUS taskUnsafe(
    void
    );

/******************************************************************************
 * taskIdSelf - Get current task id
 *
 * RETURNS: Task id 
 */

int taskIdSelf(
    void
    );

/******************************************************************************
 * taskTcb - Get TCB
 *
 * RETURNS: Pointer to task control block or NULL
 */

TCB_ID taskTcb(
    int taskId
    );

/******************************************************************************
 * taskStackAllot - Allot memory from callers stack
 *
 * RETURNS: Pointer to memory, or NULL
 */

void* taskStackAllot(
    int      taskId,
    unsigned size
    );

/******************************************************************************
 * taskIdVerify - Verify that this is actually a task
 * 
 * RETURNS: OK or ERROR
 */
 
STATUS taskIdVerify(
    int taskId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskLib_h */

