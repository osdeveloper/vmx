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

/* qPrioLib.c - Priority queue */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/private/kernLibP.h>
#include <util/dllLib.h>
#include <util/qLib.h>
#include <util/qPrioLib.h>

/* Forward declarations */
LOCAL Q_PRIO_HEAD* qPrioCreate(
    void
    );

LOCAL STATUS qPrioInit(
    Q_PRIO_HEAD *pQPriHead
    );

LOCAL STATUS qPrioDestroy(
    Q_PRIO_HEAD *pQPriHead
    );

LOCAL STATUS qPrioTerminate(
    Q_PRIO_HEAD *pQPriHead
    );

LOCAL void qPrioPut(
    Q_PRIO_HEAD *pQPriHead,
    Q_PRIO_NODE *pQPriNode,
    int key
    );

LOCAL Q_PRIO_NODE* qPrioGet(
    Q_PRIO_HEAD *pQPriHead
    );

LOCAL STATUS qPrioRemove(
    Q_PRIO_HEAD *pQPriHead,
    Q_PRIO_NODE *pQPriNode
    );

LOCAL void qPrioMove(
    Q_PRIO_HEAD *pQPriHead,
    Q_PRIO_NODE *pQPriNode,
    int newKey
    );

LOCAL Q_PRIO_NODE* qPrioExpired(
    Q_PRIO_HEAD *pQPriHead
    );

LOCAL void qPrioOffset(
    Q_PRIO_HEAD *pQPriHead,
    int delta
    );

LOCAL int qPrioKey(
    Q_PRIO_NODE *pQPriNode,
    int type
    );

LOCAL int qPrioInfo(
    Q_PRIO_HEAD *pQPrioHead,
    int nodeArray[],
    int maxNodes
    );

LOCAL Q_PRIO_NODE* qPrioEach(
    Q_PRIO_HEAD *pQPrioHead,
    FUNCPTR func,
    int arg
    );

LOCAL void qPrioNull(
    void
    );

/* Locals */
LOCAL Q_CLASS qPrioClass =
{
    (FUNCPTR) qPrioCreate,
    (FUNCPTR) qPrioInit,
    (FUNCPTR) qPrioDestroy,
    (FUNCPTR) qPrioTerminate,
    (FUNCPTR) qPrioPut,
    (FUNCPTR) qPrioGet,
    (FUNCPTR) qPrioRemove,
    (FUNCPTR) qPrioMove,
    (FUNCPTR) qPrioNull,
    (FUNCPTR) qPrioExpired,
    (FUNCPTR) qPrioKey,
    (FUNCPTR) qPrioOffset,
    (FUNCPTR) qPrioInfo,
    (FUNCPTR) qPrioEach,
    &qPrioClass
};

Q_CLASS_ID qPrioClassId = &qPrioClass;

/******************************************************************************
 * qPrioCreate - Create priority queue
 *
 * RETURNS: Priority queue head pointer
 */

LOCAL Q_PRIO_HEAD* qPrioCreate(
    void
    )
{
    Q_PRIO_HEAD *pQPriHead;

    /* Allocate memory for struct */
    pQPriHead = malloc( sizeof(Q_PRIO_HEAD) );
    if (pQPriHead != NULL) {
        /* Call initializer below */
        if (qPrioInit(pQPriHead) != OK)
        {
            pQPriHead = NULL;
        }
    }

    return pQPriHead;
}

/******************************************************************************
 * qPrioInit - Initialize a priority queue
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS qPrioInit(
    Q_PRIO_HEAD *pQPriHead
    )
{
    return dllInit(&pQPriHead->qPrio.head);
}

/******************************************************************************
 * qPrioDestroy - Deallocate a priority queue
 *
 * RETURNS: OK
 */

LOCAL STATUS qPrioDestroy(
    Q_PRIO_HEAD *pQPriHead
    )
{
    /* Free memory */
    free(pQPriHead);

    return OK;
}

/******************************************************************************
 * qPrioTerminate - Invalidate a priority queue
 *
 * RETURNS: OK
 */

LOCAL STATUS qPrioTerminate(
    Q_PRIO_HEAD *pQPriHead
    )
{
    return OK;
}

/******************************************************************************
 * qPrioPut - Put a node on the queue
 *
 * RETURNS: OK or ERROR
 */

LOCAL void qPrioPut(
    Q_PRIO_HEAD *pQPriHead,
    Q_PRIO_NODE *pQPriNode,
    int key
    )
{
    Q_PRIO_NODE *pQNode = (Q_PRIO_NODE *) DLL_HEAD( (DL_LIST *) pQPriHead);

    pQPriNode->qPrio.key = key;

    while (pQNode != NULL)
    {
        if (key < pQNode->qPrio.key)
        {
            dllInsert(
                &pQPriHead->qPrio.head,
                DLL_PREV(&pQNode->qPrio.node),
                &pQPriNode->qPrio.node
            );
            return;
        }

        pQNode = (Q_PRIO_NODE *) DLL_NEXT(&pQNode->qPrio.node);
    }

    dllInsert(
        &pQPriHead->qPrio.head,
        (DL_NODE *) DLL_TAIL(&pQPriHead->qPrio.head),
        &pQPriNode->qPrio.node
        );
}

/******************************************************************************
 * qPrioGet - Get and remove a node from queue
 *
 * RETURNS: Pointer to node
 */

