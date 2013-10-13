/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 - 2009 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* qFifoLib.c - Fifo queue */

#include <stdlib.h>
#include <vmx.h>
#include <util/qFifoLib.h>

/* Forward declarations */
LOCAL Q_FIFO_HEAD* qFifoCreate(void);
LOCAL STATUS qFifoInit(Q_FIFO_HEAD *pQFifoHead);
LOCAL STATUS qFifoDestroy(Q_FIFO_HEAD *pQFifoHead);
LOCAL void qFifoPut(Q_FIFO_HEAD *pQFifoHead,
		    Q_FIFO_NODE *pQFifoNode,
		    int key);
LOCAL Q_FIFO_NODE* qFifoGet(Q_FIFO_HEAD *pQFifoHead);
LOCAL STATUS qFifoRemove(Q_FIFO_HEAD *pQFifoHead,
			 Q_FIFO_NODE *pQFifoNode);
LOCAL int qFifoInfo(Q_FIFO_HEAD *pQFifoHead, int nodeArray[], int maxNodes);
LOCAL Q_FIFO_NODE* qFifoEach(Q_FIFO_HEAD *pQFifoHead, FUNCPTR func, int arg);
LOCAL void qFifoNull(void);

/* Locals */
LOCAL Q_CLASS qFifoClass =
{
  (FUNCPTR) qFifoCreate,
  (FUNCPTR) qFifoInit,
  (FUNCPTR) qFifoDestroy,
  (FUNCPTR) qFifoNull,
  (FUNCPTR) qFifoPut,
  (FUNCPTR) qFifoGet,
  (FUNCPTR) qFifoRemove,
  (FUNCPTR) qFifoNull,
  (FUNCPTR) qFifoNull,
  (FUNCPTR) qFifoNull,
  (FUNCPTR) qFifoNull,
  (FUNCPTR) qFifoNull,
  (FUNCPTR) qFifoInfo,
  (FUNCPTR) qFifoEach,
  &qFifoClass
};

Q_CLASS_ID qFifoClassId = &qFifoClass;

/*******************************************************************************
* qFifoCreate - Create queue
*
* RETURNS: Queue head pointer
*******************************************************************************/

LOCAL Q_FIFO_HEAD* qFifoCreate(void)
{
  Q_FIFO_HEAD *pQFifoHead;

  /* Allocate memory for struct */
  pQFifoHead = malloc( sizeof(Q_FIFO_HEAD) );

  if (pQFifoHead == NULL) {
    return(NULL);
  }

  /* Call initializer below */
  if (qFifoInit(pQFifoHead) != OK) {
    return(NULL);
  }

  return(pQFifoHead);
}

/*******************************************************************************
* qFifoInit - Initialize queue
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS qFifoInit(Q_FIFO_HEAD *pQFifoHead)
{

  dllInit(&pQFifoHead->qFifo.head);

  return(OK);
}

/*******************************************************************************
* qFifoDestroy - Deallocate queue
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS qFifoDestroy(Q_FIFO_HEAD *pQFifoHead)
{
  /* Free memory */
  free(pQFifoHead);

  return(OK);
}

/*******************************************************************************
* qFifoPut - Put a node on the queue
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL void qFifoPut(Q_FIFO_HEAD *pQFifoHead,
		    Q_FIFO_NODE *pQFifoNode,
		    int key)
{
  if (key == FIFO_KEY_HEAD)
    dllInsert (&pQFifoHead->qFifo.head, (DL_NODE *) NULL,
	       &pQFifoNode->qFifo.node);
  else
    dllAdd(&pQFifoHead->qFifo.head, &pQFifoNode->qFifo.node);
}

/*******************************************************************************
* qFifoGet - Get and remove a node from queue
*
* RETURNS: Pointer to node
*******************************************************************************/

LOCAL Q_FIFO_NODE* qFifoGet(Q_FIFO_HEAD *pQFifoHead)
{
  if (DLL_EMPTY(&pQFifoHead->qFifo.head))
    return(NULL);

  return ((Q_FIFO_NODE *) dllGet(&pQFifoHead->qFifo.head));
}

/*******************************************************************************
* qFifoListRemove - Remove node
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS qFifoRemove(Q_FIFO_HEAD *pQFifoHead,
			 Q_FIFO_NODE *pQFifoNode)
{
  dllRemove(&pQFifoHead->qFifo.head, &pQFifoNode->qFifo.node);

  return(OK);
}

/*******************************************************************************
* qFifoInfo - Get info about queue
*
* RETURNS: Number of infos read
*******************************************************************************/

LOCAL int qFifoInfo(Q_FIFO_HEAD *pQFifoHead, int nodeArray[], int maxNodes)
{
  DL_NODE *pNode;
  int *pElement;

  /* If null array just return number of nodes */
  if (nodeArray == NULL)
    return dllCount((DL_LIST *) pQFifoHead);

  /* Get first node */
  pNode = (DL_NODE *) DLL_HEAD( (DL_LIST *) pQFifoHead);

  /* Set element pointer */
  pElement = nodeArray;

  /* While node in list or maxNodex GE 0*/
  while ( (pNode != NULL) && (--maxNodes >= 0) ) {

    *(pElement++) = (int) pNode;
    pNode = DLL_NEXT(pNode);

  }

  /* Return numbder of nodes read */
  return (pElement - nodeArray);
}

/*******************************************************************************
* qFifoEach - Run function for each node in queue
*
* RETURNS: NULL or node where it ended
*******************************************************************************/

LOCAL Q_FIFO_NODE* qFifoEach(Q_FIFO_HEAD *pQFifoHead, FUNCPTR func, int arg)
{
  DL_NODE *pNode;

  /* Get first node */
  pNode = (DL_NODE *) DLL_HEAD( (DL_LIST *) pQFifoHead);

  /* While node in list or nonzero return from function */
  while ( (pNode != NULL) && ((* func) (pNode, arg)) )
    pNode = DLL_NEXT(pNode);

  return (Q_FIFO_NODE *) pNode;
}

/*******************************************************************************
* qFifoNull - Null method
*
* RETURNS: N/A
*******************************************************************************/

LOCAL void qFifoNull(void)
{
  return;
}
