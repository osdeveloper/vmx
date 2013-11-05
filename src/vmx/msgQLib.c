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

/* msgQLib.c - Message queue library */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <util/qFifoLib.h>
#include <util/qPrioLib.h>
#include <util/qMsgLib.h>
#include <vmx/taskLib.h>
#include <vmx/memPartLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/msgQLib.h>

/* Imports */
STATUS qMsgPut(
    MSG_Q_ID msgQId,
    Q_MSG_HEAD *pQHead,
    Q_MSG_NODE *pNode,
    int key
    );

Q_MSG_NODE* qMsgGet(
    MSG_Q_ID msgQId,
    Q_MSG_HEAD *pQHead,
    unsigned timeout
    );

STATUS qMsgDestroy(
    Q_MSG_HEAD *pQHead
    );

/* Locals */
LOCAL OBJ_CLASS msgQClass;
LOCAL BOOL msgQLibInstalled = FALSE;

/* Globals */
CLASS_ID msgQClassId = &msgQClass;

/******************************************************************************
 * msgQLibInit - Initialize message quque library
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQLibInit(
    void
    )
{
    STATUS status;
    if (msgQLibInstalled)
    {
        status = OK;
    }
    else
    {
        if (classInit(
                msgQClassId,
                sizeof(MSG_Q),
                OFFSET(MSG_Q, objCore),
                memSysPartId,
                (FUNCPTR) msgQCreate,
                (FUNCPTR) msgQInit,
                (FUNCPTR) msgQDestroy) != OK)
        {
            status = ERROR;
        }
        else
        {
            msgQLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * msgQCreate - Create a message queue
 *
 * RETURNS: MSG_Q_ID or NULL
 */

MSG_Q_ID msgQCreate(
    int maxMsg,
    int maxMsgLength,
    int options
    )
{
    MSG_Q_ID msgQId;
    void *pPool;
    unsigned size;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        msgQId = NULL;
    }
    else
    {
        if (msgQLibInstalled != TRUE)
        {
            errnoSet(S_msgQLib_NOT_INSTALLED);
            msgQId = NULL;
        }
        else
        {
            /* Get size */
            size = (unsigned) maxMsg * MSG_NODE_SIZE(maxMsgLength);

            /* Allocate mem */
            msgQId = (MSG_Q_ID) objAllocPad(msgQClassId, size, &pPool);
            if (msgQId != NULL)
            {
                /* Initialize */
                if (msgQInit(
                        msgQId,
                        maxMsg,
                        maxMsgLength,
                        options,
                        pPool
                        ) != OK)
                {
                    objFree(msgQClassId, msgQId);
                    msgQId = NULL;
                }
            }
        }
    }

    return msgQId;
}

