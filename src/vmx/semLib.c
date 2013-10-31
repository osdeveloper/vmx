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

/* semLib.c - Sempahore library */

#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <util/qFifoLib.h>
#include <util/qPrioLib.h>
#include <vmx/objLib.h>
#include <vmx/classLib.h>
#include <vmx/private/kernLibP.h>
#include <vmx/workQLib.h>
#include <vmx/vmxLib.h>
#include <vmx/taskLib.h>
#include <vmx/memPartLib.h>
#include <vmx/semLib.h>

/* Locals */
LOCAL BOOL semLibInstalled = FALSE;

/* Internals */
OBJ_CLASS semClass;

/* Forward declarations */
LOCAL STATUS semGiveDefer(
    SEM_ID semId
    );

LOCAL STATUS semFlushDefer(
    SEM_ID semId
    );

FUNCPTR semGiveTable[MAX_SEM_TYPE] =
{
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid
};

FUNCPTR semTakeTable[MAX_SEM_TYPE] =
{
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid
};

FUNCPTR semFlushTable[MAX_SEM_TYPE] =
{
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid
};

FUNCPTR semGiveDeferTable[MAX_SEM_TYPE] =
{
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid
};

FUNCPTR semFlushDeferTable[MAX_SEM_TYPE] =
{
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid,
    (FUNCPTR) semInvalid
};

/* Externals */
CLASS_ID semClassId = &semClass;

/******************************************************************************
 * semLibInit - Initialize semaphore library
 *
 * RETURNS: OK or ERROR
 */

