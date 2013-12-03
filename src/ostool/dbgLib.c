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

/* dbgLib.c - Debug library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <os/ioLib.h>
#include <os/tyLib.h>
#include <os/excLib.h>
#include <ostool/dbgLib.h>

/* Defines */

/* Imports */
IMPORT int shellTaskId;

/* Locals */
LOCAL BOOL dbgLibInstalled = FALSE;

LOCAL void dbgTyAbort(
    void
    );

LOCAL void dbgTaskTyAbort(
    void
    );

/* Globals */

/* Functions */

/******************************************************************************
 * dbgLibInit - Initialize debug library
 *
 * RETURNS: OK
 */

STATUS dbgLibInit(
    void
    )
{
    STATUS status;

    /* Check if already installed */
    if (dbgLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        tyAbortFuncSet((FUNCPTR) dbgTyAbort);

        /* Mark as installed */
        dbgLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * dbgTyAbort - Abort shell with terminal abort key
 *
 * RETURNS: N/A
 */

LOCAL void dbgTyAbort(
    void
    )
{
    /* Generate exception to restart shell */
    excJobAdd(
        (VOIDFUNCPTR) dbgTaskTyAbort,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0,
        (ARG) 0
        );
}

/******************************************************************************
 * dbgTaskTyAbort - Task shell abort function
 *
 * RETURNS: N/A
 */

LOCAL void dbgTaskTyAbort(
    void
    )
{
    /* Temprary remove abort function */
    tyAbortFuncSet((FUNCPTR) NULL);

    /* Flush standard I/O */
    ioctl(STDIN_FILENO, FIOFLUSH, 0);
    ioctl(STDOUT_FILENO, FIOFLUSH, 0);
    ioctl(STDERR_FILENO, FIOFLUSH, 0);

    /* If shell restart fails */
    if (taskRestart(shellTaskId) != OK)
    {
        fprintf(stderr, "spawning new shell.\n");

        /* Start a new shell */
        if (shellSpawn(0, (ARG) TRUE) != OK)
        {
            fprintf(stderr, "shell spawn failed.\n");
        }
    }
    else
    {
        fprintf(stderr, "%s restarted.\n", taskName(shellTaskId) );
    }

    /* Restore shell abort function */
    tyAbortFuncSet((FUNCPTR) dbgTyAbort);
}