/******************************************************************************
 * msgQInit - Initialize a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQInit(
    MSG_Q_ID msgQId,
    int maxMsg,
    int maxMsgLength,
    int options,
    void *pMsgPool
    )
{
    STATUS status;
    int nodeSize;
    int i;
    Q_CLASS_ID msgQType;

    if (msgQLibInstalled != TRUE)
    {
        errnoSet(S_msgQLib_NOT_INSTALLED);
        status = ERROR;
    }
    else
    {
        /* Get size */
        nodeSize = MSG_NODE_SIZE(maxMsgLength);

        /* Clear */
        memset(msgQId, 0, sizeof(MSG_Q));

        /* Initialize queues */
        switch(options & MSG_Q_TYPE_MASK)
        {
            case MSG_Q_FIFO:
                msgQType = qFifoClassId;
                status = OK;
                break;

            case MSG_Q_PRIORITY:
                msgQType = qPrioClassId;
                status = OK;
                break;

            default:
                errnoSet(S_msgQLib_INVALID_Q_TYPE);
                status = ERROR;
                break;
        }

        if (status == OK)
        {
            if (qInit(
                    (Q_HEAD *) &msgQId->msgQ,
                    qMsgClassId,
                    msgQType,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0
                    ) != OK)
            {
                status = ERROR;
            }
            else
            {
                if (qInit(
                        (Q_HEAD *) &msgQId->freeQ,
                        qMsgClassId,
                        msgQType,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0
                        ) != OK)
                {
                    status = ERROR;
                }
                else
                {
                    /* Insert nodes on the free queue */
                    for (i = 0; i < maxMsg; i++)
                    {
                        qMsgPut(
                            msgQId,
                            &msgQId->freeQ,
                            (Q_MSG_NODE *) pMsgPool,
                            Q_MSG_PRI_ANY
                            );
                        pMsgPool = (void *) (((char *) pMsgPool) + nodeSize);
                    }

                    /* Initialize structure */
                    msgQId->options = options;
                    msgQId->maxMsg = maxMsg;
                    msgQId->maxMsgLength = maxMsgLength;

                    objCoreInit(&msgQId->objCore, msgQClassId);
                    status = OK;
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * msgQTerminate - Terminate a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQTerminate(
    MSG_Q_ID msgQId
    )
{
    return msgQDestroy(msgQId, FALSE);
}

/******************************************************************************
 * msgQDelete - Delete a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQDelete(
    MSG_Q_ID msgQId
    )
{
    return msgQDestroy(msgQId, FALSE);
}

/******************************************************************************
 * msgQDestroy - Destroy a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQDestroy(
    MSG_Q_ID msgQId,
    BOOL deallocate
    )
{
    STATUS status;
    Q_MSG_NODE *pNode;
    int errnoCopy;
    unsigned timeout;
    int nMsg;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        status = ERROR;
    }
    else
    {
        /* Lock */
        taskSafe();
        taskLock();

        if (OBJ_VERIFY(msgQId, msgQClassId) != OK)
        {
            taskUnlock();
            taskUnsafe();
            status = ERROR;
        }
        else
        {
            /* Invalidate */
            objCoreTerminate(&msgQId->objCore);

            taskUnlock();
            timeout = WAIT_NONE;
            nMsg = 0;
            errnoCopy = errnoGet();

            /* Pick out all nodes */
            while (nMsg < msgQId->maxMsg)
            {
                while (((pNode = qMsgGet(
                                     msgQId,
                                     &msgQId->freeQ,
                                     timeout
                                     )) != NULL) &&
                       (pNode != (Q_MSG_NODE *) NONE))
                {
                    nMsg++;
                }

                while (((pNode = qMsgGet(
                                     msgQId,
                                     &msgQId->msgQ,
                                     timeout
                                     )) != NULL) &&
                       (pNode != (Q_MSG_NODE *) NONE))
                {
                    nMsg++;
                }

                /* Wait a little */
                timeout = 1;
            }

            errnoSet(errnoCopy);

            kernelState = TRUE;

            qMsgTerminate(&msgQId->msgQ);
            qMsgTerminate(&msgQId->freeQ);

            vmxExit();

            if (deallocate == TRUE)
            {
                objFree(msgQClassId, msgQId);
            }

            taskUnsafe();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * msgQSend - Send a message on message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQSend(
    MSG_Q_ID msgQId,
    void *buffer,
    unsigned nBytes,
    unsigned timeout,
    unsigned priority
    )
{
    STATUS    status  = OK;
    MSG_NODE *pMsg    = (MSG_NODE *) NONE;

    /* Lock */
    if (INT_CONTEXT() == FALSE)
    {
        taskLock();
    }
    else
    {
        if (timeout != WAIT_NONE)
        {
            errnoSet(S_msgQLib_INVALID_TIMEOUT);
            status = ERROR;
        }
    }

    /* Wait for free slot on message queue */
    while ((status != ERROR) && (pMsg == (MSG_NODE *) NONE))
    {
        if (OBJ_VERIFY(msgQId, msgQClassId) != OK)
        {
            if (INT_CONTEXT() == FALSE)
            {
                taskUnlock();
            }
            status = ERROR;
            break;
        }

        /* Check if size is within range */
        if (nBytes > msgQId->maxMsgLength)
        {
            errnoSet(S_msgQLib_INVALID_MSG_LENGTH);
            if (INT_CONTEXT() == FALSE)
            {
                taskUnlock();
            }
            status = ERROR;
            break;
        }

        /* Get next node from free queue */
        pMsg = (MSG_NODE *) qMsgGet(msgQId, &msgQId->freeQ, timeout);
        if (pMsg == NULL)
        {
            if (errnoGet() == S_objLib_TIMEOUT)
            {
                if (OBJ_VERIFY(msgQId, msgQClassId) == OK)
                {
                    msgQId->sendTimeouts++;
                }
                else
                {
                    errnoSet(S_objLib_DELETED);
                }
            }

            if (INT_CONTEXT() == FALSE)
            {
                taskUnlock();
            }
            status = ERROR;
            break;
        }
    }

    if (status != ERROR)
    {
        /* Put node on message queue */
        pMsg->msgLength = nBytes;
        memcpy(MSG_NODE_DATA(pMsg), buffer, nBytes);
        if (qMsgPut(msgQId, &msgQId->msgQ, &pMsg->node, priority) != OK)
        {
            status = ERROR;
        }

        if (INT_CONTEXT() == FALSE)
        {
            taskUnlock();
        }
    }

    return status;
}

/******************************************************************************
 * msgQReceive - Receive a message from message queue
 *
 * RETURNS: number of bytes read or ERROR
 */

int msgQReceive(
    MSG_Q_ID msgQId,
    void *buffer,
    unsigned maxBytes,
    unsigned timeout
    )
{
    int       result = OK;
    MSG_NODE *pMsg   = (MSG_NODE *) NONE;

    if (INT_RESTRICT() != OK)
    {
        errnoSet(S_intLib_NOT_ISR_CALLABLE);
        result = ERROR;
    }

    /* Lock */
    taskLock();

    /* Message receive loop */
    while ((result != ERROR) && (pMsg == (MSG_NODE *) NONE))
    {
        if (OBJ_VERIFY(msgQId, msgQClassId) != OK)
        {
            taskUnlock();
            result = ERROR;
            break;
        }

        /* Get next message from message queue */
        pMsg = (MSG_NODE *) qMsgGet(msgQId, &msgQId->msgQ, timeout);
        if (pMsg == NULL)
        {
            if (errnoGet() == S_objLib_TIMEOUT)
            {
                if (OBJ_VERIFY(msgQId, msgQClassId) == OK)
                {
                    msgQId->sendTimeouts++;
                }
                else
                {
                    errnoSet(S_objLib_DELETED);
                }
            }

            taskUnlock();
            result = ERROR;
            break;
        }
    }

    if (result != ERROR)
    {
        /* Store data */
        result = min(pMsg->msgLength, maxBytes);
        memcpy(buffer, MSG_NODE_DATA(pMsg), result);

        /* Put node back on free queue */
        qMsgPut(msgQId, &msgQId->freeQ, &pMsg->node, 1);

        taskUnlock();
    }

    return result;
}

/******************************************************************************
 * msgQNumMsgs - Get number of messages on message queue
 *
 * RETURNS: Number of messages on queue or ERROR
 */

int msgQNumMsgs(
    MSG_Q_ID msgQId
    )
{
    int count;

    /* Verify object class */
    if (OBJ_VERIFY(msgQId, msgQClassId) != OK)
    {
        count = ERROR;
    }
    else
    {
        count = msgQId->msgQ.count;
    }

    return count;
}

