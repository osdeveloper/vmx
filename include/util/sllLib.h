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

#ifndef _sllLib_h
#define _sllLib_h

#ifdef __cplusplus
extern "C" {
#endif

/* defines */

/******************************************************************************
 * SLL_NEXT - Get next node
 *
 * RETURNS: Pointer to next node or NULL
 */

#define SLL_NEXT(node)         (((node))->next)

/******************************************************************************
 * SLL_HEAD - Get head node on list
 *
 * RETURNS: Pointer to head node or NULL
 */

#define SLL_HEAD(list)         (((list))->head)

/******************************************************************************
 * SLL_TAIL - Get tail node on list
 *
 * RETURNS: Pointer to tail node or NULL
 */

#define SLL_TAIL(list)         (((list))->tail)

/******************************************************************************
 * SLL_EMPTY - Check if list is empty
 *
 * RETURNS: Pointer to tail node or NULL
 */

#define SLL_EMPTY(list)        (((list)->head == NULL))

/* typedefs */

typedef struct sl_node
{
    struct sl_node *next;
} SL_NODE;

typedef struct sl_list
{
    SL_NODE *head;
    SL_NODE *tail;
} SL_LIST;

/******************************************************************************
 * sllInit - initialize a singly linked list
 *
 * RETURNS: OK or ERROR
 */

STATUS sllInit(
    SL_LIST *pList             /* list to initialize */
    );

/******************************************************************************
 * sllPutAtHead - Put node at list head
 *
 * RETURNS: OK or ERROR
 */

STATUS sllPutAtHead(
    SL_LIST *pList,
    SL_NODE *pNode
    );

/******************************************************************************
 * sllPutAtTail - Put node at tail of list
 *
 * RETURNS: OK or ERROR
 */

STATUS sllPutAtTail(
    SL_LIST *pList,
    SL_NODE *pNode
    );

/******************************************************************************
 * sllPrevious - Find previous node in list
 *
 * RETURNS: Pointer to node
 */

SL_NODE* sllPrevious(
    SL_LIST *pList,
    SL_NODE *pNode
    );

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
    );

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
    );

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
    );

/******************************************************************************
 * sllGet - get (and remove) the node at the head of the list
 *
 * RETURNS: pointer to node, NULL if list is empty
 */

SL_NODE* sllGet(
    SL_LIST *pList             /* ptr to list from which to get an item */
    );

/******************************************************************************
 * sllCount - Report number of nodes in list
 *
 * RETURNS: Number of nodes in list
 */

int sllCount(
    SL_LIST *pList
    );

/******************************************************************************
 * sllEach - Called for each node in list
 *
 * RETURNS: Pointer to node where it stopped
 */

SL_NODE* sllEach(
    SL_LIST *pList,
    FUNCPTR func,
    ARG arg
    );

#ifdef __cplusplus
}
#endif

#endif

