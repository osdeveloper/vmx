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

/* listLib.h - Linked list */

#ifndef _listLib_h
#define _listLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

/******************************************************************************
 * LIST_HEAD - Get head node on list
 *
 * RETURNS: Pointer to head node or NULL
 */

#define LIST_HEAD(pList)       (((LIST *)(pList))->head)

/******************************************************************************
 * LIST_TAIL - Get tail node on list
 *
 * RETURNS: Pointer to tail node or NULL
 */

#define LIST_TAIL(pList)       (((LIST *)(pList))->tail)

/******************************************************************************
 * LIST_NEXT - Get next node
 *
 * RETURNS: Pointer to next node or NULL
 */

#define LIST_NEXT(pNode)       (((LIST_NODE *)(pNode))->next)

/******************************************************************************
 * LIST_PREV - Get previous node
 *
 * RETURNS: Pointer to previous node or NULL
 */

#define LIST_PREV(pNode)       (((LIST_NODE *)(pNode))->prev)

/******************************************************************************
 * LIST_EMPTY - Check if list is empty
 *
 * RETURNS: Pointer to tail node or NULL
 */

#define LIST_EMPTY(pList)      ((((LIST *)pList)->head == NULL) )

/* A node in the linked list */
typedef struct listnode
{
    struct listnode *next;
    struct listnode *prev;
} LIST_NODE;

/* The list type itself */
typedef struct
{
    LIST_NODE *head;
    LIST_NODE *tail;
    int        count;
} LIST;

/******************************************************************************
 * listInit - Initialize linked list datastruct
 *
 * RETURNS: OK or ERROR
 */

STATUS listInit(
    LIST *pList
    );

/******************************************************************************
 * listInsert - Insert element in list
 *
 * RETURNS: OK or ERROR
 */

STATUS listInsert(
    LIST *pList,
    LIST_NODE *pPrev,
    LIST_NODE *pNode
    );

/******************************************************************************
 * listAdd - Add an element to the list
 *
 * RETURNS: OK or ERROR
 */

STATUS listAdd(
    LIST *pList,
    LIST_NODE *pNode
    );

/******************************************************************************
 * listGet - Get Node from list
 *
 * RETURNS: Pointer to list node or NULL
 */

LIST_NODE* listGet(
    LIST *pList
    );

/******************************************************************************
 * listRemove - Remove an element from the list
 *
 * RETURNS: OK or ERROR
 */

STATUS listRemove(
    LIST *pList,
    LIST_NODE *pNode
    );

/******************************************************************************
 * listCount - Get number of elements in list
 *
 * RETURNS: Number of list elements 
 */

int listCount(
    LIST *pList
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _listLib_h */

