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

/* semMLib.c - Mutex sempahore library */

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
LOCAL BOOL semMLibInstalled = FALSE;

LOCAL STATUS semMCoreInit(
    SEM_ID semId,
    int options
    );

LOCAL STATUS semMGive(
    SEM_ID semId
    );

LOCAL STATUS semMKernGive(
    SEM_ID semId,
    int kernWork
    );
 
LOCAL STATUS semMTake(
    SEM_ID semId,
    unsigned timeout
    );

/******************************************************************************
 * semMLibInit - Initialize mutex semaphore library
 *
 * RETURNS: OK
 */

STATUS semMLibInit(
    void
    )
{
    STATUS status;

    if (semMLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Install call methods */
        semGiveTable[SEM_TYPE_MUTEX] = (FUNCPTR) semMGive;
        semTakeTable[SEM_TYPE_MUTEX] = (FUNCPTR) semMTake;

        /* Mark library as installed */
        semMLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * semMCreate - Allocate and init mutex semaphore
 *
 * RETURNS: SEM_ID or NULL
 */

SEM_ID semMCreate(
    int options
    )
{
    SEM_ID semId;

    /* Check if lib is installed */
    if (semMLibInstalled != TRUE)
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
            if (semMInit(semId, options) != OK)
            {
                objFree(semClassId, semId);
                semId = NULL;
            }
        }
    }

    return semId;
}
  
