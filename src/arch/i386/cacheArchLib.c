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

/* cacheArchLib.c - Architecture dependent cache library */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <arch/regs.h>
#include <os/memPartLib.h>
#include <os/vmLib.h>
#include <os/cacheLib.h>
#include <arch/cacheArchLib.h>

/* Defines */

/* Locals */
LOCAL BOOL cacheArchLibInstalled = FALSE;

/* Globals */

/* Functions */

/******************************************************************************
 * cacheArchInit - Initalize architecture dependent cache library
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheArchLibInit(
    CACHE_MODE textMode,
    CACHE_MODE dataMode
    )
{
    STATUS status;

    /* Check if already installed */
    if (cacheArchLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Install cache functions */
        cacheLib.enableFunc  = (FUNCPTR) cacheArchEnable;
        cacheLib.disableFunc = (FUNCPTR) cacheArchDisable;

        /* Install text update function */
        cacheLib.textUpdateFunc = (FUNCPTR) NULL;
        cacheLib.pipeFlushFunc  = (FUNCPTR) NULL;

        /* Install dma functions */
        cacheLib.dmaVirtToPhysFunc = (FUNCPTR) NULL;
        cacheLib.dmaPhysToVirtFunc = (FUNCPTR) NULL;

        /* If snoop enabled */
        if (dataMode & CACHE_SNOOP_ENABLE)
        {
            /* Install cache functions */
            cacheLib.lockFunc       = (FUNCPTR) NULL;
            cacheLib.unlockFunc     = (FUNCPTR) NULL;
            cacheLib.clearFunc      = (FUNCPTR) NULL;
            cacheLib.flushFunc      = (FUNCPTR) NULL;
            cacheLib.invalidateFunc = (FUNCPTR) NULL;

            /* Install dma functions */
            cacheLib.dmaMallocFunc = (FUNCPTR) cacheArchDmaMallocSnoop;
            cacheLib.dmaFreeFunc   = (FUNCPTR) cacheArchDmaFreeSnoop;
        }
        else
        {
            /* Install cache functions */
            cacheLib.lockFunc       = (FUNCPTR) cacheArchLock;
            cacheLib.unlockFunc     = (FUNCPTR) cacheArchUnlock;
            cacheLib.clearFunc      = (FUNCPTR) cacheArchClear;
            cacheLib.flushFunc      = (FUNCPTR) cacheArchFlush;
            cacheLib.invalidateFunc = (FUNCPTR) cacheArchClear;

            /* Install dma functions */
            cacheLib.dmaMallocFunc = (FUNCPTR) cacheArchDmaMalloc;
            cacheLib.dmaFreeFunc   = (FUNCPTR) cacheArchDmaFree;
        }

        /* If error in parameters */
        if ((textMode & CACHE_WRITEALLOCATE) ||
            (dataMode & CACHE_WRITEALLOCATE) ||
            (textMode & CACHE_NO_WRITEALLOCATE) ||
            (dataMode & CACHE_NO_WRITEALLOCATE) ||
            (textMode & CACHE_BURST_ENABLE) ||
            (dataMode & CACHE_BURST_ENABLE) ||
            (textMode & CACHE_BURST_DISABLE) ||
            (dataMode & CACHE_BURST_DISABLE))
        {
            status = ERROR;
        }
        else
        {
            /* Reset cache */
            cacheI386Reset();

            /* Initailize globals */
            cacheDataMode     = dataMode;
            cacheDataEnabled  = FALSE;
            cacheMmuAvailable = FALSE;

            /* Mark as installed */
            cacheArchLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * cacheArchEnable - Enable cache
 *
 * RETURNS: OK
 */

STATUS cacheArchEnable(
    CACHE_TYPE cache
    )
{
    /* Call low-level cache enable */
    cacheI386Enable();

    /* If data cache */
    if (cache == DATA_CACHE)
    {
        /* Enable data cache */
        cacheDataEnabled = TRUE;
        cacheFuncsSet();
    }

    return OK;
}

/******************************************************************************
 * cacheArchDisable - Disable cache
 *
 * RETURNS: OK
 */

STATUS cacheArchDisable(
    CACHE_TYPE cache
    )
{
    /* Call low-level cache disable */
    cacheI386Disable();

    /* If data cache */
    if (cache == DATA_CACHE)
    {
        /* Enable data cache */
        cacheDataEnabled = FALSE;
        cacheFuncsSet();
    }

    return OK;
}

/******************************************************************************
 * cacheArchLock - Lock cache
 *
 * RETURNS: OK
 */

STATUS cacheArchLock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    /* Call low-level cache lock */
    cacheI386Lock();

    return OK;
}

/******************************************************************************
 * cacheArchUnlock - Unlock cache
 *
 * RETURNS: OK
 */

STATUS cacheArchUnlock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    /* Call low-level cache unlock */
    cacheI386Unlock();

    return OK;
}

