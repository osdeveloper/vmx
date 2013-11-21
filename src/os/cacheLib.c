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

/* cacheLib.c - Cache library */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <os/cacheLib.h>

/* Defines */

/* Locals */

/* Globals */
BOOL cacheLibInstalled     = FALSE;
CACHE_MODE cacheDataMode   = CACHE_DISABLED;
BOOL cacheDataEnabled      = FALSE;
BOOL cacheMmuAvailable     = FALSE;
FUNCPTR cacheDmaMallocFunc = NULL;
FUNCPTR cacheDmaFreeFunc   = NULL;

/* Functions */

/******************************************************************************
 * cacheLibInit - Initalize cache library
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheLibInit(
    CACHE_MODE textMode,
    CACHE_MODE dataMode
    )
{
    STATUS status;

    /* If installed */
    if (cacheLibInstalled)
    {
        status = OK;
    }
    else
    {
        /* Install arch cache library */
        if (cacheArchLibInit(textMode, dataMode) != OK)
        {
            status = ERROR;
        }
        else
        {
            /* Mark as installed */
            cacheLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * cacheEnable - Enable cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheEnable(
    CACHE_TYPE cache
    )
{
    return CACHE_ENABLE(cache);
}

/******************************************************************************
 * cacheDisable - Disable cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheDisable(
    CACHE_TYPE cache
    )
{
    return CACHE_DISABLE(cache);
}

/******************************************************************************
 * cacheLock - Lock cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheLock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    return CACHE_LOCK(cache, addr, bytes);
}

/******************************************************************************
 * cacheUnlock - Unlock cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheUnlock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    return CACHE_UNLOCK(cache, addr, bytes);
}

/******************************************************************************
 * cacheFlush - Flush cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheFlush(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    return CACHE_FLUSH(cache, addr, bytes);
}

/******************************************************************************
 * cacheInvalidate - Invalidate cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheInvalidate(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    return CACHE_INVALIDATE(cache, addr, bytes);
}

/******************************************************************************
 * cacheClear - Clear cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheClear(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    )
{
    return CACHE_CLEAR(cache, addr, bytes);
}

/******************************************************************************
 * cacheTextUpdate - Text segment update cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheTextUpdate(
    void   *addr,
    size_t  bytes
    )
{
    return CACHE_TEXT_UPDATE(addr, bytes);
}

/******************************************************************************
 * cachePipeFlush - Flush pipe cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cachePipeFlush(
    void
    )
{
    return CACHE_PIPE_FLUSH();
}

/******************************************************************************
 * cacheDrvFlush - Flush cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDrvFlush(
    CACHE_FUNCS *pFunc,
    void        *addr,
    size_t       bytes
    )
{
    return CACHE_DRV_FLUSH(pFunc, addr, bytes);
}

/******************************************************************************
 * cacheDrvInvalidate - Invalidate cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDrvInvalidate(
    CACHE_FUNCS *pFunc,
    void *addr,
    size_t bytes
    )
{
    return CACHE_DRV_INVALIDATE(pFunc, addr, bytes);
}

/******************************************************************************
 * cacheDrvVirtyToPhys - Virtual to physical memory
 *
 * RETURNS: Physical address
 */

void* cacheDrvVirtToPhys(
    CACHE_FUNCS *pFunc,
    void        *addr
    )
{
    return CACHE_DRV_VIRT_TO_PHYS(pFunc, addr);
}

/******************************************************************************
 * cacheDrvPhysToVirt - Physical to virtual memory
 *
 * RETURNS: Virtual address
 */

void* cacheDrvPhysToVirt(
    CACHE_FUNCS *pFunc,
    void        *addr
    )
{
    return CACHE_DRV_PHYS_TO_VIRT(pFunc, addr);
}

/******************************************************************************
 * cacheDrvIsWriteCoherent - Get cache write coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDrvIsWriteCoherent(
    CACHE_FUNCS *pFunc
    )
{
    return CACHE_DRV_IS_WRITE_COHERENT(pFunc);
}

/******************************************************************************
 * cacheDrvIsReadCoherent - Get cache read coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDrvIsReadCoherent(
    CACHE_FUNCS *pFunc
    )
{
    return CACHE_DRV_IS_READ_COHERENT(pFunc);
}

/******************************************************************************
 * cacheDmaFlush - Flush dma cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDmaFlush(
    void   *addr,
    size_t  bytes
    )
{
    return CACHE_DMA_FLUSH(addr, bytes);
}

/******************************************************************************
 * cacheDmaInvalidate - Invalidate dma cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDmaInvalidate(
    void   *addr,
    size_t  bytes
    )
{
    return CACHE_DMA_INVALIDATE(addr, bytes);
}

/******************************************************************************
 * cacheDmaVirtyToPhys - Virtual to physical memory
 *
 * RETURNS: Physical address
 */

void* cacheDmaVirtToPhys(
    void *addr
    )
{
    return CACHE_DMA_VIRT_TO_PHYS(addr);
}

/******************************************************************************
 * cacheDmaPhysToVirt - Physical to virtual memory
 *
 * RETURNS: Virtual address
 */

void* cacheDmaPhysToVirt(
    void *addr
    )
{
    return CACHE_DMA_PHYS_TO_VIRT(addr);
}

/******************************************************************************
 * cacheDmaIsWriteCoherent - Get cache write coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDmaIsWriteCoherent(
    void
    )
{
    return CACHE_DMA_IS_WRITE_COHERENT();
}

/******************************************************************************
 * cacheDmaIsReadCoherent - Get cache read coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDmaIsReadCoherent(
    void
    )
{
    return CACHE_DMA_IS_READ_COHERENT();
}

/******************************************************************************
 * cacheUsrFlush - Flush user cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheUsrFlush(
    void   *addr,
    size_t  bytes
    )
{
    return CACHE_USR_FLUSH(addr, bytes);
}

/******************************************************************************
 * cacheUsrInvalidate - Invalidate user cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheUsrInvalidate(
    void   *addr,
    size_t  bytes
    )
{
    return CACHE_USR_INVALIDATE(addr, bytes);
}

/******************************************************************************
 * cacheUsrIsWriteCoherent - Get cache write coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheUsrIsWriteCoherent(
    void
    )
{
    return CACHE_USR_IS_WRITE_COHERENT();
}

/******************************************************************************
 * cacheUsrIsReadCoherent - Get cache read coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheUsrIsReadCoherent(
    void
    )
{
    return CACHE_USR_IS_READ_COHERENT();
}

/******************************************************************************
 * cacheFuncsSet - Set cache functions
 *
 * RETURNS: N/A
 */

void cacheFuncsSet(
    void
    )
{
    /* If no cache or fully coherent cache */
    if ((cacheDataEnabled == FALSE) || (cacheDataMode & CACHE_SNOOP_ENABLE))
    {
        cacheUsrFunc       = cacheNullFunc;
        cacheDmaFunc       = cacheNullFunc;
        cacheDmaMallocFunc = (FUNCPTR) NULL;
        cacheDmaFreeFunc   = (FUNCPTR) NULL;
    }
    else
    {
        cacheUsrFunc.invalidateFunc = cacheLib.invalidateFunc;
        cacheUsrFunc.flushFunc      = cacheLib.flushFunc;

        /* If mmu is avaliable */
        if (cacheMmuAvailable == TRUE)
        {
            cacheDmaFunc.flushFunc      = (FUNCPTR) NULL;
            cacheDmaFunc.invalidateFunc = (FUNCPTR) NULL;
            cacheDmaFunc.virtToPhysFunc = cacheLib.dmaVirtToPhysFunc;
            cacheDmaFunc.physToVirtFunc = cacheLib.dmaPhysToVirtFunc;
            cacheDmaMallocFunc          = cacheLib.dmaMallocFunc;
            cacheDmaFreeFunc            = cacheLib.dmaFreeFunc;
        }
        else
        {
            cacheDmaFunc       = cacheUsrFunc;
            cacheDmaMallocFunc = (FUNCPTR) NULL;
            cacheDmaFreeFunc   = (FUNCPTR) NULL;
        }
    }
}

