/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* listLib.c - A double linked list library */

#include <vmx.h>
#include <vmx/listLib.h>

/**************************************************************
* listInit - Initialize linked list datastruct
*
* RETURNS: OK
**************************************************************/

STATUS listInit(LIST *pList)
{
  pList->head = NULL;
  pList->tail = NULL;

  return(OK);
}

/**************************************************************
* listInsert - Insert element in list
*
* RETURNS: N/A
**************************************************************/

void listInsert(LIST *pList,
		LIST_NODE *pPrev,
		LIST_NODE *pNode)
{
  LIST_NODE *pNext;
  
  if (pPrev == NULL) {
    pNext = pList->head;	/* Put node at beginnig of list */
    pList->head = pNode;
  }
  else {
    pNext = pPrev->next;	/* next points to previous next */
    pPrev->next = pNode;
  }

  if (pNext == NULL)
    pList->tail = pNode;	/* Put node at end of list */
  else
    pNext->prev = pNode;	/* next previous points to node */

  /* Setup pointers */
  pNode->next = pNext;
  pNode->prev = pPrev;
}

/**************************************************************
* listAdd - Add an element to the list
*
* RETURNS: N/A
**************************************************************/

void listAdd(LIST *pList, LIST_NODE *pNode)
{
  listInsert(pList, pList->tail, pNode);
}

/**************************************************************
* listGet - Get Node from list
*
* RETURNS: Node
**************************************************************/

LIST_NODE *listGet(LIST *pList)
{
  LIST_NODE *pNode = pList->head;

  if (pNode != NULL)
  {
    pList->head = pNode->next;

    if (pNode->next == NULL)
      pList->tail = NULL;
    else
      pNode->next->prev = NULL;
  }

  return (pNode);
}

/**************************************************************
* listRemove - Remove an element from the list
*
* RETURNS: N/A
**************************************************************/

void listRemove(LIST *pList, LIST_NODE *pNode)
{
  /* Is this the first node? */
  if (pNode->prev == NULL)
    pList->head = pNode->next;
  else
    pNode->prev->next = pNode->next;

  /* Is this the last node? */
  if (pNode->next == NULL)
    pList->tail = pNode->prev;
  else
    pNode->next->prev = pNode->prev;
}

