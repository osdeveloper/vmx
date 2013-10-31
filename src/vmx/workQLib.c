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

/* workQLib.c - Kernel work queue library */

/*
The kernel queue is implemented as a ring buffer.
*/

/* includes */

#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/sysArchLib.h>
#include <vmx/errnoLib.h>
#include <vmx/workQLib.h>

/* defines */

#define MAX_WORK_Q_JOBS        256    /* Always a power of two. */

/* structs */

typedef struct
{
    FUNCPTR func;    /* ptr to function that was added */
    int     numArgs; /* # of arguments to added item */
    ARG     arg1;    /* 1st argument of added function (if applicable) */
    ARG     arg2;    /* 2nd argument of added function (if applicable) */
} WORK_Q_JOB;

/* globals */
volatile BOOL workQIsEmpty;

/* locals */
LOCAL volatile unsigned workQReadIndex;
LOCAL volatile unsigned workQWriteIndex;
LOCAL WORK_Q_JOB        workQJobs[MAX_WORK_Q_JOBS];

/******************************************************************************
 * workQLibInit - Initialize kernel work queue
 *
 * RETURNS: OK
 */

STATUS workQLibInit(
    void
    )
{
    workQReadIndex = 0;
    workQWriteIndex = 0;
    workQIsEmpty = TRUE;

    return OK;
}

/******************************************************************************
 * workQAdd0 - Add a kernel job with no aguments
 *
 * RETURNS: N/A
 */

void workQAdd0(
    FUNCPTR func    /* ptr to deferred function */
    )
{
    int oldLevel;                 /* previous interrupt lock level */
    FAST WORK_Q_JOB *pJob;        /* ptr to job item */

    INT_LOCK(oldLevel);           /* LOCK INTERRUPTS */

    /* Get place to store job in buffer */
    pJob = (WORK_Q_JOB *) &workQJobs[workQWriteIndex];

    workQWriteIndex++;                   /* Advance ring buffer write index */
    workQWriteIndex &= (MAX_WORK_Q_JOBS - 1);

    if (workQWriteIndex == workQReadIndex)
    {
        workQPanic();
    }

    INT_UNLOCK(oldLevel);                /* UNLOCK INTERRUPTS */

    /* Queue not empty */
    workQIsEmpty = FALSE;

    /* Store data */
    pJob->func = func;
    pJob->numArgs = 0;
}

/******************************************************************************
 * workQAdd1 - Add a kernel job with one agument
 *
 * RETURNS: N/A
 */

void workQAdd1(
    FUNCPTR func,     /* ptr to function to add */
    ARG arg1          /* 1st argument to deferred function */
    )
{
    int oldLevel;
    FAST WORK_Q_JOB *pJob;

    INT_LOCK(oldLevel);                  /* LOCK INTERRUPTS */

    /* Get place to store job in buffer */
    pJob = (WORK_Q_JOB *) &workQJobs[workQWriteIndex];

    workQWriteIndex++;                   /* Advance ring buffer write index */
    workQWriteIndex &= (MAX_WORK_Q_JOBS - 1);

    if (workQWriteIndex == workQReadIndex)
    {
        workQPanic();
    }

    INT_UNLOCK(oldLevel);                /* UNLOCK INTERRUPTS */

    /* Queue not empty */
    workQIsEmpty = FALSE;

    /* Store data */
    pJob->func = func;
    pJob->arg1 = arg1;
    pJob->numArgs = 1;
}

/******************************************************************************
 * workQAdd2 - Add a kernel job with two aguments
 *
 * RETURNS: N/A
 */

void workQAdd2(
    FUNCPTR func,     /* ptr to function to add */
    ARG arg1,         /* 1st argument to deferred function */
    ARG arg2          /* 2nd argument to deferred function */
    )
{
    int oldLevel;
    FAST WORK_Q_JOB *pJob;

    INT_LOCK(oldLevel);                  /* LOCK INTERRUPTS */

    /* Get place to store job in buffer */
    pJob = (WORK_Q_JOB *) &workQJobs[workQWriteIndex];

    workQWriteIndex++;                   /* Advance ring buffer write index */
    workQWriteIndex &= (MAX_WORK_Q_JOBS - 1);

    if (workQWriteIndex == workQReadIndex)
    {
        workQPanic();
    }

    INT_UNLOCK(oldLevel);                /* UNLOCK INTERRUPTS */

    /* Queue not empty */
    workQIsEmpty = FALSE;

    /* Store data */
    pJob->func = func;
    pJob->arg1 = arg1;
    pJob->arg2 = arg2;
    pJob->numArgs = 2;
}

/******************************************************************************
 * workQDoWork - Do job on the kernel queue
 *
 * RETURNS: N/A
 */

void workQDoWork(
    void
    )
{
    FAST WORK_Q_JOB *pJob;
    int keepErrno;
    int level;

    /* Store current errno */
    keepErrno = errno;
    INT_LOCK(level);

    /*
     * Do all jobs on store.
     * As <kernelState> is TRUE, more jobs can still come in.  Thus, it
     * is important to lock interrupts while mucking with the indices.
     */

    while (workQReadIndex != workQWriteIndex)
    {
        /* Get next job */
        pJob = (WORK_Q_JOB *) &workQJobs[workQReadIndex];

        workQReadIndex++;                  /* Advance ring buffer read index */
        workQReadIndex &= (MAX_WORK_Q_JOBS - 1);

        INT_UNLOCK(level);

        /* Call job procedure (<kernelState> is TRUE!) */
        switch(pJob->numArgs)
        {
            case 1:
                (FUNCPTR *) (pJob->func)(pJob->arg1);
                break;
            case 2:
                (FUNCPTR *) (pJob->func)(pJob->arg1, pJob->arg2);
                break;
            default:
                (FUNCPTR *) (pJob->func)();
                break;
        }

        INT_LOCK(level);   /* Lock interrupts again. */
    }

    workQIsEmpty = TRUE;   /* Queue must be empty */
    INT_UNLOCK(level);     /* Restore interrupts */

    errno = keepErrno;     /* Restore errno */
}

/******************************************************************************
 * workQPanic - Fatal error in queue
 *
 * RETURNS: N/A
 */

void workQPanic(
    void
    )
{
    sysReboot();
}

