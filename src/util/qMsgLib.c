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

/* qMsgLib.c - Used with message queues */

#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/private/kernLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/msgQLib.h>
#include <vmx/workQLib.h>
#include <vmx/errnoLib.h>
#include <vmx/sigLib.h>
#include <util/dllLib.h>
#include <util/qMsgLib.h>

/* Forward declarations */

LOCAL Q_MSG_HEAD* qMsgCreate(
    Q_CLASS_ID pendQClass
    );

LOCAL STATUS qMsgInit(
    Q_MSG_HEAD *pQHead,
    Q_CLASS_ID pendQType
    );

STATUS qMsgDestroy(
    Q_MSG_HEAD *pQHead
    );

STATUS qMsgTerminate(
    Q_MSG_HEAD *pQHead
    );

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

LOCAL int qMsgInfo(
    Q_MSG_HEAD *pQHead,
    int nodeArray[],
    int max
    );

LOCAL Q_MSG_NODE* qMsgEach(
    Q_MSG_HEAD *pQHead,
    FUNCPTR func,
    ARG arg
    );

LOCAL void qMsgNull(
    void
    );

LOCAL void qMsgPendQGet(
    MSG_Q_ID msgQId,
    Q_MSG_HEAD *pQHead
    );

/* Locals */
LOCAL Q_CLASS qMsgClass =
{
  (FUNCPTR) qMsgCreate,
  (FUNCPTR) qMsgInit,
  (FUNCPTR) qMsgDestroy,
  (FUNCPTR) qMsgTerminate,
  (FUNCPTR) qMsgPut,
  (FUNCPTR) qMsgGet,
  (FUNCPTR) qMsgNull,
  (FUNCPTR) qMsgNull,
  (FUNCPTR) qMsgNull,
  (FUNCPTR) qMsgNull,
  (FUNCPTR) qMsgNull,
  (FUNCPTR) qMsgNull,
  (FUNCPTR) qMsgInfo,
  (FUNCPTR) qMsgEach,
  &qMsgClass
};

/* Exports */
Q_CLASS_ID qMsgClassId = &qMsgClass;

/******************************************************************************
 * qMsgCreate - Create queue
 *
 * RETURNS: Queue head pointer
 */

LOCAL Q_MSG_HEAD* qMsgCreate(
    Q_CLASS_ID pendQType
    )
{
    Q_MSG_HEAD *pQHead;

    /* Allocate memory for struct */
    pQHead = malloc(sizeof(Q_MSG_HEAD));
    if (pQHead != NULL)
    {
        /* Call initializer below */
        if (qMsgInit(pQHead, pendQType) != OK)
        {
            free(pQHead);
            pQHead = NULL;
        }
    }

  return pQHead;
}

/******************************************************************************
 * qMsgInit - Initialize queue
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS qMsgInit(
    Q_MSG_HEAD *pQHead,
    Q_CLASS_ID pendQType
    )
{
    STATUS status;

    pQHead->first = NULL;
    pQHead->last = NULL;
    pQHead->count = 0;

    if (qInit(&pQHead->pendQ, pendQType, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) != OK)
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * qMsgDestroy - Destroy queue
 *
 * RETURNS: OK
 */

STATUS qMsgDestroy(
    Q_MSG_HEAD *pQHead
    )
{
    /* End queue */
    qMsgTerminate(pQHead);

    /* Free memory */
    free(pQHead);

    return OK;
}

/******************************************************************************
 * qMsgTerminate - End queue
 *
 * RETURNS: OK
 */

STATUS qMsgTerminate(
    Q_MSG_HEAD *pQHead
    )
{
    vmxPendQTerminate(&pQHead->pendQ);

    return OK;
}

/******************************************************************************
 * qMsgPut - Put a node on the queue
 *
 * RETURNS: OK or ERROR
 */

