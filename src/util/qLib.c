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

/* qLib.c - Queue library */

#include <stdlib.h>
#include <stdarg.h>
#include <util/qLib.h>

/******************************************************************************
 * qCreate - Create a new queue
 *
 * RETURNS: Pointer to head or NULL
 */

Q_HEAD* qCreate(
    Q_CLASS *pQClass,
    ...
    )
{
    va_list pArg;
    Q_HEAD *pQHead;
    int i;
    ARG arg[10];

    /* Verify class */
    if (Q_CLASS_VERIFY(pQClass) != OK) {
        pQHead = NULL;
    }
    else
    {
        /* Read args into array */
        va_start(pArg, pQClass);
        for (i = 0; i < 10; i++)
        {
            arg[i] = va_arg(pArg, ARG);
        }
        va_end(pArg);

        /* Allocate buffer */
        pQHead = malloc(sizeof(Q_HEAD));
        if (pQHead != NULL)
        {
            /* Call initializer function below */
            if (qInit(
                    pQHead,
                    pQClass,
                    arg[0],
                    arg[1],
                    arg[2],
                    arg[3],
                    arg[4],
                    arg[5],
                    arg[6],
                    arg[7],
                    arg[8],
                    arg[9]
                    ) != OK)
            {
                free(pQHead);
                pQHead = NULL;
            }
        }
    }

    return pQHead;
}

/******************************************************************************
 * qInit - Initialize queue structure
 *
 * RETURNS: OK or ERROR
 */

STATUS qInit(
    Q_HEAD *pQHead,
    Q_CLASS *pQClass,
    ...
    )
{
    STATUS status;
    va_list pArg;
    int i;
    ARG arg[10];

    /* Verify class */
    if (Q_CLASS_VERIFY(pQClass) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Read args into array */
        va_start(pArg, pQClass);
        for (i = 0; i < 10; i++)
        {
            arg[i] = va_arg(pArg, ARG);
        }
        va_end(pArg);

        /* Store class */
        pQHead->pQClass = pQClass;

        /* Call initializer */
        if (((*(pQClass->initMethod))(
                    pQHead,
                    arg[0],
                    arg[1],
                    arg[2],
                    arg[3],
                    arg[4],
                    arg[5],
                    arg[6],
                    arg[7],
                    arg[8],
                    arg[9])
                    ) != OK)
        {
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
 * qDestroy - Deallocate
 *
 * RETURNS: OK or ERROR
 */

STATUS qDestroy(
    Q_HEAD *pQHead
    )
{
    STATUS status;

    /* Finnish off headnode */
    if (qTerminate(pQHead) != OK)
    {
        status = ERROR;
    }
    else
    {
        pQHead->pQClass = NULL;
        free(pQHead);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * qTerminate - Finnish of head node
 *
 * RETURNS: OK or ERROR
 */

STATUS qTerminate(
    Q_HEAD *pQHead
    )
{
    STATUS status;

    if (((*(pQHead->pQClass->terminateMethod))(pQHead)) != OK)
    {
        status = ERROR;
    }
    else
    {
        pQHead->pQClass = NULL;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * qFirst - Get first node in queue
 *
 * RETURNS: Pointer to first node in queue
 */

Q_NODE* qFirst(
    Q_HEAD *pQHead
    )
{
    return Q_FIRST(pQHead);
}

/******************************************************************************
 * qPut - Put node into queue
 *
 * RETURNS: N/A
 */

void qPut(
    Q_HEAD *pQHead,
    Q_NODE *pQNode,
    unsigned long key
    )
{
    Q_PUT(pQHead, pQNode, key);
}

/******************************************************************************
 * qGet - Get and remove first node on queue
 *
 * RETURNS: First node on queue
 */

Q_NODE* qGet(
    Q_HEAD *pQHead
    )
{
    return Q_GET(pQHead);
}

/******************************************************************************
 * qRemove - Remove node from queue
 *
 * RETURNS: OK or ERROR
 */

STATUS qRemove(
    Q_HEAD *pQHead,
    Q_NODE *pQNode
    )
{
    return Q_REMOVE(pQHead, pQNode);
}

/******************************************************************************
 * qMove - Move a node in the key based on given key
 *
 * RETURNS: N/A
 */

void qMove(
    Q_HEAD *pQHead,
    Q_NODE *pQNode,
    unsigned long newKey
    )
{
    Q_MOVE(pQHead, pQNode, newKey);
}

/******************************************************************************
 * qAdvance - Advance a frame (time slice)
 *
 * RETURNS: N/A
 */

void qAdvance(
    Q_HEAD *pQHead
    )
{
    Q_ADVANCE(pQHead);
}

/******************************************************************************
 * qExpired - Get time expire node
 *
 * RETURNS: Pointer to node on head or NULL
 */

Q_NODE* qExpired(
    Q_HEAD *pQHead
    )
{
    return Q_EXPIRED(pQHead);
}

/******************************************************************************
 * qKey - Get key of node
 *
 * RETURNS: Key for node
 */

unsigned long qKey(
    Q_HEAD *pQHead,
    Q_NODE *pQNode,
    int keyType
    )
{
    return Q_KEY(pQHead, pQNode, keyType);
}

/******************************************************************************
 * qOffset - Calibrate each node in queue with some delta
 *
 * RETURNS: N/A
 */

void qOffset(
    Q_HEAD *pQHead,
    unsigned long keyDelta
    )
{
    Q_OFFSET(pQHead, keyDelta);
}

/******************************************************************************
 * qInfo - Gather info about queue
 *
 * RETURNS: Number of node pointers in array
 */

int qInfo(
    Q_HEAD *pQHead,
    Q_NODE nodeArray[],
    int maxNodes
    )
{
    return Q_INFO(pQHead, nodeArray, maxNodes);
}

/******************************************************************************
 * qEach - Call function for each node in queue
 *
 * RETURNS: NULL of whore queue processed or pointer to Q_NODE when ended
 */

Q_NODE* qEach(
    Q_HEAD *pQHead,
    FUNCPTR func,
    int arg
    )
{
    return Q_EACH(pQHead, func, arg);
}

