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

/* hashLib.h - Hash table library header */

#ifndef _hashLib_h
#define _hashLib_h

#include <util/private/hashLibP.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct
{
    HASH_NODE node;            /* Hash node */
    int key;                   /* Node key */
    int data;                  /* Hash data */
} H_NODE_INT;

typedef struct
{
    HASH_NODE node;            /* Hash node */
    char *str;                 /* Node key */
    int data;                  /* Hash data */
} H_NODE_STRING;

/* Imports */
IMPORT CLASS_ID hashClassId;

/* Functions */
/******************************************************************************
 * hashLibInit - Initialize hash table library
 *
 * RETURNS: OK or ERROR
 */

STATUS hashLibInit(
    void
    );

/******************************************************************************
 * hashTableCreate - Create hash table
 *
 * RETURNS: HASH_ID or NULL
 */

HASH_ID hashTableCreate(
    int log2Size,
    FUNCPTR cmpFunc,
    FUNCPTR keyFunc,
    int keyArg
    );

/******************************************************************************
 * hashTableInit - Initialize hash table
 *
 * RETURNS: OK or ERROR
 */

STATUS hashTableInit(
    HASH_ID hashId,
    SL_LIST *pList,
    int log2Size,
    FUNCPTR cmpFunc,
    FUNCPTR keyFunc,
    int keyArg
    );

/******************************************************************************
 * hashTableDestroy - Destroy hash table
 *
 * RETURNS: OK or ERROR
 */

STATUS hashTableDestroy(
    HASH_ID hashId,
    BOOL dealloc
    );

/******************************************************************************
 * hashTableTerminate - Devalidate hash table
 *
 * RETURNS: OK or ERROR
 */

STATUS hashTableTerminate(
    HASH_ID hashId
    );

/******************************************************************************
 * hashTableDelete - Delete hash table
 *
 * RETURNS: OK or ERROR
 */

STATUS hashTableDelete(
    HASH_ID hashId
    );

/******************************************************************************
 * hashTablePut - Put a node on the hash table
 *
 * RETURNS: OK or ERROR
 */

STATUS hashTablePut(
    HASH_ID hashId,
    HASH_NODE *pHashNode
    );

/******************************************************************************
 * hashTableFind - Find node in the hash table
 *
 * RETURNS: Pointer to hash node or NULL
 */

HASH_NODE* hashTableFind(
    HASH_ID hashId,
    HASH_NODE *pMatchNode,
    int cmpArg
    );

/******************************************************************************
 * hashTableEach - Call function for each node in hash table
 *
 * RETURNS: Hash node id where it stopped
 */

HASH_NODE* hashTableEach(
    HASH_ID hashId,
    FUNCPTR func,
    ARG arg
    );

/******************************************************************************
 * hashRemove - Remove node from hash table
 *
 * RETURNS: OK or ERROR
 */

STATUS hashTableRemove(
    HASH_ID hashId,
    HASH_NODE *pHashNode
    );

/******************************************************************************
 * hashKeyCmp - Compare two hash keys
 *
 * RETURNS: TRUE or FALSE
 */

BOOL hashKeyCmp(
    H_NODE_INT *pMatchNode,
    H_NODE_INT *pHashNode,
    int keyCmpArg
    );

/******************************************************************************
 * hashStringCmp - Compare two hash key strings
 *
 * RETURNS: TRUE or FALSE
 */

BOOL hashStringCmp(
    H_NODE_STRING *pMatchNode,
    H_NODE_STRING *pHashNode,
    int keyCmpArg
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _hashLib_h */