STATUS qMsgPut(
    MSG_Q_ID msgQId,
    Q_MSG_HEAD *pQHead,
    Q_MSG_NODE *pNode,
    int key
    )
{
    STATUS status;
    int level;

    /* Lock interrupts */
    INT_LOCK(level);

    if (key == Q_MSG_PRI_TAIL)
    {
        /* Add to tail */
        pNode->next = NULL;

        /* Insert */
        if (pQHead->first == NULL)
        {
            pQHead->last = pNode;
            pQHead->first = pNode;
        }
        else
        {
            pQHead->last->next = pNode;
            pQHead->last = pNode;
        }
    }
    else
    {
        /* Insert at head */
        if ((pNode->next = pQHead->first) == NULL)
        {
            pQHead->last = pNode;
        }

        pQHead->first = pNode;
    }

    /* Increase counter */
    pQHead->count++;

    if (kernelState == TRUE)
    {
        INT_UNLOCK(level);
        workQAdd2((FUNCPTR) qMsgPendQGet, msgQId, pQHead);
    }
    else
    {
        /* Check if anybody is waiting for message */
        if (Q_FIRST(&pQHead->pendQ) == NULL)
        {
            INT_UNLOCK(level);
            status = OK;
        }
        else
        {
            /* Unlock pedning task waiting for message */
            kernelState = TRUE;
            INT_UNLOCK(level);
            vmxPendQGet(&pQHead->pendQ);
            vmxExit();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * qMsgPendQGet - Kernel routine to unblock
 *
 * RETURNS: N/A
 */

LOCAL void qMsgPendQGet(
    MSG_Q_ID msgQId,
    Q_MSG_HEAD *pQHead
    )
{
    if (Q_FIRST(&pQHead->pendQ) != NULL)
    {
        vmxPendQGet(&pQHead->pendQ);
    }
}

/******************************************************************************
 * qMsgGet - Get and remove a node from queue
 *
 * RETURNS: Pointer to node or NULL
 */

Q_MSG_NODE* qMsgGet(
    MSG_Q_ID msgQId,
    Q_MSG_HEAD *pQHead,
    unsigned timeout
    )
{
    STATUS status;
    int level;
    Q_MSG_NODE *pNode;

    /* Lock interrupts */
    INT_LOCK(level);

    while ((pNode = pQHead->first) == NULL)
    {
        if (timeout == WAIT_NONE)
        {
            errnoSet(S_msgQLib_INVALID_TIMEOUT);
            INT_UNLOCK(level);
            pNode = NULL;
        }
        else
        {
            /* Enter kernel mode */
            kernelState = TRUE;
            INT_UNLOCK(level);

            /* Put task on hold */
            vmxPendQPut(&pQHead->pendQ, timeout);

            /* Exit trough kernel */
            status = vmxExit();
            if (status == SIG_RESTART)
            {
                pNode = (Q_MSG_NODE *) NONE;
                break;
            }
            else
            {
                if (status != OK)
                {
                    pNode = NULL;
                    break;
                }
                else
                {
                    /* Verify object */
                    if (OBJ_VERIFY(msgQId, msgQClassId) != OK)
                    {
                        pNode = NULL;
                        break;
                    }
                    else
                    {
                        /* Lock interrupts */
                        INT_LOCK(level);
                    }
                }
            }
        }
    }

    if (pNode != NULL)
    {
        pQHead->first = pNode->next;
        pQHead->count--;
        INT_UNLOCK(level);
    }

    return pNode;
}

/******************************************************************************
 * qMsgInfo - Get info about list
 *
 * RETURNS: Number of items
 */

LOCAL int qMsgInfo(
    Q_MSG_HEAD *pQHead,
    int nodeArray[],
    int max
    )
{
    Q_MSG_NODE *pNode;
    int *pElement;
    int level;
    int count;

    /* Just count requested */
    if (nodeArray == NULL)
    {
        count = pQHead->count;
    }
    else
    {
        /* Store element start */
        pElement = nodeArray;

        /* Lock interrupts */
        INT_LOCK(level);

        /* Get first node */
        pNode = pQHead->first;

        /* While node not null and max not reached */
        while ((pNode != NULL) && (--max >= 0))
        {
            *(pElement++) = (int) pNode;
            pNode = pNode->next;
        }

        /* Unlock interrupts */
        INT_UNLOCK(level);

        count = pElement - nodeArray;
    }

    return count;
}

/******************************************************************************
 * qMsgEach - Run function for each node
 *
 * RETURNS: Pointer to node where it ended
 */

LOCAL Q_MSG_NODE* qMsgEach(
    Q_MSG_HEAD *pQHead,
    FUNCPTR func,
    ARG arg
    )
{
    Q_MSG_NODE *pNode;
    int level;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Get first node */
    pNode = pQHead->first;

    /* While node not null and function returns true */
    while ((pNode != NULL) && ((*func)(pNode, arg)))
    {
        pNode = pNode->next;
    }

    /* Unlock interrupts */
    INT_UNLOCK(level);

    return pNode;
}

/******************************************************************************
 * qMsgNull - Null method
 *
 * RETURNS: N/A
 */

LOCAL void qMsgNull(
    void
    )
{
    return;
}

