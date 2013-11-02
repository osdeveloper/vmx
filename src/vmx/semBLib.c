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

/* semBLib.c - Binary sempahore library */

#include <vmx.h>
#include <arch/intArchLib.h>
#include <stdlib.h>

#include <vmx/errnoLib.h>
#include <vmx/classLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/taskLib.h>
#include <vmx/sigLib.h>
#include <arch/intArchLib.h>
#include <vmx/semLib.h>

/* Locals */
LOCAL BOOL semBLibInstalled = FALSE;

LOCAL STATUS semBCoreInit(
    SEM_ID semId,
    int options,
    SEM_B_STATE state
    );

LOCAL STATUS semBGive(
    SEM_ID semId
    );

LOCAL void semBGiveDefer(
    SEM_ID semId
    );

/******************************************************************************
 * semBTake - Take hold of semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semBTake(
    SEM_ID semId,
    unsigned timeout
    );

/******************************************************************************
 * semBLibInit - Initialize binary semaphore library
 *
 * RETURNS: OK
 */

STATUS semBLibInit(
    void
    )
{
    STATUS status;

    if (semBLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Install call methods */
        semGiveTable[SEM_TYPE_BINARY]       = (FUNCPTR) semBGive;
        semTakeTable[SEM_TYPE_BINARY]       = (FUNCPTR) semBTake;
        semFlushTable[SEM_TYPE_BINARY]      = (FUNCPTR) semQFlush;
        semGiveDeferTable[SEM_TYPE_BINARY]  = (FUNCPTR) semBGiveDefer;
        semFlushDeferTable[SEM_TYPE_BINARY] = (FUNCPTR) semQFlushDefer;

        /* Mark library as installed */
        semBLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semBCreate - Allocate and init semaphore
 *
 * RETURNS: SEM_ID or NULL
 */

SEM_ID semBCreate(
    int options,
    SEM_B_STATE state
    )
{
    SEM_ID semId;

    /* Check if lib is installed */
    if (semBLibInstalled != TRUE)
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
            if (semBInit(semId, options, state) != OK)
            {
                objFree(semClassId, semId);
                semId = NULL;
            }
        }
    }

    return semId;
}
  
/******************************************************************************
 * semBInit - Init semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semBInit(
    SEM_ID semId,
    int options,
    SEM_B_STATE state
    )
{
    STATUS status;

    /* Check if lib is installed */
    if (semBLibInstalled != TRUE)
    {
        errnoSet(S_semLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {
        if (options & SEM_DELETE_SAFE)
        {
            errnoSet(S_semLib_INVALID_OPTION);
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
                if (semBCoreInit(semId, options, state) != OK)
                {
                    status = ERROR;
                }
                else
                {
                    status = OK;
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * semBCoreInit - Init semaphore object core
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semBCoreInit(
    SEM_ID semId,
    int options,
    SEM_B_STATE state
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
        /* Setup state */
        switch(state)
        {
            case SEM_EMPTY:
                SEM_OWNER_SET(semId, taskIdCurrent);
                status = OK;
                break;

            case SEM_FULL:
                SEM_OWNER_SET(semId, NULL);
                status = OK;
                break;

            default: 
                errnoSet(S_semLib_INVALID_OPTION);
                status = ERROR;
        }

        if (status == OK)
        {
            /* Initialize variables */
            semId->recurse = 0;
            semId->options = options;
            semId->semType = SEM_TYPE_BINARY;

            /* Initialize object core */
            objCoreInit(&semId->objCore, semClassId);
        }
    }

    return status;
}

/******************************************************************************
 * semBGive - Give up semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semBGive(
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
        /* Get next listening task from queue */
        SEM_OWNER_SET(semId, Q_FIRST(&semId->qHead));

        /* Check if no more tasks are waiting for this semaphore */
        if (SEM_OWNER_GET(semId) == NULL)
        {
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
            vmxExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * semBTake - Take hold of semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semBTake(
    SEM_ID semId,
    unsigned timeout
    )
{
    STATUS status;
    int level;

    if (INT_RESTRICT() != OK)
    {
        errnoSet (S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
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
                /* Check if it is already given back */
                if (SEM_OWNER_GET(semId) == NULL)
                {
                    /* Then take it */
                    SEM_OWNER_SET(semId, taskIdCurrent);

                    /* Unlock interrupts */
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
                        status = vmxExit();
                    }
                }
            }
        } while(status == SIG_RESTART);
    }

    return status;
}

/******************************************************************************
 * semBGiveDefer - Give semaphore as defered work
 *
 * RETURNS: N/A
 */

LOCAL void semBGiveDefer(
    SEM_ID semId
    )
{
    TCB_ID pOwner;

    /* Verify class */
    if (OBJ_VERIFY(semId, semClassId) == OK)
    {
        /* Get task id */
        pOwner = SEM_OWNER_GET(semId);

        /* Set to next owner */
        SEM_OWNER_SET(semId, Q_FIRST(&semId->qHead));

        /* Check if it exists */
        if (SEM_OWNER_GET(semId) != NULL)
        {
            vmxPendQGet(&semId->qHead);
        }
    }
}

