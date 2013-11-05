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

/* logLib.c - Logging library */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <vmx/semLib.h>
#include <vmx/msgQLib.h>
#include <os/logLib.h>

/* Globals */
int logTaskId           = 0;
int logTaskPriority     = 0;
int logTaskOptions      = TASK_OPTIONS_UNBREAKABLE;
int logTaskStackSize    = 8000;
int mutextOptionsLogLib = SEM_Q_FIFO | SEM_DELETE_SAFE;

/* Locals */
LOCAL BOOL      logLibInstalled = FALSE;
LOCAL SEMAPHORE logFdSem;
LOCAL MSG_Q_ID  logMsgQId = NULL;
LOCAL int       logMsgsLost = 0;
LOCAL int       logFd[MAX_LOGFDS];
LOCAL int       numLogFds = 0;
LOCAL int       oldMsgsLost = 0;

LOCAL void logTask(
    void
    );

LOCAL void lprintf(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5
    );

/******************************************************************************
 * logLibInit - Inititalize logging library
 *
 * RETURNS: OK or ERROR
 */

STATUS logLibInit(
    int fd,
    int maxMsgs
    )
{
    STATUS status;

    /* Check if already installed */
    if (logLibInstalled == TRUE)
    {
        if (logTaskId == 0)
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
        /* Create message queue */
        logMsgQId = msgQCreate(maxMsgs, sizeof(LOG_MSG), MSG_Q_FIFO);
        if (logMsgQId == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Start log task */
            logTaskId = taskSpawn(
                            "tLogTask",
                            logTaskPriority,
                            logTaskOptions,
                            logTaskStackSize,
                            (FUNCPTR) logTask,
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
            if (logTaskId == 0)
            {
                status = ERROR;
            }
            else
            {
                /* Initialize semaphore */
                semMInit(&logFdSem, mutextOptionsLogLib);

                /* File descriptor */
                logFdAdd(fd);

                /* Mark as installed */
                logLibInstalled = TRUE;
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * logTask - Logging task
 *
 * RETURNS: OK or ERROR
 */

LOCAL void logTask(
    void
    )
{
    LOG_MSG logMsg;
    char *tName;
    int newMsgsLost;

    /* Initialize locals */
    newMsgsLost = oldMsgsLost;

    /* Endless loop */
    while (1)
    {
        /* If failed to get message */
        if (msgQReceive(logMsgQId, &logMsg, sizeof(logMsg), WAIT_FOREVER) !=
            sizeof(logMsg))
        {
            /* Error */
            lprintf(
                "logTask: error reading log messages.\n",
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );
        }
        else
        {
            if (logMsg.id == -1)
            {
                lprintf(
                    "interrupt: ",
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0
                    );
            }
            else
            {
                /* Get name from task */
                tName = taskName(logMsg.id);
                if (tName == NULL)
                {
                    lprintf(
                        "%#x (): task dead",
                        (ARG) logMsg.id,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0
                        );
                }
                else
                {
                    lprintf(
                        "%#x (%s): ",
                        (ARG) logMsg.id,
                        (ARG) tName,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0
                        );
                }
            }

            /* If message format null */
            if (logMsg.fmt == NULL)
            {
                lprintf(
                    "<null \"fmt\" parameter>\n",
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0
                    );
            }
            else
            {
                lprintf(
                    logMsg.fmt,
                    logMsg.arg[0],
                    logMsg.arg[1],
                    logMsg.arg[2],
                    logMsg.arg[3],
                    logMsg.arg[4],
                    logMsg.arg[5]
                    );
            }
        }

        /* If lost messages changed */
        if (newMsgsLost != oldMsgsLost)
        {
            lprintf(
                "logTask: %d log messages lost.\n",
                (ARG) newMsgsLost - oldMsgsLost,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );

            oldMsgsLost = newMsgsLost;
        }
    }
}

/******************************************************************************
 * logMsg - Send job to log task
 *
 * RETURNS: Size of message or EOF
 */

int logMsg(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5
    )
{
    int ret;
    int timeout;
    LOG_MSG logMsg;

    /* If called from interupt */
    if (INT_CONTEXT() == TRUE)
    {
        logMsg.id = -1;
        timeout   = WAIT_NONE;
    }
    else
    {
        logMsg.id = taskIdSelf();
        timeout   = WAIT_FOREVER;
    }

    /* Setup struct */
    logMsg.fmt    = fmt;
    logMsg.arg[0] = arg0;
    logMsg.arg[1] = arg1;
    logMsg.arg[2] = arg2;
    logMsg.arg[3] = arg3;
    logMsg.arg[4] = arg4;
    logMsg.arg[5] = arg5;

    /* If message got error when sending */
    if (msgQSend(
            logMsgQId,
            &logMsg,
            sizeof(logMsg),
            timeout,
            MSG_PRI_NORMAL
            ) != OK)
    {
        logMsgsLost++;
        ret = EOF;
    }
    else
    {
        ret = sizeof(logMsg);
    }

    return ret;
}

/******************************************************************************
 * logFdAdd - Add logging file descriptor
 *
 * RETURNS: OK or ERROR
 */

STATUS logFdAdd(
    int fd
    )
{
    STATUS status;

    semTake(&logFdSem, WAIT_FOREVER);

    /* If maximum file descriptor reached */
    if ((numLogFds + 1) > MAX_LOGFDS)
    {
        status = ERROR;
    }
    else
    {
        /* Add file descriptor */
        logFd[numLogFds++] = fd;
        status = OK;
    }

    semGive(&logFdSem);

    return status;
}

/******************************************************************************
 * lprintf - Log printf
 *
 * RETURNS: N/A
 */

LOCAL void lprintf(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5
    )
{
    int i;

    semTake(&logFdSem, WAIT_FOREVER);

    for (i = 0; i < numLogFds; i++)
    {
        fdprintf(logFd[i], fmt, arg0, arg1, arg2, arg3, arg4, arg5);
    }

    semGive(&logFdSem);
}

