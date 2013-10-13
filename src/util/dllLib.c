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

/* includes */

#include <vmx.h>
#include <stdlib.h>
#include <util/dllLib.h>

/* defines */

/* typedefs */

/***************************************************************************
 * dllInit - initialize a doubly linked list
 *
 * RETURNS: OK or ERROR
 */

STATUS dllInit(
    DL_LIST *pList             /* list to initialize */
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

/***************************************************************************
 * dllAdd - add a node to the tail of the list
 *
 * This routine adds a node to the tail of the list.  It does not verify that
 * the node was not in the list in the first place.
 *
 * RETURNS: OK or ERROR
 */

STATUS dllAdd(
    DL_LIST *pList,            /* list to which to add <pNode> */
    DL_NODE *pNode             /* node to add to tail of <pList> */
    )
{
    STATUS status;
    DL_NODE *pTail;            /* tail node of <pList> */

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

        pNode->prev = pTail;
        pNode->next = NULL;
        pList->tail = pNode;
        status = OK;
    }

    return status;
}

/***************************************************************************
 * dllRemove - remove node from the list
 *
 * This routine removes a node from the list.  It does not verify that the
 * node was part of the list in the first place.
 *
 * RETURNS: OK or ERROR
 */

STATUS dllRemove(
    DL_LIST *pList,            /* list from which to remove <pNode> */
    DL_NODE *pNode             /* node to remove from <pList> */
    )
{
    STATUS status;

    DL_NODE *pPrev;            /* item preceding <pNode> */
    DL_NODE *pNext;            /* item following <pNode> */

    if (pList == NULL || pNode == NULL)
    {
        status = ERROR;
    }
    else
    {
        pPrev = pNode->prev;
        pNext = pNode->next;

        if (pPrev == NULL)         /* special case: <pNode> is @ head */
        {
            pList->head = pNext;
        }
        else                       /* general case:  update [next] ptr */
        {
            pPrev->next = pNext;
        }

        if (pNext == NULL)         /* special case: <pNode> is @ tail */
        {
            pList->tail = pPrev;
        }
        else                       /* general case: update [prev] ptr */
        {
            pNext->prev = pPrev;
        }

        status = OK;
    }

    return status;
}

/***************************************************************************
 * dllInsert - insert a node after the specified node
 *
 * This routine inserts a node after the specified node.  If NULL is specified,
 * then it inserts it at the head of the list.
 *
 * RETURNS: OK or ERROR
 */

STATUS dllInsert(
    DL_LIST *pList,            /* list to which to insert <pNode> */
    DL_NODE *pPrev,            /* node preceding <pNode> in <pList> */
    DL_NODE *pNode             /* node to insert after <pPrev> in <pList> */
    )
{
    STATUS status;
    DL_NODE *pNext;

    if (pList == NULL || pNode == NULL)
    {
        status = ERROR;
    }
    else
    {
        if (pPrev == NULL)         /* special case: <pNode> is @ head */
        {
            pNext = pList->head;
            pList->head = pNode;
        }
        else
        {                          /* general case: update <pPrev> [next] ptr */
            pNext = pPrev->next;
            pPrev->next = pNode;
        }

        if (pNext == NULL)         /* special case: <pPrev> is @ pTail */
        {
            pNode->prev = pList->tail;
            pList->tail = pNode;
        }
        else                       /* general case: update <pNext> [prev] ptr */
        {
            pNext->prev = pNode;
        }

        pNode->next = pNext;       /* update <pNode> [next] and [prev] ptrs */
        pNode->prev = pPrev;
        status = OK;
    }

    return status;
}

/***************************************************************************
 * dllGet - get (and remove) the node at the head of the list
 *
 * RETURNS: pointer to node, NULL if list is empty
 */

DL_NODE* dllGet(
    DL_LIST *pList             /* ptr to list from which to get an item */
    )
{
    DL_NODE *pNode;            /* ptr to retrieved item */

    if (pList == NULL)
    {
        pNode = NULL;
    }
    else
    {
        pNode = pList->head;
        if (pNode != NULL)         /* special case: list is empty */
        {
            pList->head = pNode->next;    /* general case: list is not empty */
            if (pNode->next == NULL)      /* special case: list only one item */
            {
                pList->tail = NULL;
            }
            else                          /* general case: list had items */
            {
                pNode->next->prev = NULL;
            }
        }
    }

    return pNode;
}

/***************************************************************************
 * dllCount - Report number of nodes in list
 *
 * RETURNS: Number of nodes in list
 */

int dllCount(
    DL_LIST *pList             /* ptr to list from which to get an item */
    )
{
    DL_NODE *pNode;            /* ptr to current node in list */
    int count;                 /* node counter */

    if (pList == NULL)
    {
        count = 0;
    }
    else
    {
        count = 0;
        pNode = DLL_HEAD(pList);   /* get first node in list */
        while (pNode != NULL)
        {
            count++;                   /* increase node counter */
            pNode = DLL_NEXT(pNode);   /* get next node in list */
        }
    }

    return count;
}

