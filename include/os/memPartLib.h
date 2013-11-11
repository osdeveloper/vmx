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

/* memPartLib.h - Memory library header */

#ifndef _memPartLib_h
#define _memPartLib_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_part *PART_ID;

#include <ostool/moduleNumber.h>
#include <vmx.h>
#include <util/dllLib.h>
#include <os/private/memPartLibP.h>

#define S_memPartLib_NOT_INSTALLED       (M_memPartLib | 0x0001)
#define S_memPartLib_INVALID_NBYTES      (M_memPartLib | 0x0002)
#define S_memPartLib_BLOCK_ERROR         (M_memPartLib | 0x0004)
#define S_memPartLib_NOT_IMPLEMENTED     (M_memPartLib | 0xffff)

/* Exports */
IMPORT PART_ID memSysPartId;
IMPORT CLASS_ID memPartClassId;

/******************************************************************************
 * memPartLibInit - Initialize memory partition library
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartLibInit(
    char *pPool,
    unsigned poolSize
    );

/******************************************************************************
 * memPartCreate - Create memory partition
 *
 * RETURNS: Created memory partition
 */

PART_ID memPartCreate(
    char *pPool,
    unsigned poolSize
    );

/******************************************************************************
 * memPartInit - Initialize memory partition
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartInit(
    PART_ID partId,
    char *pPool,
    unsigned poolSize
    );

/******************************************************************************
 * memPartDestroy - Free memory partition
 *
 * RETURNS: ERROR
 */

STATUS memPartDestroy(
    PART_ID partId
    );

/******************************************************************************
 * memPartAddToPool - Add memory to partition pool
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartAddToPool(
    PART_ID partId,
    char *pPool,
    unsigned poolSize
    );

/******************************************************************************
 * memPartAlignedAlloc - Allocate aligned memory
 *
 * RETURNS: Pointer to memory block or NULL
 */

void* memPartAlignedAlloc(
    PART_ID partId,
    unsigned nBytes,
    unsigned alignment
    );

/******************************************************************************
 * memPartAlloc - Allocate memory
 *
 * RETURNS: Pointer to memory block or NULL
 */

void* memPartAlloc(
    PART_ID partId,
    unsigned nBytes
    );

/******************************************************************************
 * memPartRealloc - Allocate buffer of different size
 *
 * RETURNS: Pointer to memory or NULL
 */

void* memPartRealloc(
    PART_ID partId,
    void *ptr,
    unsigned nBytes
    );

/******************************************************************************
 * memPartFree - Free memory block
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartFree(
    PART_ID partId,
    void *ptr
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _memPartLib_h */

