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

#include <vmx/moduleNumber.h>

#define S_taskLib_NOT_INSTALLED                 (M_taskLib | 0x0001)
#define S_taskLib_NULL_TASK_ID                  (M_taskLib | 0x0002)
#define S_taskLib_INVALID_TIMEOUT               (M_taskLib | 0x0003)
#define S_taskLib_STACK_OVERFLOW                (M_taskLib | 0x0004)

#define TASK_READY                      0x00
#define TASK_SUSPEND                    0x01
#define TASK_PEND                       0x02
#define TASK_DELAY                      0x04
#define TASK_DEAD                       0x08

#define NUM_TASK_PRIORITIES             256
#define MAX_TASK_NAME_LEN               256
#define MAX_TASK_DELAY                  0xffffffff
#define DEFAULT_STACK_SIZE              0x1000

#define MAX_TASK_ARGS                   10

#define TASK_UNDELAYED                  2
#define TASK_DELETED                    3

#define TASK_OPTIONS_DEALLOC_STACK      0x01

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/regs.h>
#include <vmx/classLib.h>
#include <vmx/objLib.h>
#include <vmx/semLib.h>
#include <util/qLib.h>

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

    /* Registers */
    REG_SET regs;                         /* 0x38 */

    /* Entry point */
    FUNCPTR entry;                        /* 0x60 */

    /* Errno */
    int errno;                            /* 0x64 */

    /* Status */
    unsigned id;                          /* 0x68 */
    unsigned status;                      /* 0x6c */
    unsigned lockCount;                   /* 0x70 */

    char name[MAX_TASK_NAME_LEN];
    unsigned stackDepth;
    unsigned priority;
    int options;

    /* Timing */
    unsigned timeSlice;

    /* Used with semaphores */
    Q_HEAD *pPendQ;

    /* Safety */
    unsigned safeCount;
    Q_HEAD safetyQ;

    /* Error codes */
    int errorStatus;
    int exitCode;

    /* Class */
    OBJ_CORE objCore;

} __attribute__((packed)) TCB;

/* Exports */
typedef TCB *TCB_ID;
IMPORT CLASS_ID taskClassId;

/* Macros */

/******************************************************************************
 * TASK_ID_VERIFY - Verify that this is a task object
 *
 * RETURNS: OK or ERROR
 */

#define TASK_ID_VERIFY(tcbId)                                                  \
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
    );

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
    );

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

extern STATUS taskDestroy(TCB_ID pTcb,
                          BOOL freeStack,
                          unsigned timeout,
                          BOOL forceDestroy);

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

extern STATUS taskUndelay(TCB_ID pTcb);
extern STATUS taskPrioritySet(TCB_ID pTcb, unsigned priority);
extern STATUS taskPriorityGet(TCB_ID pTcb, unsigned *priority);
extern STATUS taskRestart(TCB_ID pTcb);
extern void taskExit(int code);
extern STATUS taskLock(void);
extern STATUS taskUnlock(void);
extern STATUS taskSafe(void);
extern STATUS taskUnsafe(void);
extern TCB_ID taskIdSelf(void);

/******************************************************************************
 * taskTcb - Get TCB
 *
 * RETURNS: Task control block id or NULL
 */

TCB_ID taskTcb(
    int taskId
    );

extern void *taskStackAllot(TCB_ID pTcb, unsigned size);
extern STATUS taskIdVerify(TCB_ID pTcb);
extern int taskIdle(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskLib_h */

