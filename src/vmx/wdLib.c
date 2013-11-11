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

/* wdLib.c - Watchdog library */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/workQLib.h>
#include <vmx/taskLib.h>
#include <os/classLib.h>
#include <os/memPartLib.h>
#include <vmx/wdLib.h>

/* Locals */
LOCAL BOOL      wdLibInstalled = FALSE;
LOCAL OBJ_CLASS wdClass;

/* Globals */
CLASS_ID wdClassId = &wdClass;

/* Functions */

/******************************************************************************
 * wdLibInit - Initialize wachdog library
 *
 * RETURNS: OK or ERROR
 */

STATUS wdLibInit(
    void
    )
{
    STATUS status;

    /* Check if installed */
    if (wdLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Initialize class */
        if (classInit(
                wdClassId,
                sizeof(WDOG),
                OFFSET(WDOG, objCore),
                memSysPartId,
                (FUNCPTR) wdCreate,
                (FUNCPTR) wdInit,
                (FUNCPTR) wdDestroy
                ) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Mark as installed */
            wdLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * wdCreate - Create watchdog timer
 *
 * RETURNS: Whatchdog timer id or NULL
 */

WDOG_ID wdCreate(
    void
    )
{
    WDOG_ID wdId;

    if (wdLibInstalled != TRUE)
    {
        wdId = NULL;
    }
    else
    {
        /* Allocate struct */
        wdId = (WDOG_ID) objAlloc(wdClassId);
        if (wdId != NULL)
        {
            /* Initialize object */
            if (wdInit(wdId) != OK)
            {
                objFree(wdClassId, wdId);
                wdId = NULL;
            }
        }
    }

    return wdId;
}

/******************************************************************************
 * wdInit - Inititalize watchdog timer
 *
 * RETURNS: OK or ERROR
 */

STATUS wdInit(
    WDOG_ID wdId
    )
{
    STATUS status;

    if (wdLibInstalled != TRUE)
    {
        status = ERROR;
    }
    else
    {
        wdId->status        = WDOG_OUT_OF_Q;
        wdId->dfrStartCount = 0;

        objCoreInit(&wdId->objCore, wdClassId);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * wdDestroy - Free watchdog timer
 *
 * RETURNS: OK or ERROR
 */

STATUS wdDestroy(
    WDOG_ID wdId,
    BOOL dealloc
    )
{
    STATUS status;
    int level;

    /* Not callable from interrupts */
    if (INT_RESTRICT() != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Lock interrupts */
        INT_LOCK(level);

        /* Verify object */
        if (OBJ_VERIFY(wdId, wdClassId) != OK )
        {
            INT_UNLOCK(level);
            status = ERROR;
        }
        else
        {
            /* Terminate object */
            objCoreTerminate(&wdId->objCore);

            /* Enter kernel */
            kernelState = TRUE;

            /* Unlock interrupts */
            INT_UNLOCK(level);

            /* Cancel watchdog timer */
            vmxWdCancel(wdId);
            wdId->status = WDOG_DEAD;

            taskSafe();

            /* Exit kernel */
            vmxExit();

            /* Deallocate if requested */
            if (dealloc == TRUE)
            {
                objFree(wdClassId, wdId);
            }

            taskUnsafe();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * wdDelete - Terminate and deallocate watchdog timer
 *
 * RETURNS: OK or ERROR
 */

STATUS wdDelete(
    WDOG_ID wdId
    )
{
    return wdDestroy(wdId, TRUE);
}

/******************************************************************************
 * wdTerminate - Terminate watchdog timer
 *
 * RETURNS: OK or ERROR
 */

STATUS wdTerminate(
    WDOG_ID wdId
    )
{
    return wdDestroy(wdId, FALSE);
}

/******************************************************************************
 * wdStart - Start watchdog timer
 *
 * RETURNS: OK or ERROR
 */

STATUS wdStart(
    WDOG_ID wdId,
    int delay,
    FUNCPTR func,
    ARG arg
    )
{
    STATUS status;
    int level;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Verify object */
    if (OBJ_VERIFY(wdId, wdClassId) != OK)
    {
        INT_UNLOCK(level);
        status = ERROR;
    }
    else
    {
        /* If in kernel mode */
        if (kernelState == TRUE)
        {
            wdId->dfrStartCount++;
            wdId->wdFunc = func;
            wdId->wdArg  = arg;

            /* Unlock interrupts */
            INT_UNLOCK(level);

            /* Add to kernel queue */
            workQAdd2((FUNCPTR) vmxWdStart, (ARG) wdId, (ARG) delay);
            status = OK;
        }
        else
        {
            wdId->dfrStartCount++;
            wdId->wdFunc = func;
            wdId->wdArg  = arg;

            /* Enter kernel */
            kernelState = TRUE;

            /* Unlock interrupts */
            INT_UNLOCK(level);

            /* Start watchdog timer */
            if (vmxWdStart(wdId, delay) != OK)
            {
                vmxExit();
                status = ERROR;
            }
            else
            {
                /* Exit kernel */
                vmxExit();
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * wdCancel - Cancel watchdog timer
 *
 * RETURNS: OK or ERROR
 */

STATUS wdCancel(
    WDOG_ID wdId
    )
{
    STATUS status;
    int level;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Verify object */
    if (OBJ_VERIFY(wdId, wdClassId) != OK)
    {
        INT_UNLOCK(level);
        status = ERROR;
    }
    else
    {
        /* If in kernel mode */
        if (kernelState == TRUE)
        {
            /* Unlock interrupts */
            INT_UNLOCK(level);

            /* Add to kernel queue */
            workQAdd1((FUNCPTR) vmxWdCancel, (ARG) wdId);
            status = OK;
        }
        else
        {
            /* Enter kernel */
            kernelState = TRUE;

            /* Unlock interrupts */
            INT_UNLOCK(level);

            vmxWdCancel(wdId);

            /* Exit kernel */
            vmxExit();
            status = OK;
        }

    }

    return status;
}

