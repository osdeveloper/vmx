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

/* wdShow.c - Watchdog show functions */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <os/classLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/wdLib.h>
#include <vmx/wdShow.h>

/* Defines */

/* Locals */
LOCAL char *wdStateName[] =
{
    "OUT_OF_Q",
    "IN_Q",
    "DEAD"
};

/* Globals */

/* Functions */

/******************************************************************************
 * wdShowInit - Initialize watchdog show functions
 *
 * RETURNS: N/A
 */

STATUS wdShowInit(
    void
    )
{
    return classShowConnect(wdClassId, (FUNCPTR) wdShow);
}

/******************************************************************************
 * wdShow - Show watchdog status
 *
 * RETURNS: OK or ERROR
 */

STATUS wdShow(
    WDOG_ID wdId
    )
{
    STATUS status;
    int level, ticks, state;
    FUNCPTR func;
    ARG arg;

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
        /* Get status */
        state = wdId->status;

        /* If in queue */
        if (state == WDOG_IN_Q)
        {
            ticks = Q_KEY(&tickQHead, &wdId->tickNode, 1);
            func = wdId->wdFunc;
            arg  = wdId->wdArg;
        }
        else
        {
            ticks = 0;
            func  = (FUNCPTR) NULL;
            arg   = (ARG) 0;
        }

        /* Unlock interrupts */
        INT_UNLOCK(level);

        /* Printf info */
        printf("\n");
        printf("Watchdog Id         : %-#10x\n", (int) wdId);
        printf("State               : %-10s\n", wdStateName[state & 0x03]);
        printf("Ticks Reaminig      : %-10d\n", ticks);
        printf("Routine             : %-#10x\n", (int) func);
        printf("Parameter           : %-#10x\n", (int) arg);
        printf("\n");
        status = OK;
    }

    return status;
}

