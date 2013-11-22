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

/* cacheArchLib.h - Architecture dependent cache library header */

#ifndef _cacheArchLib_h
#define _cacheArchLib_h

#include <types/vmxCpu.h>
#include <vmx.h>

#if      CPU_FAMILY==I386
#include <arch/i386/taskI386Lib.h>
#endif

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * cacheArchInit - Initalize architecture dependent cache library
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheArchLibInit(
    CACHE_MODE textMode,
    CACHE_MODE dataMode
    );

/******************************************************************************
 * cacheArchEnable - Enable cache
 *
 * RETURNS: OK
 */

STATUS cacheArchEnable(
    CACHE_TYPE cache
    );

/******************************************************************************
 * cacheArchDisable - Disable cache
 *
 * RETURNS: OK
 */

STATUS cacheArchDisable(
    CACHE_TYPE cache
    );

/******************************************************************************
 * cacheArchLock - Lock cache
 *
 * RETURNS: OK
 */

STATUS cacheArchLock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheArchUnlock - Unlock cache
 *
 * RETURNS: OK
 */

STATUS cacheArchUnlock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheArchClear - Clear cache
 *
 * RETURNS: OK
 */

STATUS cacheArchClear(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheArchFlush - Unlock cache
 *
 * RETURNS: OK
 */

STATUS cacheArchFlush(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheArchClearEntry - Clear cache entry
 *
 * RETURNS: OK
 */

STATUS cacheArchClearEntry(
    CACHE_TYPE  cache,
    void       *address
    );

/******************************************************************************
 * cacheArchDmaMalloc - Allocate dma memory
 *
 * RETURNS: Pointer to memory or NULL
 */

void* cacheArchDmaMalloc(
    size_t bytes
    );

/******************************************************************************
 * cacheArchDmaFree - Free allocated dma memory
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheArchDmaFree(
    void *buf
    );

/******************************************************************************
 * cacheArchDmaMallocSnoop - Allocate cache line aligen buffer
 *
 * RETURNS: Pointer to memory or NULL
 */

void* cacheArchDmaMallocSnoop(
    size_t bytes
    );

/******************************************************************************
 * cacheArchDmaFreeSnoop - Free cache line aligned allocated memory
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheArchDmaFreeSnoop(
    void *buf
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _cacheArchLib_h */

