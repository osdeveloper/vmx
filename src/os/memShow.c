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

/* memShow.c - Show memory partition info */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <vmx/classLib.h>
#include <vmx/taskLib.h>
#include <vmx/semLib.h>
#include <os/memPartLib.h>
#include <os/private/memPartLibP.h>
#include <os/memLib.h>
#include <os/memShow.h>

/* Locals */
LOCAL size_t memPartAvailable(
    PART_ID partId,
    size_t *largestBlock,
    BOOL printEach
    );

/******************************************************************************
 * memShowInit - Initialize memory show routines
 *
 * RETURNS: OK or ERROR
 */

STATUS memShowInit(
    void
    )
{
    return classShowConnect(memPartClassId, (FUNCPTR) memPartShow);
}

/******************************************************************************
 * memPartShow - Show memory partition statistics
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartShow(
    PART_ID partId,
    int type
    )
{
    STATUS status;
    unsigned numBlocks;
    unsigned totalBytes   = 0;
    unsigned largestWords = 0;

    /* Setup locals */
    totalBytes = 0;
    largestWords = 0;

    /* Verify object */
    if (OBJ_VERIFY(partId, memPartClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Print free list header if type is one */
        if (type == 1)
        {
            printf("\nFREE LIST:\n");
            printf("   num    addr       size\n");
            printf("  ---- ---------- ----------\n");
        }

        semTake(&partId->sem, WAIT_FOREVER);

        /* Get total bytes and print all blocks if requested */
        totalBytes = memPartAvailable(
                         partId,
                         &largestWords,
                         (type == 1) ? TRUE : FALSE
                         );
        if (totalBytes == ERROR)
        {
            semGive(&partId->sem);
            status = ERROR;
        }
        else
        {
            /* Convert to word size */
            largestWords /= 2;

            /* If type is one add space */
            if (type == 1)
            {
                printf("\n\n");
            }

            /* Get number if block in the free list */
            numBlocks = dllCount(&partId->freeList);

            /* If type is one add heading info */
            if (type == 1)
            {
                printf("SUMMARY:\n");
            }

            /* Print free block statistics */
            printf(" status    bytes     blocks   avg block  max block\n");
            printf(" ------ ---------- --------- ---------- ----------\n");
            printf("current\n");

            if (numBlocks != 0)
            {
                printf(
                    "   free %10u %9u %10u %10u\n",
                    totalBytes,
                    numBlocks,
                    totalBytes / numBlocks,
                    2 * largestWords
                    );
            }
            else
            {
                printf("   no free blocks\n");
            }

            /* Print total blocks statistics */
            if (partId->currBlocksAlloced > 0)
            {
                printf(
                    "  alloc %10u %9u %10u          -\n",
                    2 * partId->currWordsAlloced,
                    partId->currBlocksAlloced,
                    2 * partId->currWordsAlloced / partId->currBlocksAlloced
                    );
            }
            else
            {
                printf("   no allocated blocks\n");
            }

            printf("cumulative\n");

            /* Print cumulative blocks statistics */
            if (partId->cumBlocksAlloced > 0)
            {
                printf(
                    "  alloc %10u %9u %10u          -\n",
                    2 * partId->cumWordsAlloced,
                    partId->cumBlocksAlloced,
                    2 * partId->cumWordsAlloced / partId->cumBlocksAlloced
                    );
            }
            else
            {
                printf("   no allocated blocks\n");
            }

            semGive(&partId->sem);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * memPartAvailable - Get available memory in partition
 *
 * RETURNS: Number of bytes lefet in memory
 */

LOCAL size_t memPartAvailable(
    PART_ID partId,
    size_t *largestBlock,
    BOOL printEach
    )
{
    BLOCK_HEADER *pHeader;
    DL_NODE *pNode;
    size_t totalBytes   = 0;
    size_t largestWords = 0;
    int    i            = 0;

    /* Verify object */
    if (OBJ_VERIFY(partId, memPartClassId) != OK)
    {
        totalBytes = ERROR;
    }
    else
    {
        /* For all nodes in the free list */
        for (pNode = DLL_HEAD(&partId->freeList);
             pNode != NULL;
             pNode = DLL_NEXT(pNode))
        {
            /* Get block header from node */
            pHeader = NODE_TO_HEADER(pNode);

            /* If block is invalid */
            if (memPartBlockValidate(partId, pHeader, pHeader->free) != TRUE)
            {
                printf("  invalid block at %#x deleted\n", (unsigned) pHeader);

                /* Remove invalid block */
                dllRemove(&partId->freeList, HEADER_TO_NODE(pHeader));
                totalBytes = ERROR;
                break;
            }

            /* Add block size to total bytes count */
            totalBytes += 2 * pHeader->nWords;

            /* Check if this is the largest block */
            if (pHeader->nWords > largestWords)
            {
                largestWords = pHeader->nWords;
            }

            /* Print block info */
            if (printEach == TRUE)
            {
                printf(
                    "  %4d 0x%08x %10u\n",
                    i++,
                    (unsigned) pHeader,
                    (unsigned) 2 * pHeader->nWords
                    );
            }
        }

        /* Store largest block if requested */
        if ((totalBytes != ERROR) && (largestBlock != NULL))
        {
            *largestBlock = largestWords * 2;
        }
    }

    return totalBytes;
}

