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

#ifndef _dllLib_h
#define _dllLib_h

#ifdef __cplusplus
extern "C" {
#endif

/* defines */

/******************************************************************************
 * DLL_NEXT - Get next node
 *
 * RETURNS: Pointer to next node or NULL
 */

#define DLL_NEXT(node)         (((node))->next)

/******************************************************************************
 * DLL_PREV - Get previous node
 *
 * RETURNS: Pointer to previous node or NULL
 */

#define DLL_PREV(node)         (((node))->prev)

/******************************************************************************
 * DLL_HEAD - Get head node on list
 *
 * RETURNS: Pointer to head node or NULL
 */

#define DLL_HEAD(list)         (((list))->head)

/******************************************************************************
 * DLL_TAIL - Get tail node on list
 *
 * RETURNS: Pointer to tail node or NULL
 */

#define DLL_TAIL(list)         (((list))->tail)

/******************************************************************************
 * DLL_EMPTY - Check if list is empty
 *
 * RETURNS: Pointer to tail node or NULL
 */

#define DLL_EMPTY(list)        (((list)->head == NULL))

/* typedefs */

typedef struct dl_node
{
    struct dl_node *prev;
    struct dl_node *next;
} DL_NODE;

typedef struct dl_list
{
    DL_NODE *head;
    DL_NODE *tail;
} DL_LIST;

/***************************************************************************
 * dllInit - initialize a doubly linked list
 *
 * RETURNS: OK or ERROR
 */

STATUS dllInit(
    DL_LIST *pList             /* list to initialize */
    );

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
    );

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
    );

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
    );

/***************************************************************************
 * dllGet - get (and remove) the node at the head of the list
 *
 * RETURNS: pointer to node, NULL if list is empty
 */

DL_NODE* dllGet(
    DL_LIST *pList             /* ptr to list from which to get an item */
    );

/***************************************************************************
 * dllCount - Report number of nodes in list
 *
 * RETURNS: Number of nodes in list
 */

int dllCount(
    DL_LIST *pList             /* ptr to list from which to get an item */
    );


#ifdef __cplusplus
}
#endif

#endif