/******************************************************************************
 * cacheArchClear - Clear cache
 *
 * RETURNS: OK
 */

STATUS cacheArchClear(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    /* Call low-level cache clear */
    cacheI386Clear();

    return OK;
}

/******************************************************************************
 * cacheArchFlush - Unlock cache
 *
 * RETURNS: OK
 */

STATUS cacheArchFlush(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    /* Call low-level cache flush */
    cacheI386Flush();

    return OK;
}

/******************************************************************************
 * cacheArchClearEntry - Clear cache entry
 *
 * RETURNS: OK
 */

STATUS cacheArchClearEntry(
    CACHE_TYPE  cache,
    void       *address
    )
{
#if (CPU != I80386)
    __asm__ __volatile__("wbinvd");
#endif

    return OK;
}

/******************************************************************************
 * cacheArchDmaMalloc - Allocate dma memory
 *
 * RETURNS: Pointer to memory or NULL
 */

void* cacheArchDmaMalloc(
    size_t bytes
    )
{
    void *buf;
    int   pageSize;

    /* Get page size */
    pageSize = vmPageSizeGet();
    if (pageSize == ERROR)
    {
        errnoSet(S_vmLib_NOT_INSTALLED);
        buf = NULL;
    }
    else
    {
        /* Round up memory to page size */
        bytes = ROUND_UP(bytes, pageSize);

        /* Allocate memory */
        buf = valloc(bytes);
        if (buf != NULL)
        {
            vmStateSet(
                NULL,
                buf,
                bytes,
                VM_STATE_MASK_CACHEABLE,
                VM_STATE_NOT_CACHEABLE
                );
        }
    }

    return buf;
}

/******************************************************************************
 * cacheArchDmaFree - Free allocated dma memory
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheArchDmaFree(
    void *buf
    )
{
    STATUS        status;
    BLOCK_HEADER *pHeader;

    pHeader = BLOCK_TO_HEADER((char *) buf);
    vmStateSet(
        NULL,
        buf,
        (pHeader->nWords * 2) - sizeof(BLOCK_HEADER),
        VM_STATE_MASK_CACHEABLE,
        VM_STATE_CACHEABLE
        );

    /* Free memory */
    free(buf);

    return OK;
}

/******************************************************************************
 * cacheArchDmaMallocSnoop - Allocate cache line aligen buffer
 *
 * RETURNS: Pointer to memory or NULL
 */

void* cacheArchDmaMallocSnoop(
    size_t bytes
    )
{
    void *buf;

    /* Align size */
    bytes = ROUND_UP(bytes, _CACHE_ALIGN_SIZE);

    /* Allocate memory */
    buf = memalign(_CACHE_ALIGN_SIZE, bytes);

    return buf;
}

/******************************************************************************
 * cacheArchDmaFreeSnoop - Free cache line aligned allocated memory
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheArchDmaFreeSnoop(
    void *buf
    )
{
    free(buf);

    return OK;
}

