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

/* memPartLibP.h - Private memory partition library header */

#ifndef _memPartLibP_h
#define _memPartLibP_h

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx.h>
#include <util/dllLib.h>
#include <vmx/classLib.h>
#include <vmx/private/objLibP.h>
#include <vmx/private/semLibP.h>

/* Macros */

/******************************************************************************
 * NEXT_HEADER - Get next memory block header
 *
 * RETURNS: Pointer to memory block header
 */

#define NEXT_HEADER(pHeader)                                                  \
    ((BLOCK_HEADER *)((char *)(pHeader) + (2 * (pHeader)->nWords)))

/******************************************************************************
 * PREV_HEADER - Get previous memory block header
 *
 * RETURNS: Pointer to memory block header
 */

#define PREV_HEADER(pHeader)                                                  \
    ((pHeader)->pPrevHeader)

/******************************************************************************
 * HEADER_TO_BLOCK - Get memory block for header
 *
 * RETURNS: Pointer to memory block
 */

#define HEADER_TO_BLOCK(pHeader)                                              \
    ((char *)((int)pHeader + sizeof(BLOCK_HEADER)))

/******************************************************************************
 * BLOCK_TO_HEADER - Get header for memory block
 *
 * RETURNS: Pointer to memory block header
 */

#define BLOCK_TO_HEADER(pBlock)                                               \
    ((BLOCK_HEADER *)((int)pBlock - sizeof(BLOCK_HEADER)))

/******************************************************************************
 * HEADER_TO_NODE - Get list node from memory block header
 *
 * RETURNS: Pointer to list node
 */

#define HEADER_TO_NODE(pHeader)                                               \
    (&((FREE_BLOCK *) pHeader)->node)

/******************************************************************************
 * NODE_TO_HEADER - Get memory block header from list node
 *
 * RETURNS: Pointer to memory block header
 */

#define NODE_TO_HEADER(pNode)                                                 \
    ((BLOCK_HEADER *) ((int)pNode - OFFSET(FREE_BLOCK,node)))

typedef struct mem_part
{
    OBJ_CORE  objCore;
    DL_LIST   freeList;
    SEMAPHORE sem;
    unsigned  totalWords;
    unsigned  minBlockWords;
    unsigned  options;
    unsigned  currBlocksAlloced;
    unsigned  currWordsAlloced;
    unsigned  cumBlocksAlloced;
    unsigned  cumWordsAlloced;
} PARTITION;

typedef struct blockHeader
{
    struct blockHeader *pPrevHeader;
    unsigned            nWords : 31;
    unsigned            free   :  1;
} BLOCK_HEADER;

typedef struct
{
    struct
    {
        struct blockHeader *pPrevHeader;
        unsigned            nWords : 31;
        unsigned            free   :  1;
    } header;

    DL_NODE               node;
} FREE_BLOCK;

/******************************************************************************
 * memPartBlockValidate - Check validity of a block
 *
 * RETURNS: TRUE or FALSE
 */

BOOL memPartBlockValidate(
    PART_ID partId,
    BLOCK_HEADER *pHeader,
    BOOL isFree
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _memPartLibP_h */

