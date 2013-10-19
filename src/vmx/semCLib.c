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

/* semCLib.c - Counting sempahore library */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/errnoLib.h>
#include <vmx/classLib.h>
#include <vmx/private/kernLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/taskLib.h>
#include <vmx/sigLib.h>
#include <arch/intArchLib.h>
#include <vmx/semLib.h>

/* Locals */
LOCAL BOOL semCLibInstalled = FALSE;

LOCAL STATUS semCCoreInit(
    SEM_ID semId,
    int options,
    int initialCount
    );

LOCAL STATUS semCGive(
    SEM_ID semId
    );

LOCAL STATUS semCTake(
    SEM_ID semId,
    unsigned timeout
    );

LOCAL void semCGiveDefer(
    SEM_ID semId
    );

/******************************************************************************
 * semCLibInit - Initialize counting semaphore library
 *
 * RETURNS: OK
 */

STATUS semCLibInit(
    void
    )
{
    STATUS status;

    if (semCLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Install call methods */
        semGiveTable[SEM_TYPE_COUNTING]       = (FUNCPTR) semCGive;
        semTakeTable[SEM_TYPE_COUNTING]       = (FUNCPTR) semCTake;
        semFlushTable[SEM_TYPE_COUNTING]      = (FUNCPTR) semQFlush;
        semGiveDeferTable[SEM_TYPE_COUNTING]  = (FUNCPTR) semCGiveDefer;
        semFlushDeferTable[SEM_TYPE_COUNTING] = (FUNCPTR) semQFlushDefer;

        /* Mark library as installed */
        semCLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semCCreate - Allocate and init counting semaphore
 *
 * RETURNS: SEM_ID or NULL
 */

SEM_ID semCCreate(
    int options,
    int initialCount
    )
{
    SEM_ID semId;

    /* Check if lib is installed */
    if (semCLibInstalled != TRUE)
    {
        errnoSet(S_semLib_NOT_INSTALLED);
        semId = NULL;
    }
    else
    {
        /* Allocate memory */
        semId = (SEM_ID) objAlloc(semClassId);
        if (semId != NULL)
        {
            /* Initialze structure */
            if (semCInit(semId, options, initialCount) != OK)
            {
                objFree(semClassId, semId);
                semId = NULL;
            }
        }
    }

    return semId;
}
  
/******************************************************************************
 * semCInit - Init counting semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semCInit(
    SEM_ID semId,
    int options,
    int initialCount
    )
{
    STATUS status;

    /* Check if lib is installed */
    if (semCLibInstalled != TRUE)
    {
        errnoSet(S_semLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {
        /* Initialize semaphore queue */
        if (semQInit(&semId->qHead, options) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Initialize semaphore queue */
            if (semCCoreInit(semId, options, initialCount) != OK)
            {
                status = ERROR;
            }
            else
            {
                status = OK;
            }
        }
    }

   return OK;
}

/******************************************************************************
 * semCCoreInit - Init semaphore object core
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semCCoreInit(
    SEM_ID semId,
    int options,
    int initialCount
    )
{
    STATUS status;
    if (options & SEM_DELETE_SAFE)
    {
        errnoSet(S_semLib_INVALID_OPTION);
        status = ERROR;
    }
    else
    {
        /* Initialize variables */
        SEM_COUNT(semId) = initialCount;
        semId->recurse = 0;
        semId->options = options;
        semId->semType = SEM_TYPE_COUNTING;

        /* Initialize object core */
        objCoreInit(&semId->objCore, semClassId);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semCGive - Give up semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semCGive(
    SEM_ID semId
    )
{
    STATUS status;
    int level;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Verify class */
    if (OBJ_VERIFY(semId, semClassId) != OK)
    {
        INT_UNLOCK(level);
        status = ERROR;
    }
    else
    {
        /* Check if no more tasks are waiting for this semaphore */
        if (Q_FIRST(&semId->qHead) == NULL)
        {
            SEM_COUNT(semId)++;
            INT_UNLOCK(level);
            status = OK;
        }
        else
        {
            /* Enter kernel mode */
            kernelState = TRUE;
            INT_UNLOCK(level);

            /* Unblock next task waiting */
            vmxPendQGet(&semId->qHead);

            /* Exit kernel mode */
            kernExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * semCTake - Take hold of semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semCTake(
    SEM_ID semId,
    unsigned timeout
    )
{
    STATUS status;
    int level;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = OK;
    }
    else
    {
        /* Loop here if status is SIG_RESTART */
        do
        {
            /* Lock interrupts */
            INT_LOCK(level);

            /* Verify class */
            if (OBJ_VERIFY(semId, semClassId) != OK)
            {
                INT_UNLOCK(level);
                status = ERROR;
            }
            else
            {
                /* Check if taken recursively */
                if (SEM_COUNT(semId) > 0)
                {
                    SEM_COUNT(semId)--;
                    INT_UNLOCK(level);
                    status = OK;
                }
                else
                {
                    if (timeout == WAIT_NONE)
                    {
                        INT_UNLOCK(level);
                        errnoSet(S_objLib_UNAVAILABLE);
                        status = ERROR;
                    } 
                    else
                    {
                        /* Enter kernel mode */
                        kernelState = TRUE;
                        INT_UNLOCK(level);

                        /* Put on pending queue */
                        vmxPendQPut(&semId->qHead, timeout);

                        /* Exit through kernel */
                        status = kernExit();
                    }
                }
            }
        } while (status == SIG_RESTART);
    }

    return status;
}

/******************************************************************************
 * semCGiveDefer - Give semaphore as defered work
 *
 * RETURNS: N/A
 */

LOCAL void semCGiveDefer(
    SEM_ID semId
    )
{
    TCB_ID pOwner;

    /* Verify class */
    if (OBJ_VERIFY(semId, semClassId) == OK)
    {
        /* Check if it exists */
        if (Q_FIRST(&semId->qHead) == NULL)
        {
            /* Increase counter */
            SEM_COUNT(semId)++;
        }
        else
        {
            vmxPendQGet(&semId->qHead);
        }
    }
}

