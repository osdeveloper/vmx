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

/* qPriBmpLib.c - Priority bitmapped queue library */


/* includes */

#include <vmx.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/qPriBmpLib.h>
#include <vmx/ffsLib.h>

/* forward declarations */ 

LOCAL Q_PRI_BMP_HEAD* qPriBmpCreate(
    unsigned size
    );

LOCAL STATUS qPriBmpInit(
    Q_PRI_BMP_HEAD *pQHead,
    unsigned size,
    DL_LIST *pList,
    unsigned *pBmp
    );

LOCAL STATUS qPriBmpDestroy(
    Q_PRI_BMP_HEAD *pQHead
    );

LOCAL STATUS qPriBmpTerminate(
    Q_PRI_BMP_HEAD *pQHead
    );

LOCAL void qPriBmpPut(
    Q_PRI_BMP_HEAD *pQHead,
    Q_PRIO_NODE *pQNode,
    int key 
    );

LOCAL Q_PRIO_NODE* qPriBmpGet(
    Q_PRI_BMP_HEAD *pQHead
    );

LOCAL STATUS qPriBmpRemove(
    Q_PRI_BMP_HEAD *pQHead,
    Q_PRIO_NODE *pQNode
    );

LOCAL void qPriBmpMove(
    Q_PRI_BMP_HEAD *pQHead,
    Q_PRIO_NODE *pQNode,
    int key
    );

LOCAL void qPriBmpOffset(
    Q_PRI_BMP_HEAD *pQHead,
    int delta
    );

LOCAL int qPriBmpKey(
    Q_PRI_BMP_HEAD *pQHead,
    int type
    );

LOCAL void qPriBmpNull(
    void
    );

/* locals */

LOCAL Q_CLASS qPriBmpClass =
{
    (FUNCPTR) qPriBmpCreate,      /* Create the queue */
    (FUNCPTR) qPriBmpInit,        /* Initialize the queue */
    (FUNCPTR) qPriBmpDestroy,     /* Destroy the queue */
    (FUNCPTR) qPriBmpTerminate,   /* Terminate the queue */
    (FUNCPTR) qPriBmpPut,         /* Add an item to the queue */
    (FUNCPTR) qPriBmpGet,         /* Get 1st item in the queue */
    (FUNCPTR) qPriBmpRemove,      /* Remove item from the queue*/
    (FUNCPTR) qPriBmpMove,        /* Move node to a new place */
    (FUNCPTR) qPriBmpNull,        /* Null method */
    (FUNCPTR) qPriBmpNull,        /* Get timeout node */
    (FUNCPTR) qPriBmpNull,        /* Get the node's key */
    (FUNCPTR) qPriBmpNull,        /* Calibrate each key with an offset */
    (FUNCPTR) qPriBmpNull,        /* Get info about queue */
    (FUNCPTR) qPriBmpNull,        /* Execute function for each node in queue */
    &qPriBmpClass
};

/* globals */

Q_CLASS_ID qPriBmpClassId = &qPriBmpClass;

/***************************************************************************
 * qHeadFirstFind - determine which priority qHead has the first node in list
 *
 * This routine determines the which priority queue head in the bitmapped
 * queue has the first node.
 *
 * RETURNS: ERROR if empty, otherwise qHead index
 */

LOCAL int qHeadFirstFind(
    Q_PRI_BMP_HEAD *pQHead
    )
{
    int i, bit;
    int result = ERROR;

    for (i = 0; i < 8; i++)
    {
        bit = ffsLsb(pQHead->qPriBmp.bmp[i]);

        if (bit != 0)
        {
            result = (i << 5) + bit - 1;
            break;
        }
    }

    return result;
}

/***************************************************************************
 * qPriBmpCreate - create a priority bitmapped queue
 *
 * This routine creates a priority bitmapped queue.  The size of the
 * bitmap should be a multiple of 32.  If it is not, it will be rounded up
 * to the nearest multiple thereof.
 *
 * RETURNS: ptr to the newly created Q_PRI_BMP_HEAD; NULL on error
 */

LOCAL Q_PRI_BMP_HEAD* qPriBmpCreate(
    unsigned  size
    )
{
    Q_PRI_BMP_HEAD *pQHead;
    DL_LIST *pList;
    unsigned *pBmp;

    size = (size + 31) & (~0x1f);    /* Round up to multiple of 32 */
    if (size == 0)
    {
        pQHead = NULL;
    }
    else
    {
        pQHead = (Q_PRI_BMP_HEAD *) malloc(sizeof (Q_PRI_BMP_HEAD) +
                                           size * sizeof (DL_LIST) +
                                           (size / 32) * sizeof (unsigned));
        if (pQHead != NULL)
        {
            pList = (DL_LIST *) ((char *) pQHead + sizeof (Q_PRI_BMP_HEAD));
            pBmp  = (unsigned *) ((char *) pList + (size * sizeof (DL_LIST)));

            if (qPriBmpInit(pQHead, size, pList, pBmp) != OK)
            {
                pQHead = NULL;
            }
        }
    }

    return pQHead;
}

