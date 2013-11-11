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

/* pipeDrv.c - Pipe device driver */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/listLib.h>
#include <vmx/taskLib.h>
#include <vmx/semLib.h>
#include <vmx/msgQLib.h>
#include <vmx/msgQShow.h>
#include <os/errnoLib.h>
#include <os/ioLib.h>
#include <os/pipeDrv.h>

/* Defines */

/* Globals */
int pipeDrvMsgQOptions = MSG_Q_FIFO;

/* Locals */
LOCAL int pipeDrvNum = ERROR;

LOCAL int pipeCreate(
    PIPE_DEV *pPipeDev,
    const char *name,
    mode_t mode,
    const char *symlink
    );

LOCAL int pipeDelete(
    PIPE_DEV *pPipeDev,
    const char *name,
    mode_t mode
    );

LOCAL int pipeOpen(
    PIPE_DEV *pPipeDev,
    const char *name, int flags,
    mode_t mode
    );

LOCAL int pipeClose(
    PIPE_DEV *pPipeDev
    );

LOCAL int pipeRead(
    PIPE_DEV *pPipeDev,
    char *buf,
    unsigned int maxBytes
    );

LOCAL int pipeWrite(
    PIPE_DEV *pPipeDev,
    char *buf,
    int maxBytes
    );

LOCAL pipeIoctl(
    PIPE_DEV *pPipeDev,
    int func, ARG arg
    );

/* Functions */

/******************************************************************************
 * pipeDrvInit- Install pipe device
 *
 * RETURNS: OK or ERROR
 */

STATUS pipeDrvInit(
    void
    )
{
    STATUS status;

    if (pipeDrvNum == ERROR)
    {
        pipeDrvNum = iosDrvInstall(
                         (FUNCPTR) pipeCreate,
                         (FUNCPTR) pipeDelete,
                         (FUNCPTR) pipeOpen,
                         (FUNCPTR) pipeClose,
                         (FUNCPTR) pipeRead,
                         (FUNCPTR) pipeWrite,
                         (FUNCPTR) pipeIoctl
                         );
        if (pipeDrvNum == ERROR)
        {
            status = ERROR;
        }
        else
        {
            status =  OK;
        }
    }
    else
    {
        status = OK;
    }

    return status;
}

/*******************************************************************************
 * pipeDevCreate - Create pipe device
 *
 * RETURNS: OK or ERROR
 */

