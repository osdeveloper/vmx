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

/* udvmxmem.c - Universal graphics library memory pool management */

#include <stdlib.h>
#include <string.h>

#include <os/memPartLib.h>

#include "ugl.h"

/* Types */

typedef struct {
    UGL_UINT32       magicNumber;
    UGL_MEM_POOL_ID  poolId;
    UGL_SIZE         memSize;
} UGL_MEM_HEADER;

/* Locals */

UGL_LOCAL UGL_UINT32 uglMemMagicNumber = 0xabcd;

/******************************************************************************
 *
 * uglOSMemPoolCreate - Create memory pool
 *
 * RETURNS: Memory pool id, or UGL_NULL
 */

UGL_MEM_POOL_ID uglOSMemPoolCreate (
    void *    poolAddr,
    UGL_SIZE  poolSize
    ) {

    return memPartCreate ((char *) poolAddr, poolSize);
}

/******************************************************************************
 *
 * uglOSMemPoolDestroy - Destroy memory pool
 *
 * RETURNS: UGL_STATUS_ERROR
 */

UGL_STATUS uglOSMemPoolDestroy (
    UGL_MEM_POOL_ID  poolId
    ) {

    return (UGL_STATUS_ERROR);
}

/******************************************************************************
 *
 * uglOSMemAlloc - Allocate memory from memory pool
 *
 * RETURNS: Pointer to memory, or UGL_NULL
 */

void * uglOSMemAlloc (
    UGL_MEM_POOL_ID  poolId,
    UGL_SIZE         memSize
    ) {
    UGL_MEM_HEADER * pHeader;

    /* Add size for header */
    memSize += sizeof (UGL_MEM_HEADER);

    /* If no pool given, allocate from default pool */
    if (poolId == UGL_NULL) {
        uglMemDefaultPoolGet (&poolId);
        if (poolId == UGL_NULL) {
            pHeader = (UGL_MEM_HEADER *) malloc (memSize);
        }
        else {
            pHeader = (UGL_MEM_HEADER *) memPartAlloc (poolId, memSize);
        }
    }
    else {
        pHeader = (UGL_MEM_HEADER *) memPartAlloc (poolId, memSize);
    }

    /* Setup header */
    if (pHeader != UGL_NULL) {
        pHeader->magicNumber = uglMemMagicNumber;
        pHeader->poolId      = poolId;
        pHeader->memSize     = memSize;
    }

    return (&pHeader[1]);
}

/******************************************************************************
 *
 * uglOSMemCalloc - Allocate memory objects from memory pool and clear it
 *
 * RETURNS: Pointer to memory, or UGL_NULL
 */

void * uglOSMemCalloc (
    UGL_MEM_POOL_ID  poolId,
    UGL_ORD          numItems,
    UGL_SIZE         itemSize
    ) {
    UGL_SIZE  memSize;
    void *    pMem;

    /* Calculate size */
    memSize = numItems * itemSize;

    /* Allocate memory */
    pMem = uglOSMemAlloc (poolId, memSize);

    /* Clear memory */
    if (pMem != UGL_NULL) {
        memset (pMem, 0, memSize);
    }

    return (pMem);
}

/******************************************************************************
 *
 * uglOSMemRealloc - Change size of allocated memory from memory pool
 *
 * RETURNS: Pointer to memory, or UGL_NULL
 */

void * uglOSMemRealloc (
    UGL_MEM_POOL_ID  poolId,
    void *           pMem,
    UGL_SIZE         memSize
    ) {
    UGL_MEM_HEADER * pPrevHeader;
    UGL_MEM_HEADER * pHeader;
    void *           pNewMem;

    /* Allocate new memory */
    if (pMem == UGL_NULL) {
        return uglOSMemAlloc (poolId, memSize);
    }

    /* Add size for header */
    memSize += sizeof (UGL_MEM_HEADER);

    /* Get memory pool */
    if (poolId == UGL_NULL) {
        uglMemDefaultPoolGet (&poolId);
    }

    /* Get previous header */
    pPrevHeader = (UGL_MEM_HEADER *) ((char *) pMem - sizeof (UGL_MEM_HEADER));

    /* Check previous header */
    if (pPrevHeader->magicNumber != uglMemMagicNumber) {
        return (UGL_NULL);
    }

    if (memSize != pPrevHeader->memSize || poolId != pPrevHeader->poolId) {
        if (poolId == UGL_NULL && pPrevHeader->poolId == UGL_NULL) {
            pHeader = (UGL_MEM_HEADER *) realloc (pPrevHeader, memSize);
            if (pHeader == UGL_NULL) {
                return (UGL_NULL);
            }

            pHeader->magicNumber = uglMemMagicNumber;
            pHeader->poolId      = poolId;
            pHeader->memSize     = memSize;
            pNewMem = (void *) &pHeader[1];
        }
        else {
            pNewMem = uglOSMemAlloc (poolId, memSize);
            if (pNewMem == UGL_NULL) {
                return (UGL_NULL);
            }

            memcpy (pNewMem, pMem, min (memSize, pPrevHeader->memSize));
            uglOSMemFree (pMem);
        }
    }
    else {
        pNewMem = pMem;
    }

    return (pNewMem);
}

/******************************************************************************
 *
 * uglOSMemFree - Free memory allocated from memory pool
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSMemFree (
    void * pMem
    ) {
    UGL_MEM_HEADER * pHeader;
    UGL_MEM_POOL_ID  poolId;

    /* Check params */
    if (pMem == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get header */
    pHeader = (UGL_MEM_HEADER *) ((char *) pMem - sizeof (UGL_MEM_HEADER));

    /* Check header */
    if (pHeader->magicNumber != uglMemMagicNumber) {
        return (UGL_STATUS_ERROR);
    }

    /* Get memory pool */
    poolId = pHeader->poolId;

    /* Free memory */
    if (poolId == UGL_NULL) {
        free (pHeader);
    }
    else {
        if (memPartFree (poolId, pHeader) != OK) {
            return (UGL_STATUS_ERROR);
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglOSMemSizeGet - Get size of allocated memory
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSMemSizeGet (
    UGL_SIZE * pMemSize,
    void *     pMem
    ) {
    UGL_MEM_HEADER * pHeader;

    /* Check params */
    if (pMem == UGL_NULL || pMemSize == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get header */
    pHeader = (UGL_MEM_HEADER *) ((char *) pMem - sizeof (UGL_MEM_HEADER));

    /* Check header */
    if (pHeader->magicNumber != uglMemMagicNumber) {
        return (UGL_STATUS_ERROR);
    }

    /* Store size */
    *pMemSize = pHeader->memSize;

    return (UGL_STATUS_OK);
}