LOCAL Q_PRIO_NODE* qPrioGet(
    Q_PRIO_HEAD *pQPriHead
    )
{
    Q_PRIO_NODE *pQNode;

    if (DLL_EMPTY(&pQPriHead->qPrio.head))
    {
        pQNode = NULL;
    }
    else
    {
        pQNode = (Q_PRIO_NODE *) dllGet(&pQPriHead->qPrio.head);
    }

    return pQNode;
}

/******************************************************************************
 * qPrioRemove - Remove node
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS qPrioRemove(
    Q_PRIO_HEAD *pQPriHead,
    Q_PRIO_NODE *pQPriNode
    )
{
    return dllRemove(&pQPriHead->qPrio.head, &pQPriNode->qPrio.node);
}

/******************************************************************************
 * qPrioMove - Move node to new place based on key
 *
 * RETURNS: N/A
 */

LOCAL void qPrioMove(
    Q_PRIO_HEAD *pQPriHead,
    Q_PRIO_NODE *pQPriNode,
    int newKey
    )
{
    Q_PRIO_NODE *pPrev = (Q_PRIO_NODE *) DLL_PREV(&pQPriNode->qPrio.node);
    Q_PRIO_NODE *pNext = (Q_PRIO_NODE *) DLL_NEXT(&pQPriNode->qPrio.node);

    if ( ((pPrev == NULL) || (newKey >= pPrev->qPrio.key)) &&
         ((pNext == NULL) || (newKey <= pNext->qPrio.key)) )
    {
        pQPriNode->qPrio.key = newKey;
    }
    else
    {
        qPrioRemove(pQPriHead, pQPriNode);
        qPrioPut(pQPriHead, pQPriNode, newKey);
    }
}

/******************************************************************************
 * qPrioExpired - Return timeout node
 *
 * RETURNS: Pointer to node
 */

LOCAL Q_PRIO_NODE* qPrioExpired(
    Q_PRIO_HEAD *pQPriHead
    )
{
    Q_PRIO_NODE *pQPriNode = (Q_PRIO_NODE *) DLL_HEAD((DL_LIST *) pQPriHead);
    Q_PRIO_NODE *pQNode;

    if ((pQPriNode != NULL) && (pQPriNode->qPrio.key <= kernTicks))
    {
        pQNode = (Q_PRIO_NODE *) dllGet(&pQPriHead->qPrio.head);
    }
    else
    {
        pQNode = NULL;
    }

    return pQNode;
}

/******************************************************************************
 * qPrioOffset - Calibrate every key with an offset delta
 *
 * RETURNS: N/A
 */

LOCAL void qPrioOffset(
    Q_PRIO_HEAD *pQPriHead,
    int delta
    )
{
    Q_PRIO_NODE *pQPriNode;

    for (pQPriNode = (Q_PRIO_NODE *) DLL_HEAD(&pQPriHead->qPrio.head);
         pQPriNode != NULL;
         pQPriNode = (Q_PRIO_NODE *) DLL_NEXT(&pQPriNode->qPrio.node))
    {
        pQPriNode->qPrio.key += delta;
    }
}

/******************************************************************************
 * qPrioKey - Get key
 *
 * RETURNS: key
 */

LOCAL int qPrioKey(
    Q_PRIO_NODE *pQPriNode,
    int type
    )
{
    int key;

    if (type == 0)
    {
        key = pQPriNode->qPrio.key;
    }
    else
    {
        key = pQPriNode->qPrio.key - kernTicks;
    }

    return key;
}

/******************************************************************************
 * qPrioInfo - Get info about queue
 *
 * RETURNS: Number of infos read
 */

LOCAL int qPrioInfo(
    Q_PRIO_HEAD *pQPrioHead,
    int nodeArray[],
    int maxNodes
    )
{
    DL_NODE *pNode;
    int *pElement;
    int count;

    /* If null array just return number of nodes */
    if (nodeArray == NULL)
    {
        count = dllCount((DL_LIST *) pQPrioHead);
    }
    else
    {
        /* Get first node */
        pNode = (DL_NODE *) DLL_HEAD( (DL_LIST *) pQPrioHead);

        /* Set element pointer */
        pElement = nodeArray;

        /* While node in list or maxNodex GE 0*/
        while ((pNode != NULL) && (--maxNodes >= 0))
        {
            *(pElement++) = (int) pNode;
            pNode = DLL_NEXT(pNode);

        }

        count = pElement - nodeArray;
    }

    return count;
}

/******************************************************************************
 * qPrioEach - Run function for each node in queue
 *
 * RETURNS: NULL or node where it ended
 */

LOCAL Q_PRIO_NODE* qPrioEach(
    Q_PRIO_HEAD *pQPrioHead,
    FUNCPTR func,
    int arg
    )
{
    DL_NODE *pNode;

    /* Get first node */
    pNode = (DL_NODE *) DLL_HEAD( (DL_LIST *) pQPrioHead);

    /* While node in list or nonzero return from function */
    while ((pNode != NULL) && ((*func)(pNode, arg)))
    {
        pNode = DLL_NEXT(pNode);
    }

    return (Q_PRIO_NODE *) pNode;
}

/******************************************************************************
 * qPrioNull - Null method
 *
 * RETURNS: N/A
 */

LOCAL void qPrioNull(
    void
    )
{
    return;
}

