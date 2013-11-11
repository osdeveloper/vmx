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

/* memLib.c - Memory library */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/semLib.h>
#include <os/classLib.h>
#include <os/memPartLib.h>
#include <os/private/memPartLibP.h>
#include <os/memLib.h>
#include <os/logLib.h>
#include <os/memLib.h>

/* Locals */
LOCAL BOOL      memLibInstalled = FALSE;
LOCAL char      memMsgBlockTooBig[] =
    "memPartAlloc: block too big - %d in partition.\n";
LOCAL char      memMsgBlockError[] =
    "%s: invalid block %#x in partition %#x.\n";

LOCAL void memPartAllocError(
    PART_ID partId,
    unsigned nBytes
    );

LOCAL void memPartBlockError(
    PART_ID partId,
    char *pBlock,
    char *label
    );

/******************************************************************************
 * memLibInit - Initialize memory library
 *
 * RETURNS: OK
 */

STATUS memLibInit(
    void
    )
{
    STATUS status;

    /* Check if already installed */
    if (memLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Setup */
        memPartAllocErrorFunc = (FUNCPTR) memPartAllocError;
        memPartBlockErrorFunc = (FUNCPTR) memPartBlockError;
        memPartOptionsDefault = MEM_ALLOC_ERROR_LOG_FLAG |
                                MEM_BLOCK_ERROR_LOG_FLAG |
                                MEM_ALLOC_ERROR_SUSPEND_FLAG |
                                MEM_BLOCK_CHECK;

        /* Mark as installed */
        memLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * memPartOptionsSet - Set memory options
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartOptionsSet(
    PART_ID partId,
    unsigned options
    )
{
    STATUS status;

    if (memLibInstalled != TRUE)
    {
        status = ERROR;
    }
    else
    {
        /* Verify object */
        if (OBJ_VERIFY(partId, memPartClassId) != OK)
        {
            status = ERROR;
        }
        else
        {
            semTake(&partId->sem, WAIT_FOREVER);

            /* Set options */
            partId->options |= options;

            /* If block check needed by option */
            if (options &
                (MEM_BLOCK_ERROR_LOG_FLAG | MEM_BLOCK_ERROR_SUSPEND_FLAG))
            {
                partId->options |= MEM_BLOCK_CHECK;
            }

            semGive(&partId->sem);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * memPartAllocError - Allocation error handler
 *
 * RETURNS: N/A
 */

LOCAL void memPartAllocError(
    PART_ID partId,
    unsigned nBytes
    )
{
    if (partId->options & MEM_ALLOC_ERROR_LOG_FLAG)
    {
        logMsg(
            memMsgBlockTooBig,
            (ARG) nBytes,
            (ARG) partId,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }
}

/******************************************************************************
 * memPartBlockError - Block error handler
 *
 * RETURNS: N/A
 */

LOCAL void memPartBlockError(
    PART_ID partId,
    char *pBlock,
    char *label
    )
{
    if (partId->options & MEM_BLOCK_ERROR_LOG_FLAG)
    {
        logMsg(
            memMsgBlockError,
            (ARG) label,
            (ARG) pBlock,
            (ARG) partId,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }
}