STATUS pipeDevCreate(
    char *name,
    int maxMsg,
    int maxBytes
    )
{
    STATUS status;
    PIPE_DEV *pPipeDev;

    /* Not callable from interrupts */
    if (INT_RESTRICT() != OK)
    {
        status = ERROR;
    }
    else
    {
        /* If invalid driver number */
        if (pipeDrvNum == ERROR)
        {
            status = ERROR;
        }
        else
        {
            /* Allocate memory for pipe */
            pPipeDev = (PIPE_DEV *) malloc(sizeof(PIPE_DEV) +
                maxMsg * MSG_NODE_SIZE(maxBytes));
            if (pPipeDev == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Initailize data structure */
                pPipeDev->numOpens = 0;

                /* Initalize message queue */
                if (msgQInit(
                        &pPipeDev->msgQ,
                        maxMsg,
                        maxBytes,
                        pipeDrvMsgQOptions,
                        (((char *) pPipeDev) + sizeof(PIPE_DEV))
                        ) != OK)
                {
                    free(pPipeDev);
                    status = ERROR;
                }
                else
                {
                    /* Initalize select wakup list */
                    selWakeupListInit(&pPipeDev->selWakeupList);

                    /* Install driver */
                    if (iosDevAdd(
                            &pPipeDev->devHeader,
                            name,
                            pipeDrvNum
                            ) != OK)
                    {
                        msgQTerminate(&pPipeDev->msgQ);
                        free(pPipeDev);
                        status = ERROR;
                    }
                    else
                    {
                        status = OK;
                    }
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * pipeDevDelete - Delete pipe device
 *
 * RETURNS: OK or ERROR
 */

STATUS pipeDevDelete(
    char *name,
    BOOL force
    )
{
    STATUS status;

    PIPE_DEV *pPipeDev;
    char *pTail;
    SEL_WAKEUP_NODE *wNode;

    /* Not callable from interrupts */
    if (INT_RESTRICT() != OK)
    {
        status = ERROR;
    }
    else
    {
        /* If invalid driver number */
        if (pipeDrvNum == ERROR)
        {
            status = ERROR;
        }
        else
        {
            /* Find device */
            pPipeDev = (PIPE_DEV *) iosDevFind(name, &pTail);
            if (pPipeDev == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* If not forced delete */
                if (force == FALSE)
                {
                    if (pPipeDev->numOpens != 0)
                    {
                        errnoSet(EMFILE);
                        status = ERROR;
                    }
                    else
                    {
                        if (selWakeupListLen(&pPipeDev->selWakeupList) != 0)
                        {
                            errnoSet(EBUSY);
                            status = ERROR;
                        }
                        else
                        {
                            status = OK;
                        }
                    }
                }

                if (status != ERROR)
                {
                    /* Delete device */
                    iosDevDelete(&pPipeDev->devHeader);

                    /* If forced and a threads selects on this pipe */
                    if ((force == TRUE) &&
                        (selWakeupListLen(&pPipeDev->selWakeupList) != 0))
                    {
                        wNode = (SEL_WAKEUP_NODE *)
                            LIST_HEAD(&pPipeDev->selWakeupList);
                        do
                        {
                            selNodeDelete(&pPipeDev->selWakeupList, wNode);
                        } while ((wNode = (SEL_WAKEUP_NODE *)
                                      LIST_NEXT(wNode)) != NULL);

                        selWakeupListTerminate(&pPipeDev->selWakeupList);
                    }

                    /* Terminate message queue */
                    msgQTerminate(&pPipeDev->msgQ);

                    /* Free struct */
                    free(pPipeDev);
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * pipeCreate - Create not supported
 *
 * RETURNS: ERROR
 */

LOCAL int pipeCreate(
    PIPE_DEV *pPipeDev,
    const char *name,
    mode_t mode,
    const char *symlink)
{
    return ERROR;
}

/******************************************************************************
 * pipeDelete - Delete not supported
 *
 * RETURNS: ERROR
 */

LOCAL int pipeDelete(
    PIPE_DEV *pPipeDev,
    const char *name,
    mode_t mode
    )
{
    return ERROR;
}

/******************************************************************************
 * pipeOpen - Open a pipe
 *
 * RETURNS: Pointer to pipe device or ERROR
 */

LOCAL int pipeOpen(
    PIPE_DEV *pPipeDev,
    const char *name,
    int flags,
    mode_t mode
    )
{
    int ret;

    if ((name != NULL) && (strlen(name) > 0))
    {
        errnoSet(S_ioLib_NO_DEVICE_NAME_IN_PATH);
        ret = ERROR;
    }
    else
    {
        /* Increase num opens */
        ++pPipeDev->numOpens;
        ret = (int) pPipeDev;
    }

    return ret;
}

/*******************************************************************************
 * pipeClose - Close a pipe
 *
 * RETURNS: OK or ERROR
 */

LOCAL int pipeClose(
    PIPE_DEV *pPipeDev
    )
{
    int ret;

    /* Check pipe device */
    if (pPipeDev == NULL)
    {
        ret = ERROR;
    }
    else
    {
        /* Descrease num opens */
        if (pPipeDev->numOpens > 0)
        {
            --pPipeDev->numOpens;
        }
        ret = OK;
    }

    return ret;
}

/******************************************************************************
 * pipeRead - Read from a pipe
 *
 * RETURNS: Number of bytes read or ERROR
 */

LOCAL int pipeRead(
    PIPE_DEV *pPipeDev,
    char *buf,
    unsigned int maxBytes
    )
{
    int nBytes;

    /* Block for messages to arrive on pipe */
    nBytes = msgQReceive(&pPipeDev->msgQ, buf, maxBytes, WAIT_FOREVER);
    if (nBytes != ERROR)
    {
        selWakeupAll(&pPipeDev->selWakeupList, SELWRITE);
    }

    return nBytes;
}

/******************************************************************************
 * pipeWrite - Write to pipe
 *
 * RETURNS: Number of bytes written or ERROR
 */

LOCAL int pipeWrite(
    PIPE_DEV *pPipeDev,
    char *buf,
    int maxBytes
    )
{
    int ret;
    int timeout;

    if (INT_CONTEXT() == FALSE)
    {
        taskLock();
        timeout = WAIT_FOREVER;
    }
    else
    {
        timeout = WAIT_NONE;
    }

    /* Send message on message queue */
    if (msgQSend(
            &pPipeDev->msgQ,
            buf,
            (unsigned) maxBytes,
            timeout,
            MSG_PRI_NORMAL
            ) != OK)
    {
        if (INT_CONTEXT() == FALSE)
        {
            taskUnlock();
        }
        ret = ERROR;
    }
    else
    {
        /* If not called from interrput */
        if (INT_CONTEXT() == FALSE)
        {
            semTake(&pPipeDev->selWakeupList.mutex, WAIT_FOREVER);
            taskUnlock();
        }

        /* Wakeup select */
        selWakeupAll(&pPipeDev->selWakeupList, SELREAD);

        if (INT_CONTEXT() == FALSE)
        {
            semGive(&pPipeDev->selWakeupList.mutex);
        }

        ret = maxBytes;
    }

    return ret;
}

/******************************************************************************
 * pipeIoctl - Perform ioctl on pipe
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS pipeIoctl(
    PIPE_DEV *pPipeDev,
    int func,
    ARG arg
    )
{
    int ret;
    MSG_Q_INFO msgQInfo;
    SEL_WAKEUP_NODE *wNode;

    /* Select function */
    switch (func)
    {
        /* Number of messages on pipe */
        case FIONMSGS:
            *(int *) arg = msgQNumMsgs(&pPipeDev->msgQ);
            ret = OK;
            break;

        /* Get number of bytes in first message on pipe */
        case FIONREAD:
            memset(&msgQInfo, 0, sizeof(msgQInfo));
            msgQInfo.msgListMax = 1;
            msgQInfo.msgLengthList = (int *) arg;
            *(int *) arg = 0;
            msgQInfoGet(&pPipeDev->msgQ, &msgQInfo);
            ret = OK;
            break;

        /* Flush all messages */
        case FIOFLUSH:
            taskLock();
            while (msgQReceive(&pPipeDev->msgQ, NULL, 0, WAIT_NONE) != ERROR);
            taskUnlock();
            ret = OK;
            break;

        /* Perform select on pipe */
        case FIOSELECT:
            wNode = (SEL_WAKEUP_NODE *) arg;
            selNodeAdd(&pPipeDev->selWakeupList, wNode);

            switch(wNode->type)
            {
                case SELREAD:
                    if (msgQNumMsgs(&pPipeDev->msgQ) > 0)
                    {
                        selWakeup(wNode);
                    }
                    break;

                case SELWRITE:
                    if (pPipeDev->msgQ.maxMsg > msgQNumMsgs(&pPipeDev->msgQ))
                    {
                        selWakeup(wNode);
                    }
                    break;
            }
            ret = OK;
            break;

        /* Unselect pipe */
        case FIOUNSELECT:
            wNode = (SEL_WAKEUP_NODE *) arg;
            selNodeDelete(&pPipeDev->selWakeupList, wNode);
            ret = OK;
            break;

        default:
            errnoSet(S_ioLib_UNKNOWN_REQUEST);
            ret = ERROR;
            break;
    }

    return ret;
}

