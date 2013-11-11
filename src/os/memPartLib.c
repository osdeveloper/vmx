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

/* memPartLib.c - Memory partitions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <util/dllLib.h>
#include <vmx/semLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/vmxLib.h>
#include <vmx/taskLib.h>
#include <os/errnoLib.h>
#include <os/classLib.h>
#include <os/memLib.h>
#include <os/memPartLib.h>

/* Locals */
LOCAL OBJ_CLASS         memPartClass;
LOCAL BOOL              memPartLibInstalled = FALSE;

LOCAL void memPartSemInit(
    PART_ID partId
    );

/* Globals */
PARTITION               memSysPartition;
CLASS_ID                memPartClassId        = &memPartClass;
PART_ID                 memSysPartId          = &memSysPartition;
unsigned                memDefaultAlignment   = _ALLOC_ALIGN_SIZE;

FUNCPTR                 memPartBlockErrorFunc = NULL;
FUNCPTR                 memPartAllocErrorFunc = NULL;
FUNCPTR                 memPartSemInitFunc    = (FUNCPTR) memPartSemInit;

unsigned                memPartOptionsDefault = MEM_BLOCK_ERROR_SUSPEND_FLAG |
                                                MEM_BLOCK_CHECK;

/******************************************************************************
 * memPartLibInit - Initialize memory partition library
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartLibInit(
    char *pPool,
    unsigned poolSize
    )
{
    STATUS status;

    /* Check if library is already installed */
    if (memPartLibInstalled == TRUE) {
        status = OK;
    }
    else
    {
        /* Initialize class */
        if (classInit(
                memPartClassId, sizeof(PARTITION),
                OFFSET(PARTITION, objCore),
                memSysPartId,
                (FUNCPTR) memPartCreate,
                (FUNCPTR) memPartInit,
                (FUNCPTR) memPartDestroy
                ) != OK)
        {
            /* errno set by classInit() */
            status = ERROR;
        }
        else
        {
            /* Initialize system partition */
            memPartInit(&memSysPartition, pPool, poolSize);

            /* Mark library installed */
            memPartLibInstalled = TRUE;
        }
    }

    return status;
}

/******************************************************************************
 * memPartCreate - Create memory partition
 *
 * RETURNS: Created memory partition
 */

PART_ID memPartCreate(
    char *pPool,
    unsigned poolSize
    )
{
    PART_ID pPart;

    /* Allocate object */
    pPart = (PART_ID) objAlloc(memPartClassId);
    if (pPart == NULL) {
        /* errno set by objAlloc() */
    }
    else
    {
        /* Initialize partition */
        if (memPartInit(pPart, pPool, poolSize) != OK)
        {
            pPart = NULL;
        }
    }

    return pPart;
}

/******************************************************************************
 * memPartInit - Initialize memory partition
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartInit(
    PART_ID partId,
    char *pPool,
    unsigned poolSize
    )
{
    STATUS status;

    if (partId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Clear descriptor */
        memset(partId, 0, sizeof(*partId));

        /* Setup options */
        partId->options = memPartOptionsDefault;
        partId->minBlockWords = sizeof(FREE_BLOCK) >> 1;

        (*memPartSemInitFunc)(partId);

        /* Initialize free blocks list */
        dllInit(&partId->freeList);

        /* Initialize object class core */
        objCoreInit(&partId->objCore, memPartClassId);

        /* Add memory pool to partition  */
        memPartAddToPool(partId, pPool, poolSize);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * memPartDestroy - Free memory partition
 *
 * RETURNS: ERROR
 */

STATUS memPartDestroy(
    PART_ID partId
    )
{
    errnoSet(S_memPartLib_NOT_IMPLEMENTED);
    return ERROR;
}

