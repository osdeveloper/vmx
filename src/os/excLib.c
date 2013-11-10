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

/* excLib.c - Exception library */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/taskLib.h>
#include <vmx/msgQLib.h>
#include <os/excLib.h>

/* Globals */
int excTaskId        = 0;
int excTaskPriority  = 0;
int excTaskOptions   = TASK_OPTIONS_UNBREAKABLE;
int excTaskStackSize = 8000;
int excMsgsLost      = 0;

/* Locals */
LOCAL BOOL     excLibInstalled = FALSE;
LOCAL MSG_Q_ID excMsgQId;

LOCAL void excTask(
    void
    );

/******************************************************************************
 * excLibInit - Initialize exception library
 *
 * RETURNS: OK or ERROR
 */

STATUS excLibInit(
    void
    )
{
    STATUS status;

    /* Check if already running */
    if (excLibInstalled == TRUE)
    {
        if (excTaskId == 0)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }
    else
    {
        /* Create exception message queue */
        excMsgQId = msgQCreate(EXC_MAX_MSGS, sizeof(EXC_MSG), MSG_Q_FIFO);
        if (excMsgQId == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Start exception task */
            excTaskId = taskSpawn(
                            "tExcTask",
                            excTaskPriority,
                            excTaskOptions,
                            excTaskStackSize,
                            (FUNCPTR) excTask,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0,
                            (ARG) 0
                            );
            if (excTaskId == 0)
            {
                status = ERROR;
            }
            else
            {
                /* Mark as intalled */
                excLibInstalled = TRUE;
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * excJobAdd - Add work to exception task
 *
 * RETURNS: OK or ERROR
 */

STATUS excJobAdd(
    VOIDFUNCPTR func,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5
    )
{
    STATUS status;
    int wt;
    EXC_MSG message;

    if (excLibInstalled != TRUE)
    {
        status = ERROR;
    }
    else
    {
        /* Setup work structure */
        message.func   = func;
        message.arg[0] = arg0;
        message.arg[1] = arg1;
        message.arg[2] = arg2;
        message.arg[3] = arg3;
        message.arg[4] = arg4;
        message.arg[5] = arg5;

        /* Select wait method */
        if (INT_CONTEXT() == TRUE)
        {
            wt = WAIT_NONE;
        }
        else
        {
            wt = WAIT_FOREVER;
        }

        /* Send message to queue */
        if (msgQSend(
                excMsgQId,
                &message,
                sizeof(message),
                wt,
                MSG_PRI_NORMAL
                ) != OK)
        {
            ++excMsgsLost;
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * excTask - Exception task
 *
 * RETURNS: N/A
 */

LOCAL void excTask(
    void
    )
{
    static int oldLost = 0;
    int newLost;
    EXC_MSG message;

    /* Endless loop */
    while (1)
    {
        /* Get for work from message queue */
        if (msgQReceive(excMsgQId, &message, sizeof(message), WAIT_FOREVER) ==
            sizeof(message))
        {
            (*message.func)(
                message.arg[0],
                message.arg[1],
                message.arg[2],
                message.arg[3],
                message.arg[4],
                message.arg[5]
                );
        }

        /* Check if more calls where lost */
        if ((newLost = excMsgsLost) != oldLost)
        {
            oldLost = newLost;
        }
    }
}

/******************************************************************************
 * printErr - Print error message
 *
 * RETURNS: Return from printf
 */

int printErr(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4
    )
{
    int ret;

    ret = (int) fprintf(
                    stderr,
                    fmt,
                    (ARG) arg0,
                    (ARG) arg1,
                    (ARG) arg2,
                    (ARG) arg3,
                    (ARG) arg4
                    );

    return ret;
}

/******************************************************************************
 * printExc - Print exception message
 *
 * RETURNS: N/A
 */

void printExc(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4
    )
{
    if (excJobAdd(
            (VOIDFUNCPTR) printErr,
            fmt,
            (ARG) arg0,
            (ARG) arg1,
            (ARG) arg2,
            (ARG) arg3,
            (ARG) arg4
            ) != OK)
    {
        fprintf(
            stderr,
            fmt,
            (ARG) arg0,
            (ARG) arg1,
            (ARG) arg2,
            (ARG) arg3,
            (ARG) arg4
            );
    }
}

