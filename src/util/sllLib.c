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

/* sllLib.c - Singly linked list library */

/* includes */

#include <vmx.h>
#include <stdlib.h>
#include <util/sllLib.h>

/* defines */

/* typedefs */

/******************************************************************************
 * sllInit - initialize a singly linked list
 *
 * RETURNS: OK or ERROR
 */

STATUS sllInit(
    SL_LIST *pList             /* list to initialize */
    )
 {
    STATUS status;

    if (pList == NULL)
    {
        status = ERROR;
    }
    else
    {
        pList->head = NULL;
        pList->tail = NULL;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sllPutAtHead - Put node at list head
 *
 * RETURNS: OK or ERROR
 */

STATUS sllPutAtHead(
    SL_LIST *pList,
    SL_NODE *pNode
    )
{
    STATUS status;

    if (pList == NULL || pNode == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Next element will be head node */
        pNode->next = pList->head;

        /* If this is the first node */
        if (pList->head == NULL)
        {
            pList->head = pNode;
            pList->tail = pNode;
        }
        else
        {
            pList->head = pNode;
        }
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sllPutAtTail - Put node at tail of list
 *
 * RETURNS: OK or ERROR
 */

STATUS sllPutAtTail(
    SL_LIST *pList,
    SL_NODE *pNode
    )
{

    STATUS status;

    if (pList == NULL || pNode == NULL)
    { 
        status = ERROR;
    }
    else
    {
        /* Set next element as zero */
        pNode->next = NULL;

        /* If this is the first element in the list */
        if (pList->head == NULL)
        {
            pList->tail = pNode;
            pList->head = pNode;
        }
        else
        {
            pList->tail->next = pNode;
            pList->tail = pNode;
        }
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sllPrevious - Find previous node in list
 *
 * RETURNS: Pointer to node
 */

SL_NODE* sllPrevious(
    SL_LIST *pList,
    SL_NODE *pNode
    )
{
    STATUS status;
    SL_NODE *pLocalNode, *pFoundNode;

    if (pList == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Get head node in list */
        pLocalNode = pList->head;

        /* If list empty or it head is the sent node */
        if ((pLocalNode == NULL) || (pLocalNode == pNode))
        {
            pFoundNode = NULL;
        }
        else
        {
            pFoundNode = NULL;

            /* While more nodes in list */
            while (pLocalNode->next != NULL)
            {
                /* Found node */
                if (pLocalNode->next == pNode)
                {
                    pFoundNode = pLocalNode;
                    break;
                }

                /* Advance */
                pLocalNode = pLocalNode->next;
            }
        }
    }

    return pFoundNode;
}

/******************************************************************************
 * sllAdd - add a node to the tail of the list
 *
 * This routine adds a node to the tail of the list.  It does not verify that
 * the node was not in the list in the first place.
 *
 * RETURNS: OK or ERROR
 */

STATUS sllAdd(
    SL_LIST *pList,            /* list to which to add <pNode> */
    SL_NODE *pNode             /* node to add to tail of <pList> */
    )
{
    STATUS status;
    SL_NODE *pTail;            /* tail node of <pList> */

    if (pList == NULL || pNode == NULL)
    {
        status = ERROR;
    }
    else
    {
        if (pList->head == NULL)
        {
            pList->head = pNode;
        }

        pTail = pList->tail;
        if (pTail != NULL)
        {
            pTail->next = pNode;
        }

        pNode->next = NULL;
        pList->tail = pNode;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sllRemove - remove node from the list
 *
 * This routine removes a node from the list.  It does not verify that the
 * node was part of the list in the first place.
 *
 * RETURNS: OK or ERROR
 */

STATUS sllRemove(
    SL_LIST *pList,            /* list from which to remove <pNode> */
    SL_NODE *pPrev,            /* node preceding <pNode> */
    SL_NODE *pNode             /* node to remove from <pList> */
    )
{
    STATUS status;
    SL_NODE *pNext;            /* item following <pNode> */

    if (pList == NULL || pNode == NULL)
    {
        status = ERROR;
    }
    else
    {
        pNext = pNode->next;
        if (pPrev == NULL)             /* special case: <pNode> is @ head */
        {
            pList->head = pNext;
        }
        else                           /* general case:  update [next] ptr */
        {
            pPrev->next = pNext;
        }

        if (pNext == NULL)             /* special case: <pNode> is @ tail */
        {
            pList->tail = pPrev;
        }
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sllInsert - insert a node after the specified node
 *
 * This routine inserts a node after the specified node.  If NULL is specified,
 * then it inserts it at the head of the list.
 *
 * RETURNS: OK or ERROR
 */

STATUS sllInsert(
    SL_LIST *pList,            /* list to which to insert <pNode> */
    SL_NODE *pPrev,            /* node preceding <pNode> in <pList> */
    SL_NODE *pNode             /* node to insert after <pPrev> in <pList> */
    )
{
    STATUS status;
    SL_NODE *pNext;

    if (pList == NULL || pNode == NULL)
    {
        status = ERROR;
    }
    else
    {
        if (pPrev == NULL)        /* special case: <pNode> is @ head */
        {
            pNext = pList->head;
            pList->head = pNode;
        }
        else
        {                        /* general case: update <pPrev> [next] ptr */
            pNext = pPrev->next;
            pPrev->next = pNode;
        }

        if (pNext == NULL)       /* special case: <pPrev> is @ tail */
        {
            pList->tail = pNode;
        }

        pNode->next = pNext;     /* update <pNode> [next] and [prev] ptrs */
        status = OK;
    }

    return status;
}

/******************************************************************************
 * sllGet - get (and remove) the node at the head of the list
 *
 * RETURNS: pointer to node, NULL if list is empty
 */

SL_NODE* sllGet(
    SL_LIST *pList             /* ptr to list from which to get an item */
    )
{
    SL_NODE *pNode;              /* ptr to retrieved item */

    if (pList == NULL)
    {
        pNode = NULL;
    }
    else
    {
        pNode = pList->head;
        if (pNode != NULL)                /* special case: list is empty */
        {
            pList->head = pNode->next;    /* general case: list is not empty */
            if (pNode->next == NULL)      /* special case: only one item */
            {
                pList->tail = NULL;
            }
        }
    }

    return pNode;
}

/******************************************************************************
 * sllCount - Report number of nodes in list
 *
 * RETURNS: Number of nodes in list
 */

int sllCount(
    SL_LIST *pList
    )
{
    SL_NODE *pNode;              /* ptr to list */
    int count;                   /* counter for nodes */

    if (pList == NULL)
    {
        count = 0;
    }
    else
    {
        count = 0;
        while (pNode != NULL)
        {
            count++;                 /* increase counter */
            pNode = SLL_NEXT(pNode); /* get next node in list */
        }
    }

    return count;
}

/******************************************************************************
 * sllEach - Called for each node in list
 *
 * RETURNS: Pointer to node where it stopped
 */

SL_NODE* sllEach(
    SL_LIST *pList,
    FUNCPTR func,
    ARG arg
    )
{
    SL_NODE *pCurrNode, *pNextNode;

    if (pList == NULL || func == NULL)
    {
        pCurrNode = NULL;
    }
    else
    {
        /* Get first node in list */
        pCurrNode = SLL_HEAD(pList);

        /* While current node non-null */
        while (pCurrNode != NULL)
        {
            /* Get next node in list */
            pNextNode = SLL_NEXT(pCurrNode);

            /* Call function on current node */
            if ((*func)(pCurrNode, arg) == FALSE)
            {
                break;
            }

            /* Advance */
            pCurrNode = pNextNode;
        }
    }

    return pCurrNode;
}

