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

/* qLib.h - Queue library */

#ifndef _qLib_h
#define _qLib_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <util/qClass.h>

/* Types */
typedef struct
{
  /* Data storage */
  void          *qPriv1;
  void          *qPriv2;
  void          *qPriv3;
  void          *qPriv4;
} Q_NODE;

typedef struct
{
  /* Node */
  Q_NODE        *pFirst;

  /* Data storage */
  void          *qPriv1;
  void          *qPriv2;

  /* Class */
  Q_CLASS       *pQClass;
} Q_HEAD;

/* Macros */
/******************************************************************************
 * Q_FIRST - Return first node in queue
 *
 * RETURNS: First node in queue
 */

#define Q_FIRST(pQHead)                                                       \
  ((Q_NODE *)( ((Q_HEAD *)(pQHead))->pFirst ))

/******************************************************************************
 * Q_PUT - Insert a node in queue
 *
 * RETURNS: N/A
 */

#define Q_PUT(pQHead, pQNode, key)                                            \
  (* ( ((Q_HEAD *)(pQHead))->pQClass->putMethod) )                            \
    ( ((Q_HEAD *)(pQHead)), ((Q_NODE *)(pQNode)), (key) )

/******************************************************************************
 * Q_GET - Get and remove fist node in queue
 *
 * RETURNS: First node on queue
 */

#define Q_GET(pQHead)                                                         \
  ( (Q_NODE *)((* (((Q_HEAD *)(pQHead))->pQClass->getMethod) )                \
    ( (Q_HEAD *)(pQHead) )) )

/******************************************************************************
 * Q_REMOVE - Remove node from queue
 *
 * RETURNS: OK or ERROR
 */

#define Q_REMOVE(pQHead, pQNode)                                              \
  (* (((Q_HEAD *)(pQHead))->pQClass->removeMethod) )                          \
    ( ((Q_HEAD *)(pQHead)), ((Q_NODE *)(pQNode)) )

/******************************************************************************
 * Q_MOVE - Move a node to a new position based on given key
 *
 * RETURNS: N/A
 */

#define Q_MOVE(pQHead, pQNode, newKey)                                        \
  (* (((Q_HEAD *)(pQHead))->pQClass->moveMethod) )                            \
    ( ((Q_HEAD *)(pQHead)), ((Q_NODE *)(pQNode)), ((unsigned long)(newKey)) )

/******************************************************************************
 * Q_ADVANCE - Advance a frame (time period)
 *
 * RETURNS: N/A
 */

#define Q_ADVANCE(pQHead)                                                     \
  (* (((Q_HEAD *)(pQHead))->pQClass->advanceMethod) )                         \
    ( ((Q_HEAD *)(pQHead)) )

/******************************************************************************
 * Q_EXPIRED - Return a time to expire node
 *
 * RETURNS: Pointer to node on head or NULL
 */

#define Q_EXPIRED(pQHead)                                                     \
  ( (Q_NODE *)((* (((Q_HEAD *)(pQHead))->pQClass->expiredMethod) )            \
    ( (Q_HEAD *)(pQHead) )) )

/******************************************************************************
 * Q_KEY - Get key of node
 *
 * RETURNS: Key for node
 */

#define Q_KEY(pQHead, pQNode, keyType)                                        \
  (* (((Q_HEAD *)(pQHead))->pQClass->keyMethod))                              \
    ( ((Q_NODE *)(pQNode)), ((int)(keyType)) )

/******************************************************************************
 * Q_OFFSET - Calibrate each node in queue by some offset
 *
 * RETURNS: N/A
 */

#define Q_OFFSET(pQHead, keyDelta)                                            \
  (* (((Q_HEAD *)(pQHead))->pQClass->offsetMethod) )                          \
    ( ((Q_HEAD *)(pQHead)), ((unsigned long)(keyDelta)) )

/******************************************************************************
 * Q_INFO - Gather info about queue
 *
 * RETURNS: Number of nodes copied into array
 */

#define Q_INFO(pQHead, nodeArray, maxNodes)                                   \
  (* (((Q_HEAD *)(pQHead))->pQClass->infoMethod) )                            \
    ( ((Q_HEAD *)(pQHead)), ((int *)(nodeArray)), ((int)(maxNodes)) )

/******************************************************************************
 * Q_EACH - Run function for each node in queue
 *
 * RETURNS: NULL of the node where it ended
 */

#define Q_EACH(pQHead, func, arg)                                             \
  ( (Q_NODE *)((* (((Q_HEAD *)(pQHead))->pQClass->eachMethod) )               \
    ( ((Q_HEAD *)(pQHead)), ((FUNCPTR)(func)), ((int)(arg)) ))  )

/* Functions */

/******************************************************************************
 * qCreate - Create a new queue
 *
 * RETURNS: Pointer to head or NULL
 */

Q_HEAD* qCreate(
    Q_CLASS *pQClass,
    ...
    );

/******************************************************************************
 * qInit - Initialize queue structure
 *
 * RETURNS: OK or ERROR
 */

STATUS qInit(
    Q_HEAD *pQHead,
    Q_CLASS *pQClass,
    ...
    );

/******************************************************************************
 * qDestroy - Deallocate
 *
 * RETURNS: OK or ERROR
 */

STATUS qDestroy(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * qTerminate - Finnish of head node
 *
 * RETURNS: OK or ERROR
 */

STATUS qTerminate(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * qFirst - Get first node in queue
 *
 * RETURNS: Pointer to first node in queue
 */

Q_NODE* qFirst(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * qPut - Put node into queue
 *
 * RETURNS: N/A
 */

void qPut(
    Q_HEAD *pQHead,
    Q_NODE *pQNode,
    unsigned long key
    );

/******************************************************************************
 * qGet - Get and remove first node on queue
 *
 * RETURNS: First node on queue
 */

Q_NODE* qGet(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * qRemove - Remove node from queue
 *
 * RETURNS: OK or ERROR
 */

STATUS qRemove(
    Q_HEAD *pQHead,
    Q_NODE *pQNode
    );

/******************************************************************************
 * qMove - Move a node in the key based on given key
 *
 * RETURNS: N/A
 */

void qMove(
    Q_HEAD *pQHead,
    Q_NODE *pQNode,
    unsigned long newKey
    );

/******************************************************************************
 * qAdvance - Advance a frame (time slice)
 *
 * RETURNS: N/A
 */

void qAdvance(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * qExpired - Get time expire node
 *
 * RETURNS: Pointer to node on head or NULL
 */

Q_NODE* qExpired(
    Q_HEAD *pQHead
    );

/******************************************************************************
 * qKey - Get key of node
 *
 * RETURNS: Key for node
 */

unsigned long qKey(
    Q_HEAD *pQHead,
    Q_NODE *pQNode,
    int keyType
    );

/******************************************************************************
 * qOffset - Calibrate each node in queue with some delta
 *
 * RETURNS: N/A
 */

void qOffset(
    Q_HEAD *pQHead,
    unsigned long keyDelta
    );

/******************************************************************************
 * qInfo - Gather info about queue
 *
 * RETURNS: Number of node pointers in array
 */

int qInfo(
    Q_HEAD *pQHead,
    Q_NODE nodeArray[],
    int maxNodes
    );

/******************************************************************************
 * qEach - Call function for each node in queue
 *
 * RETURNS: NULL of whore queue processed or pointer to Q_NODE when ended
 */

Q_NODE* qEach(
    Q_HEAD *pQHead,
    FUNCPTR func,
    int arg
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _qLib_h */