/******************************************************************************
 * memPartAddToPool - Add memory to partition pool
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartAddToPool(
    PART_ID partId,
    char *pPool,
    unsigned poolSize
    )
{
    STATUS status;
    BLOCK_HEADER *pHeaderStart;
    BLOCK_HEADER *pHeaderMiddle;
    BLOCK_HEADER *pHeaderEnd;
    char *tmp;
    int reducePool;

    /* Verify object class */
    if (OBJ_VERIFY(partId, memPartClassId) != OK)
    {
        /* errno set by OBJ_VERIFY */
        status = ERROR;
    }

    /* Round address to start of memory allocation block */
    tmp = (char *) MEM_ROUND_UP(pPool);
    reducePool = tmp - pPool;
    if (poolSize >= reducePool)
    {
        poolSize -= reducePool;
    }
    else
    {
        poolSize = 0;
    }
    pPool = tmp;

    /* Round down size to memory allocation block */
    poolSize = MEM_ROUND_DOWN(poolSize);

    /* Check size */
    if ( poolSize <
           ((sizeof(BLOCK_HEADER) * 3) + (partId->minBlockWords * 2)) )
    {
        errnoSet(S_memPartLib_INVALID_NBYTES);
        status = ERROR;
    }
    else
    {
        /* Setup header at start of pool */
        pHeaderStart               = (BLOCK_HEADER *) pPool;
        pHeaderStart->pPrevHeader  = NULL;
        pHeaderStart->free         = FALSE;
        pHeaderStart->nWords       = sizeof(BLOCK_HEADER) >> 1;

        /* Setup header for block contaning allocateble memory */
        pHeaderMiddle = NEXT_HEADER(pHeaderStart);
        pHeaderMiddle->pPrevHeader = pHeaderStart;
        pHeaderMiddle->free        = TRUE;
        pHeaderMiddle->nWords      = (poolSize - 2 * sizeof(BLOCK_HEADER)) >> 1;

        /* Setup header at end of pool */
        pHeaderEnd                 = NEXT_HEADER(pHeaderMiddle);
        pHeaderEnd->pPrevHeader    = pHeaderMiddle;
        pHeaderEnd->free           = FALSE;
        pHeaderEnd->nWords         = sizeof(BLOCK_HEADER) >> 1;

        /* Lock pool */
        semTake(&partId->sem, WAIT_FOREVER);

        /* Insert nodes into free blocks list */
        dllInsert(
            &partId->freeList,
            (DL_NODE *) NULL,
            HEADER_TO_NODE(pHeaderMiddle)
            );

        /* Add size to pool */
        partId->totalWords += (poolSize >> 1);

        /* Unlock pool */
        semGive(&partId->sem);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * memAlignedBlockSplit - Split a block in the free list
 *
 * RETURNS: BLOCK_HEADER *pBlock
 */

LOCAL BLOCK_HEADER* memAlignedBlockSplit(
    PART_ID partId,
    BLOCK_HEADER *pHeader,
    unsigned nWords,
    unsigned minWords,
    unsigned alignment
    )
{
    BLOCK_HEADER *pNewHeader, *pNextHeader;
    char *pEndOfBlock, *pNewBlock;
    int blockSize;

    /* Calculate address at end of this block */
    pEndOfBlock = (char *) pHeader + (pHeader->nWords * 2);

    /* Calculate unaligned address at beginning of new block */
    pNewBlock = (char *) ( (unsigned) pEndOfBlock -
                           (nWords - sizeof(BLOCK_HEADER) / 2) * 2 );

    /* Align start address of new block */
    pNewBlock = (char *) ( (unsigned) pNewBlock & ~(alignment - 1) );

    /* Get header to new block */
    pNewHeader = BLOCK_TO_HEADER(pNewBlock);

    /* Recalculate wordsize for new block */
    blockSize = ( (char *) pNewHeader - (char *) pHeader) / 2;

    /* If block size less than minWords */
    if (blockSize < minWords)
    {
        /* Remove if new block is actually the same block as the original */
        if (pNewHeader == pHeader)
        {
            dllRemove(&partId->freeList, HEADER_TO_NODE(pHeader));
        }
        else
        {
            pNewHeader = NULL;
        }

    }
    else
    {
        /* Setup new blocks previous header pointer and size */
        pNewHeader->pPrevHeader = pHeader;
        pHeader->nWords = blockSize;
    }

    if (pNewHeader != NULL)
    {
        /* If left-over space is too small for a block of its own */
        if (  ((unsigned) pEndOfBlock - (unsigned) pNewHeader - (nWords * 2)) <
              ((minWords * 2)) )
        {
            /* Give all memory to the new block */
            pNewHeader->nWords =
                (pEndOfBlock - pNewBlock + sizeof(BLOCK_HEADER)) / 2;
            pNewHeader->free = TRUE;

            /* Setup next block previous header pointer */
            NEXT_HEADER(pNewHeader)->pPrevHeader = pNewHeader;
        }
        else
        {
            /* First set up the newly created block of the requested size */
            pNewHeader->nWords = nWords;
            pNewHeader->free = TRUE;

            /* Add left-over space to an extra block in the free list */
            pNextHeader = NEXT_HEADER(pNewHeader);
            pNextHeader->nWords =
                ((unsigned) pEndOfBlock - (unsigned) pNextHeader) / 2;
            pNextHeader->pPrevHeader = pNewHeader;
            pNextHeader->free = TRUE;

            /* Add it to the free list */
            dllAdd(&partId->freeList, HEADER_TO_NODE(pNextHeader));

            /* Set header to block after this */
            NEXT_HEADER(pNextHeader)->pPrevHeader = pNextHeader;
        }
    }

    return pNewHeader;
}

/******************************************************************************
 * memPartBlockValidate - Check validity of a block
 *
 * RETURNS: TRUE or FALSE
 */

BOOL memPartBlockValidate(
    PART_ID partId,
    BLOCK_HEADER *pHeader,
    BOOL isFree
    )
{
    BOOL valid;

    /* Start assuming the block is valid */
    valid = TRUE;

    taskLock();
    semGive(&partId->sem);

    /* Check if block header is aligned */
    if (MEM_ALIGNED(pHeader) == FALSE)
    {
        valid = FALSE;
    }

    /* Check if block size is aligned */
    if (MEM_ALIGNED(2 * pHeader->nWords) == FALSE)
    {
        valid = FALSE;
    }

    /* Check block word count */
    if (pHeader->nWords > partId->totalWords)
    {
        valid = FALSE;
    }

    /* Check if block is actually free */
    if (pHeader->free != isFree)
    {
        valid = FALSE;
    }

    /* Check next header */
    if (pHeader != PREV_HEADER(NEXT_HEADER(pHeader)))
    {
        valid = FALSE;
    }

    /* Check prev header */
    if (pHeader != NEXT_HEADER(PREV_HEADER(pHeader)))
    {
        valid = FALSE;
    }

    semTake(&partId->sem, WAIT_FOREVER);
    taskUnlock();

    return valid;
}

/******************************************************************************
 * memPartAlignedAlloc - Allocate aligned memory
 *
 * RETURNS: Pointer to memory block or NULL
 */

void* memPartAlignedAlloc(
    PART_ID partId,
    unsigned nBytes,
    unsigned alignment
    )
{
    unsigned nWords, nWordsPad;
    DL_NODE *pNode;
    BLOCK_HEADER origHeader;
    BLOCK_HEADER *pHeader, *pNewHeader;
    void *pResult;

    /* Verify object */
    if (OBJ_VERIFY(partId, memPartClassId) != OK)
    {
        pHeader = NULL;
    }
    else
    {
        /* Calculate word size including header and round up */
        nWords = (MEM_ROUND_UP(nBytes + sizeof(BLOCK_HEADER))) >> 1;

        /* If overflow */
        if ((nWords << 1) < nBytes)
        {
            if (memPartAllocErrorFunc != NULL)
            {
                (*memPartAllocErrorFunc)(partId, nBytes);
            }

            errnoSet(ENOMEM);

            /* If block error suspend flag */
            if (partId->options & MEM_ALLOC_ERROR_SUSPEND_FLAG)
            {

                if ((taskIdCurrent->options & TASK_OPTIONS_UNBREAKABLE) == 0)
                {
                    taskSuspend(0);
                }

            }

            pHeader = NULL;
        }
        else
        {
            /* Ensure that block size is greater or equal than minimum */
            if (nWords < partId->minBlockWords)
            {
                nWords = partId->minBlockWords;
            }

            semTake(&partId->sem, WAIT_FOREVER);

            /* Calculate block word size plus aligment extra */
            nWordsPad = nWords + alignment / 2;

            /* Start searching free list at head node */
            pNode = DLL_HEAD(&partId->freeList);

            /* Outer-loop until break */
            while (1)
            {
                /* Inner loop while block node is not null */
                while (pNode != NULL)
                {
                    /* Break inner loop if:
                     * - block size is greater than size + pad for alignment
                     * - block is aligned and has the correct size
                     */
                    if ((NODE_TO_HEADER(pNode)->nWords > nWordsPad) ||
                         ((NODE_TO_HEADER(pNode)->nWords == nWords) &&
                           (ALIGNED(HEADER_TO_BLOCK(NODE_TO_HEADER(pNode)),
                                                     alignment))))
                    {
                        break;
                    }

                    /* Advance to next block node in free list */
                    pNode = DLL_NEXT(pNode);
                }

                /* All the nodes it the free list has been processed */
                if (pNode == NULL)
                {
                    semGive(&partId->sem);

                    if (memPartAllocErrorFunc != NULL)
                    {
                        (*memPartAllocErrorFunc)(partId, nBytes);
                    }

                    errnoSet(ENOMEM);

                    /* If block error suspend flag */
                    if (partId->options & MEM_ALLOC_ERROR_SUSPEND_FLAG)
                    {
                        if ((taskIdCurrent->options &
                             TASK_OPTIONS_UNBREAKABLE) == 0)
                        {
                            taskSuspend(0);
                        }
                    }

                    pHeader = NULL;
                    break;
                }
                else
                {
                    /* Get header from block node just found and store it */
                    pHeader = NODE_TO_HEADER(pNode);
                    origHeader = *pHeader;

                    /* Split block just found, so that the end of the block
                     * is greater or equal to the requested size
                     */
                    pNewHeader = memAlignedBlockSplit(
                                     partId,
                                     pHeader,
                                     nWords,
                                     partId->minBlockWords,
                                     alignment
                                     );

                    /* If block succeded */
                    if (pNewHeader != NULL)
                    {
                        /* Store header to new block as requested header */
                        pHeader = pNewHeader;

                        /* Break outer-loop */
                        break;
                    }

                    /* Block split failed, it could not be split so
                     * that it has the requested size.
                     * Keep on doing the same procedure as before.
                     * - Search for block greater or equal to the size
                     * - Try to split it from the end to the requested size
                     */
                    pNode = DLL_NEXT(pNode);
                }
            }

            /* Finally!
             * Free block has been found, split from the end at the correct size
             * pHeader points to a valid memory block header
             */
            if (pHeader != NULL)
            {
                /* Mark it as allocated */
                pHeader->free = FALSE;

                /* Update memory partition statistics */
                partId->currBlocksAlloced++;
                partId->cumBlocksAlloced++;
                partId->currWordsAlloced += pHeader->nWords;
                partId->cumWordsAlloced += pHeader->nWords;

                semGive(&partId->sem);
            }
        }
    }

    if (pHeader == NULL)
    {
        pResult = NULL;
    }
    else
    {
        pResult = HEADER_TO_BLOCK(pHeader);
    }

    return pResult;
}

/******************************************************************************
 * memPartAlloc - Allocate memory
 *
 * RETURNS: Pointer to memory block or NULL
 */

void* memPartAlloc(
    PART_ID partId,
    unsigned nBytes
    )
{
    return memPartAlignedAlloc(
               partId,
               nBytes,
               memDefaultAlignment
               );
}

/******************************************************************************
 * memPartRealloc - Allocate buffer of different size
 *
 * RETURNS: Pointer to memory or NULL
 */

void* memPartRealloc(
    PART_ID partId,
    void *ptr,
    unsigned nBytes
    )
{
    unsigned nWords;
    void *pNewBlock;
    BOOL giveBackFree;
    BLOCK_HEADER *pNextHeader;
    BLOCK_HEADER *pHeader = BLOCK_TO_HEADER(ptr);

    /* Verify object class */
    if (OBJ_VERIFY(partId, memPartClassId) != OK)
    {
        pNewBlock = NULL;
    }
    else
    {
        /* If block is not allocated, then allocate new */
        if (ptr == NULL)
        {
            pNewBlock = memPartAlloc(partId, nBytes);
        }
        else
        {
            /* If zero bytes, then just free and return */
            if (nBytes == 0)
            {
                memPartFree(partId, ptr);
                pNewBlock = NULL;
            }
            else
            {
                semTake(&partId->sem, WAIT_FOREVER);

                /* Calculate size including header and aligned */
                nWords = MEM_ROUND_UP(nBytes + sizeof(BLOCK_HEADER)) >> 1;
                if (nWords < partId->minBlockWords)
                {
                    nWords = partId->minBlockWords;
                }

                /* Optional block validity check */
                if ((partId->options & MEM_BLOCK_CHECK) &&
                    (memPartBlockValidate(partId, pHeader, FALSE) == FALSE))
                {
                    semGive(&partId->sem);

                    if (memPartBlockErrorFunc != NULL)
                    {
                        (*memPartBlockErrorFunc)(partId, ptr, "memPartRealloc");
                    }

                    pNewBlock = NULL;
                }
                else
                {
                    giveBackFree = TRUE;

                    /* Test of this an increase of size */
                    if (nWords > pHeader->nWords)
                    {
                        pNextHeader = NEXT_HEADER(pHeader);

                        /* If not possible to extend */
                        if ((pNextHeader->free == FALSE) ||
                            ((pHeader->nWords + pNextHeader->nWords) < nWords))
                        {
                            semGive(&partId->sem);
                            giveBackFree = FALSE;

                            /* Allocate a completely new block and copy */
                            pNewBlock = memPartAlloc(partId, nBytes);
                            if (pNewBlock != NULL)
                            {
                                /* Copy data */
                                memcpy(
                                    pNewBlock,
                                    ptr,
                                    (size_t) (2 * pHeader->nWords -
                                              sizeof(BLOCK_HEADER))
                                    );

                                    /* Free old block */
                                    memPartFree(partId, ptr);
                            }
                        }
                        else
                        {
                            /* Take next block */
                            dllRemove(
                                &partId->freeList,
                                HEADER_TO_NODE(pNextHeader)
                                );

                            pHeader->nWords          += pNextHeader->nWords;
                            partId->currWordsAlloced += pNextHeader->nWords;
                            partId->cumWordsAlloced  += pNextHeader->nWords;

                            NEXT_HEADER(pHeader)->pPrevHeader = pHeader;
                        }
                    }

                    if (giveBackFree == TRUE)
                    {
                        /* Split of overhead and give it back */
                        pNextHeader = memAlignedBlockSplit(
                                          partId,
                                          pHeader,
                                          pHeader->nWords - nWords,
                                          partId->minBlockWords,
                                          memDefaultAlignment
                                          );

                        semGive(&partId->sem);

                        /* Free leftover block */
                        if (pNextHeader != NULL)
                        {
                            memPartFree(partId, HEADER_TO_BLOCK(pNextHeader));
                            partId->currBlocksAlloced++;
                        }
                    }
                }
            }
        }
    }

    return pNewBlock;
}

/******************************************************************************
 * memPartFree - Free memory block
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartFree(
    PART_ID partId,
    void *ptr
    )
{
    STATUS status;

    unsigned nWords;
    BLOCK_HEADER *pHeader, *pNextHeader;
    char *pBlock;

    /* Verify object */
    if (OBJ_VERIFY(partId, memPartClassId) != OK)
    {
        /* errno set by OBJ_VERIFY */
        status = ERROR;
    }
    else if (ptr == NULL)
    {
        /* Nothing to free */
        status = ERROR;
    }
    else
    {
        /* Store address as a char pointer */
        pBlock = (char *) ptr;

        /* Get header to block to free */
        pHeader = BLOCK_TO_HEADER(pBlock);

        semTake(&partId->sem, WAIT_FOREVER);

        /* If block is invalid */
        if ( (partId->options & MEM_BLOCK_CHECK) &&
              (memPartBlockValidate(partId, pHeader, FALSE) == FALSE) )
        {

            semGive(&partId->sem);

            if (memPartBlockErrorFunc != NULL)
            {
                (*memPartBlockErrorFunc)(partId, pBlock, "memPartFree");
            }

            errnoSet(S_memPartLib_BLOCK_ERROR);

            /* If block error suspend flag */
            if (partId->options & MEM_BLOCK_ERROR_SUSPEND_FLAG)
            {

                if ((taskIdCurrent->options & TASK_OPTIONS_UNBREAKABLE) == 0)
                {
                    taskSuspend(0);
                }
            }

            status = ERROR;
        }
        else
        {
            /* Get number of words in block */
            nWords = pHeader->nWords;

            /* If the previous block is free */
            if (PREV_HEADER(pHeader)->free)
            {
                /* Block is not considered free, the previous already is */
                pHeader->free = FALSE;

                /* Add this block to the previous block */
                pHeader = PREV_HEADER(pHeader);
                pHeader->nWords += nWords;
            }
            else
            {
                /* Mark block as free */
                pHeader->free = TRUE;

                /* Add block to the the free list */
                dllInsert(
                    &partId->freeList,
                    (DL_NODE *) NULL,
                    HEADER_TO_NODE(pHeader)
                    );
            }

            /* Get header to next block */
            pNextHeader = NEXT_HEADER(pHeader);

            /* If the next block is free */
            if (pNextHeader->free)
            {
                /* Add next block to this block */
                pHeader->nWords += pNextHeader->nWords;

                /* Remove next block from the free list */
                dllRemove(&partId->freeList, HEADER_TO_NODE(pNextHeader));

                /* This block has already been marked as free and added
                 * to the free list in the if-else block above this one.
                 */
            }

            /* Setup the next block previous header pointer */
            NEXT_HEADER(pHeader)->pPrevHeader = pHeader;

            /* Update partition statistics */
            partId->currBlocksAlloced--;
            partId->currWordsAlloced -= nWords;

            semGive(&partId->sem);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * memPartSemInit - Initialize memory partition semaphore
 *
 * RETURNS: N/A
 */

LOCAL void memPartSemInit(
    PART_ID partId
    )
{
    semBInit(&partId->sem, SEM_Q_PRIORITY, SEM_FULL);
}