/***************************************************************************
 * qPriBmpInit - initialize a priority bitmapped queue
 *
 * This routine initializes the priority bitmapped queue.
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS qPriBmpInit(
    Q_PRI_BMP_HEAD *pQHead,
    unsigned size,             /* must be a multiple of 32 */
    DL_LIST *pList,            /* ptr to mem where list will be stored */
    unsigned *pBmp             /* ptr to mem where bitmap will be stored */
    )
{
    STATUS status;
    int i, j;

    if (((size & 0x1f) != 0) || (size == 0))
    {
        status = ERROR;
    }
    else
    {
        pQHead->qPriBmp.head = pList;
        pQHead->qPriBmp.bmp  = pBmp;

        for (i = 0; i < size; i++)
        {
            dllInit(&pQHead->qPriBmp.head[i]);
        }

        j = size / 32;
        for (i = 0; i < j; i++)
        {
            pQHead->qPriBmp.bmp[i] = 0;
        }

        pQHead->qPriBmp.pQClass = &qPriBmpClass;
        pQHead->qPriBmp.pFirst = NULL;
        status = OK;
    }

    return status;
}

/***************************************************************************
 * qPriBmpDestroy - destroy the priority bitmapped queue
 *
 * RETURNS: OK
 */

LOCAL STATUS qPriBmpDestroy(
    Q_PRI_BMP_HEAD *pQHead
    )
{
    free(pQHead);

    return OK;
}

/***************************************************************************
 * qPriBmpTerminate - invalidate a priority bitmapped queue
 *
 * RETURNS: OK
 */

LOCAL STATUS qPriBmpTerminate(
    Q_PRI_BMP_HEAD *pQHead
    )
{
    return OK;
}

/***************************************************************************
 * qPriBmpPut - put a node on the priority bitmapped queue
 *
 * RETURNS: N/A
 */

LOCAL void qPriBmpPut(
    Q_PRI_BMP_HEAD *pQHead,       /* pointer to queue head */
    Q_PRIO_NODE *pQNode,          /* pointer to node to add */
    int key                       /* priority */
    )
{
    int first;

    /* Indicate that this queue is not empty */
    pQHead->qPriBmp.bmp[key >> 5] |= (1 << (key & 0x1f));

    /* Add the node to the tail of the specific queue */
    dllAdd(&pQHead->qPriBmp.head[key], &pQNode->qPrio.node);
    pQNode->qPrio.key = key;

    /* Always update the [pFirst]. */

    first = qHeadFirstFind(pQHead);
    pQHead->qPriBmp.pFirst = (Q_PRIO_NODE *)
                             DLL_HEAD(&pQHead->qPriBmp.head[first]);
}

/***************************************************************************
 * qPriBmpGet - get and remove a node from the bitmapped queue
 *
 * This routine gets and removes a node from the bitmapped queue.  In the
 * case of the ready queue, the retrieved node belongs to the task of the
 * highest running priority level.
 *
 * RETURNS: N/A
 */

LOCAL Q_PRIO_NODE* qPriBmpGet(
    Q_PRI_BMP_HEAD *pQHead
    )
{
    int i;
    int first;
    Q_PRIO_NODE *pQNode;
    DL_LIST *pHead;

    pQNode = pQHead->qPriBmp.pFirst;   /* Remove first item on queue. */

    if (pQNode != NULL)
    {
        /* <pQNode> is non-NULL. */

        i = pQNode->qPrio.key;
        pHead = &pQHead->qPriBmp.head[i];   /* Identify list. */
        dllGet(pHead);                      /* Remove node from queue. */

        /*
         * There is no need to assign the removed node, as it has already
         * been identified through [pFirst].
         */

        pQHead->qPriBmp.pFirst = NULL;   /* Assume bitmapped list is empty. */
        if (DLL_EMPTY(pHead))            /* If priority list empty mark it so */
        {
            pQHead->qPriBmp.bmp[i >> 5] &= ~(1 << (i & 0x1f));
        }

        /* If bitmappped list not empty, update [pFirst] */
        if ((first = qHeadFirstFind (pQHead)) != ERROR)
        {
            pQHead->qPriBmp.pFirst = (Q_PRIO_NODE *)
                                     DLL_HEAD(&pQHead->qPriBmp.head[first]);
        }
    }

    return pQNode;
}

/***************************************************************************
 * qPriBmpRemove - remove a node
 *
 * RETURNS: OK
 */

LOCAL STATUS qPriBmpRemove(
    Q_PRI_BMP_HEAD *pQHead,
    Q_PRIO_NODE *pQNode
    )
{
    DL_LIST *pHead;
    int key;

    key = pQNode->qPrio.key;
    pHead = &pQHead->qPriBmp.head[key];
    dllRemove(pHead, &pQNode->qPrio.node);

    if (DLL_EMPTY(pHead))
    {
        pQHead->qPriBmp.bmp[key >> 5] &= ~(1 << (key & 0x1f));
    }

    /* Update [pFirst] if necessary. */
    if (pQNode == pQHead->qPriBmp.pFirst)
    {
        pQHead->qPriBmp.pFirst = NULL;
        if ((key = qHeadFirstFind (pQHead)) != ERROR)
        {
            pQHead->qPriBmp.pFirst = (Q_PRIO_NODE *)
                                     DLL_HEAD (&pQHead->qPriBmp.head[key]);
        }
    }

    return OK;
}

/***************************************************************************
 * qPriBmpMove - move node to a new place based on the key
 *
 * RETURNS: N/A
 */

LOCAL void qPriBmpMove (
    Q_PRI_BMP_HEAD *pQHead,
    Q_PRIO_NODE *pQNode,
    int newKey
    )
{
    qPriBmpRemove(pQHead, pQNode);
    qPriBmpPut(pQHead, pQNode, newKey);
}

/***************************************************************************
 * qPriBmpNull - null method
 *
 * RETURNS: N/A
 */

LOCAL void qPriBmpNull(
    void
    )
{
    return;
}