STATUS semLibInit(
    void
    )
{
    STATUS status;

    /* Install library */
    if (semLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        if (classInit(
                semClassId,
                sizeof(SEMAPHORE),
                OFFSET(SEMAPHORE, objCore),
                memSysPartId,
                (FUNCPTR) NULL,
                (FUNCPTR) NULL,
                (FUNCPTR) NULL
                ) != OK)
        {
            status = ERROR;
        }
        else
        {
            semLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * semCreate - Create a semaphore
 *
 * RETURNS: Semaphore Id or NULL
 */

SEM_ID semCreate(
    int type,
    int options
    )
{
    SEM_ID semId;

    if (semLibInstalled != TRUE)
    {
        errnoSet(S_semLib_NOT_INSTALLED);
        semId = NULL;
    }
    else
    {
        /* Create semaphore of correct type */
        switch(type & SEM_TYPE_MASK)
        {
            case SEM_TYPE_BINARY:
                semId = semBCreate(options, SEM_FULL);
                break;

            case SEM_TYPE_MUTEX:
                semId = semMCreate(options);
                break;

            case SEM_TYPE_COUNTING:
                semId = semCCreate(options, 1);
                break;

            default:
                semId = NULL;
                errnoSet(S_semLib_INVALID_OPTION);
                break;
        }
    }

    return semId;
}

/******************************************************************************
 * semInit - Initialize a semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semInit(
    SEM_ID semId,
    int type,
    int options
    )
{
    STATUS status;

    if (semLibInstalled != TRUE)
    {
        errnoSet(S_semLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {
        /* Create semaphore of correct type */
        switch(type & SEM_TYPE_MASK)
        {
            case SEM_TYPE_BINARY:
                status = semBInit(semId, options, SEM_FULL);
                break;

            case SEM_TYPE_MUTEX:
                status = semMInit(semId, options);
                break;

            case SEM_TYPE_COUNTING:
                status = semCInit(semId, options, 1);
                break;

            default:
                status = ERROR;
                errnoSet(S_semLib_INVALID_OPTION);
                break;
        }
    }

    return status;
}

/******************************************************************************
 * semDelete - Delete semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semDelete(
    SEM_ID semId
    )
{
    return semDestroy(semId, TRUE);
}

/******************************************************************************
 * semTerminate - Terminate semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semTerminate(
    SEM_ID semId
    )
{
    return semDestroy(semId, FALSE);
}

/******************************************************************************
 * semDestroy - Destroy semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semDestroy(
    SEM_ID semId,
    BOOL deallocate
    )
{
    STATUS status;
    int level;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        INT_LOCK(level);

        if (OBJ_VERIFY(semId, semClassId) != OK)
        {
            INT_UNLOCK(level);
            status = ERROR;
        }
        else
        {
            objCoreTerminate(&semId->objCore);

            /* Delete it */
            kernelState = TRUE;
            INT_UNLOCK(level);
            vmxSemDelete(semId);

            taskSafe();
            vmxExit();

            if (deallocate == TRUE)
            {
                objFree(semClassId, semId);
            }

            taskUnsafe();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * semGive - Give up semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semGive(
    SEM_ID semId
    )
{
    STATUS status;

    if (kernelState == TRUE)
    {
        status = semGiveDefer(semId);
    }
    else
    {
        status = (*semGiveTable[semId->semType & SEM_TYPE_MASK])(semId);
    }

    return status;
}

/******************************************************************************
 * semTake - Take hold of semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semTake(
    SEM_ID semId,
    unsigned timeout
    )
{
    STATUS status;

    status = (*semTakeTable[semId->semType & SEM_TYPE_MASK])(semId, timeout);

    return status;
}

/******************************************************************************
 * semFlush - Flush all tasks depending on semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semFlush(
    SEM_ID semId
    )
{
    STATUS status;

    if (kernelState == TRUE)
    {
        status = semFlushDefer(semId);
    }
    else
    {
        status = (*semFlushTable[semId->semType & SEM_TYPE_MASK])(semId);
    }

    return status;
}

/******************************************************************************
 * semGiveDefer - Give sempahore defered
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semGiveDefer(
    SEM_ID semId
    )
{
    STATUS status;

    /* Verify object */
    if (OBJ_VERIFY(semId, semClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* A method is guaranteed to exist. */
        workQAdd1(semGiveDeferTable[semId->semType & SEM_TYPE_MASK], semId);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semFlushDefer - Flush all tasks depending on semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semFlushDefer(
    SEM_ID semId
    )
{
    STATUS status;

    /* Verify object */
    if (OBJ_VERIFY(semId, semClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* A method is guaranteed to exist */
        workQAdd1(semFlushDeferTable[semId->semType & SEM_TYPE_MASK], semId);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semQInit - Initialize semaphore queue
 *
 * RETURNS: OK or ERROR
 */

STATUS semQInit(
    Q_HEAD *pQHead,
    int options
    )
{
    STATUS status;

    /* Initilaize queue according to options */
    switch (options & SEM_Q_MASK)
    {
        case SEM_Q_FIFO:
            qInit(pQHead, qFifoClassId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            status = OK;
            break;

        case SEM_Q_PRIORITY:
            qInit(pQHead, qPrioClassId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            status = OK;
            break;

        default:
            errnoSet(S_semLib_INVALID_Q_TYPE);
            status = ERROR;
    }

    return status;
}

/******************************************************************************
 * semQFlush - Flush semaphore queue
 *
 * RETURNS: OK or ERROR
 */

STATUS semQFlush(
    SEM_ID semId
    )
{
    STATUS status;
    int level;

    INT_LOCK(level);

    if (OBJ_VERIFY (semId, semClassId) != OK)
    {
        INT_UNLOCK (level);
        status = ERROR;
    }

    /* Check next object */
    if (Q_FIRST(&semId->qHead) == NULL)
    {
        INT_UNLOCK(level);
    }
    else
    {
        /* Enter kernel and flush pending queue */
        kernelState = TRUE;
        INT_UNLOCK(level);
        vmxPendQFlush(&semId->qHead);
        vmxExit();
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semQFlushDefer - Flush semaphore queue in defered mode
 *
 * RETURNS: N/A
 */

void semQFlushDefer(
    SEM_ID semId
    )
{
    /* Check if flush needed */
    if (Q_FIRST(&semId->qHead) != NULL)
    {
        vmxPendQFlush(&semId->qHead);
    }
}

/******************************************************************************
 * semInvalid - Invalid semaphore function
 *
 * RETURNS: ERROR
 */

STATUS semInvalid(
    SEM_ID semId
    )
{
    return ERROR;
}

