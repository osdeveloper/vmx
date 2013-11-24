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

/* vmShow.c - Virtual memory show facilities */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/mmuArchLib.h>
#include <vmx/semLib.h>
#include <os/classLib.h>
#include <os/objLib.h>
#include <os/vmLib.h>
#include <os/vmShow.h>

/* Macros */

#define IS_IN_GLOBAL_SPACE(table, vAddr) \
    ((table)[(unsigned) vAddr / mmuPageBlockSize])

LOCAL void vmContextBlockShow(
    void     *blockStart,
    void     *blockEnd,
    void     *physAddr,
    unsigned  blockState
    );

/******************************************************************************
 * vmShowInit - Initialize virtual memory show facilities
 *
 * RETURNS: N/A
 */

void vmShowInit(
    void
    )
{
    classShowConnect(vmContextClassId, (FUNCPTR) vmContextShow);
}

/******************************************************************************
 * vmContextShow - Display virtual memory translation table for context
 *
 * RETURNS: OK or ERROR
 */

STATUS vmContextShow(
    VM_CONTEXT_ID context
    )
{
    STATUS    status;
    unsigned  state;
    unsigned  blockState;
    void     *blockStart;
    void     *physAddr;
    int       pageSize;
    int       numPages;
    int       pageCount       = 0;
    BOOL      endBlock        = FALSE;
    void     *blockPhysAddr   = NULL;
    BOOL      prevPageInvalid = TRUE;
    BOOL      doPrint         = TRUE;
    char     *virtPage        = NULL;
    char     *physPage        = NULL;

    pageSize = vmPageSizeGet();
    if (pageSize == 0)
    {
        status = ERROR;
    }
    else
    {
        if (context == NULL)
        {
            context = vmCurrentGet();
        }

        if (OBJ_VERIFY(context, vmContextClassId) != OK)
        {
            status = ERROR;
        }
        else
        {
            status = OK;

            printf ("VIRTUAL ADDR  BLOCK LENGTH  PHYSICAL ADDR   STATE\n");

            semTake(&context->sem, WAIT_FOREVER);

            vmStateGet(context, NULL, &blockState);

            numPages = (unsigned) 0x80000000 / pageSize * 2;
            for (blockStart = NULL;
                 pageCount < numPages;
                 pageCount++, virtPage += pageSize, physPage += pageSize)
            {
                if (vmTranslate(context, virtPage, &physAddr) != OK)
                {
                    endBlock = TRUE;

                    if (prevPageInvalid == TRUE)
                    {
                        doPrint = FALSE;
                    }

                    prevPageInvalid = TRUE;
                    physPage        = (char *) 0xffffffff;
                }
                else
                {
                    if ((prevPageInvalid == FALSE) && (physAddr != physPage))
                    {
                        endBlock = TRUE;
                    }

                    if (vmStateGet(context, virtPage, &state) != OK)
                    {
                        fprintf(
                            stderr,
                            "vmContextShow: error getting state for addr %x\n",
                            (unsigned) virtPage
                            );
                        status = ERROR;
                        break;
                    }

                    if ((prevPageInvalid == FALSE) && (state != blockState))
                    {
                        endBlock = TRUE;
                    }

                    if (prevPageInvalid == TRUE)
                    {
                        blockStart    = virtPage;
                        blockState    = state;
                        blockPhysAddr = physAddr;
                        physPage      = physAddr;
                    }

                    prevPageInvalid = FALSE;;
                }

                if (endBlock == TRUE)
                {
                    if (doPrint == TRUE)
                    {
                        vmContextBlockShow(
                            blockStart,
                            virtPage,
                            blockPhysAddr,
                            blockState
                            );
                    }

                    blockStart    = virtPage;
                    blockState    = state;
                    blockPhysAddr = physAddr;
                    physPage      = physAddr;
                    endBlock      = FALSE;
                    doPrint       = TRUE;
                }
            }

            if ((status == OK) && (prevPageInvalid == FALSE))
            {
                vmContextBlockShow(
                    blockStart,
                    virtPage,
                    blockPhysAddr,
                    blockState
                    );
            }

            semGive(&context->sem);
        }
    }

    return status;
}

/******************************************************************************
 * vmContextBlockShow - Display virtual memory block
 *
 * RETURNS: N/A
 */

LOCAL void vmContextBlockShow(
    void     *blockStart,
    void     *blockEnd,
    void     *physAddr,
    unsigned  blockState
    )
{
    char *info = vmGlobalInfoGet();

    printf(
        "0x%08x    0x%08x    0x%08x      ",
        (unsigned) blockStart,
        (unsigned) blockEnd - (unsigned) blockStart,
        (unsigned) physAddr
        );

    if (blockState & VM_STATE_WRITABLE)
    {
        printf("W+ ");
    }
    else
    {
        printf("W- ");
    }

    if (blockState & VM_STATE_MASK_CACHEABLE)
    {
        printf("C+ ");
    }
    else
    {
        printf("C- ");
    }

#if (CPU_FAMILY == I386)
    if ((blockState & VM_STATE_MASK_WBACK) == VM_STATE_WBACK)
    {
        printf("B+ ");
    }
    else
    {
        printf("B- ");
    }

    if ((blockState & VM_STATE_MASK_GLOBAL) == VM_STATE_GLOBAL)
    {
        printf("G+ ");
    }
    else
    {
        printf("G- ");
    }
#endif /* CPU_FAMILY == I386 */

    if (IS_IN_GLOBAL_SPACE(info, blockStart))
    {
        printf(" (global)");
    }

    printf("\n");
}