/******************************************************************************
 * semMInit - Init mutex semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semMInit(
    SEM_ID semId,
    int options
    )
{
    STATUS status;

    /* Check if lib is installed */
    if (semMLibInstalled != TRUE)
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
            if (semMCoreInit(semId, options) != OK)
            {
                status = ERROR;
            }
            else
            {
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * semMCoreInit - Init semaphore object core
 *
 * RETURNS: OK
 */

LOCAL STATUS semMCoreInit(
    SEM_ID semId,
    int options
    )
{
    /* Initialize variables */
    SEM_OWNER_SET(semId, NULL);
    semId->recurse = 0;
    semId->options = options;
    semId->semType = SEM_TYPE_MUTEX;

    /* Initialize object core */
    objCoreInit(&semId->objCore, semClassId);

    return OK;
}

/******************************************************************************
 * semMGive - Give up semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semMGive(
    SEM_ID semId
    )
{
    STATUS status;
    int level;
    int kernWork = 0x00;
    TCB_ID pOwner;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        INT_LOCK(level);

        /* Verify object */
        if (OBJ_VERIFY(semId, semClassId) != OK)
        {
            INT_UNLOCK(level);
            status = ERROR;
        }
        else
        {
            pOwner = SEM_OWNER_GET(semId);

            /* Check ownership */
            if (taskIdCurrent != pOwner)
            {
                INT_UNLOCK(level);
                errnoSet(S_semLib_NOT_OWNER);
                status = ERROR;
            }
            else
            {
                /* Check recurse */
                if (semId->recurse > 0)
                {
                    semId->recurse--;
                    INT_UNLOCK(level);
                    status = OK;
                }
                else
                {
                    /* Update to next pending task */
                    SEM_OWNER_SET(semId, Q_FIRST(&semId->qHead));
                    pOwner = SEM_OWNER_GET(semId);

                    /* Check for more kernel work */
                    if (pOwner != NULL)
                    {
                        kernWork |= SEM_M_Q_GET;
                    }

                    if ((semId->options & SEM_DELETE_SAFE) &&
                        (--taskIdCurrent->safeCount == 0) &&
                        (Q_FIRST(&taskIdCurrent->safetyQ) != NULL))
                    {
                        kernWork |= SEM_M_SAFE_Q_FLUSH;
                    }

                    /* Check if where is any kernel work to be done */
                    if (!kernWork)
                    {
                        INT_UNLOCK(level);
                        status = OK;
                    }
                    else
                    {
                        kernelState = TRUE;
                        INT_UNLOCK(level);
                        status = semMKernGive(semId, kernWork);
                    }
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * semMTake - Take hold of semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semMTake(
    SEM_ID semId,
    unsigned timeout
    )
{
    STATUS status;
    int level;
    TCB_ID pOwner;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        /* Loop here while status is SIG_RESTART */
        do
        {
            INT_LOCK(level);

            /* Verify object */
            if (OBJ_VERIFY(semId, semClassId) != OK)
            {
                INT_UNLOCK(level);
                status = ERROR;
            }
            else
            {
                /* Take semaphore if free */
                pOwner = SEM_OWNER_GET(semId);
                if (pOwner == NULL)
                {
                    SEM_OWNER_SET(semId, taskIdCurrent);

                    if (semId->options & SEM_DELETE_SAFE)
                    {
                        taskIdCurrent->safeCount++;
                    }

                    INT_UNLOCK(level);
                    status = OK;
                }
                else
                {
                    /* Check recurse */
                    if (pOwner == taskIdCurrent)
                    {
                        semId->recurse++;
                        INT_UNLOCK(level);
                        status = OK;
                    }
                    else
                    {
                        if (timeout == WAIT_NONE)
                        {
                            errnoSet(S_objLib_UNAVAILABLE);
                            status = ERROR;
                        }
                        else
                        {
                            /* Do kernel work */
                            kernelState = TRUE;
                            INT_UNLOCK(level);
                            vmxPendQPut(&semId->qHead, timeout);
                            status = kernExit();
                        }
                    }
                }
            }
        } while (status == SIG_RESTART);
    }

    return status;
}

/******************************************************************************
 * semMGiveForce - forcibly give a mutex (for debugging only)
 *
 * This routine forcibly releases a mutex semaphore.  It passes ownership to
 * the next in the queue (if any).  
 *
 * RETURNS: OK or ERROR
 */

STATUS semMGiveForce(
    SEM_ID  semId      /* mutex semaphore to forcibly give */
    )
{
    STATUS status;
    int level;
    TCB_ID pOwner;
    TCB_ID pNewOwner;
    BOOL taskIsValid;

    if (INT_RESTRICT() != OK)
    {
        errnoSet (S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        INT_LOCK(level);    /* Lock interrupts */

        if (OBJ_VERIFY (semId, semClassId) != OK)
        {
            INT_UNLOCK (level);
            status = ERROR;
        }
        else
        {
            semId->recurse = 0;         /* Quickly undo any recursive "takes" */

            pOwner = semId->semOwner;

            if (taskIdCurrent == pOwner)   /* If the semaphore is owned by  */
            {                              /* the current task, unlock ints */
                INT_UNLOCK (level);        /* and use the standard API to   */
                status = semMGive(semId);  /* give the semaphore.           */
            }
            else
            {
                /*
                 * The semaphore is owned by another task. Before using <pOwner>
                 * verify that it is still a task since it is possible that the
                 * task that took this semaphore may no longer exist.
                 *
                 * Afterwards it will be necessary to mark <kernelState> as TRUE
                 * and unlock interrupts.  Since <kernelState> exists to
                 * decrease the interrupt latency period, we try to keep queue
                 * modifications (operations that take un-fixed amount of time)
                 * in this section.
                 */
                taskIsValid = (TASK_ID_VERIFY (pOwner) == OK);

                kernelState = TRUE;
                INT_UNLOCK (level);

                if (taskIsValid)
                {
                    if ((semId->options & SEM_DELETE_SAFE) &&
                        (--pOwner->safeCount == 0) &&
                        (Q_FIRST (&pOwner->safetyQ) != NULL))
                    {
                        vmxPendQFlush (&pOwner->safetyQ);
                    }
                }

                /* Update to next pending task */
                pNewOwner = (TCB_ID) Q_FIRST (&semId->qHead);
                if ((semId->semOwner = pNewOwner) != NULL)
                {
                    vmxPendQGet(&semId->qHead);

                    if (semId->options & SEM_DELETE_SAFE)
                    {
                        pNewOwner->safeCount++;
                    }
                }

                kernExit();
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * semMKernGive - Do kernel stuff for semaphore give
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semMKernGive(
    SEM_ID semId,
    int kernWork
    )
{
    STATUS status;
    TCB_ID pOwner;

    /* Get from pending queue */
    if (kernWork & SEM_M_Q_GET)
    {
        vmxPendQGet(&semId->qHead);

        /* Get owner */
        pOwner = SEM_OWNER_GET(semId);

        if (semId->options & SEM_DELETE_SAFE)
        {
            pOwner->safeCount++;
        }
    }

    /* Should safe queue be flushed */
    if (kernWork & SEM_M_SAFE_Q_FLUSH)
    {
        vmxPendQFlush(&taskIdCurrent->safetyQ);
    }

    status = kernExit();

    return(status);
}

