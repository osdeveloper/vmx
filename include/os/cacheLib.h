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

/* cacheLib.h - Cache library header */

#ifndef _cacheLib_h
#define _cacheLib_h

/* Defines */

#include <ostool/moduleNumber.h>

#define S_cacheLib_INVALID_CACHE        (M_cacheLib | 0x0001)

/* Cache types */
#define _INSTRUCTION_CACHE      0       /* Instruction cache */
#define _DATA_CACHE             1       /* Data cache */
#define _BRANCH_CACE            2       /* Branch cache */

/* Cache bit masks */
#define CACHE_DISABLED          0x00    /* Cache disabled */
#define CACHE_WRITETROUGH       0x01    /* Cache write trough mode */
#define CAHE_COPYBACK           0x02    /* Cache copyback mode */
#define CACHE_WRITEALLOCATE     0x04    /* Cache write allocate mode */
#define CACHE_NO_WRITEALLOCATE  0x08    /* Cache no write allocate mode */
#define CACHE_SNOOP_ENABLE      0x10    /* Cache bus snooping mode */
#define CACHE_SNOOP_DISABLE     0x20    /* Cache no bus snooping mode */
#define CACHE_BURST_ENABLE      0x40    /* Cache burst cycles */
#define CACHE_BURST_DISABLE     0x80    /* Cache no burst cycles */

#ifndef _ASMLANGUAGE

#include <vmx.h>

/* Types */
typedef enum
{
    INSTRUCTION_CACHE = _INSTRUCTION_CACHE,
    DATA_CACHE        = _DATA_CACHE
} CACHE_TYPE;

typedef unsigned int CACHE_MODE;

typedef struct
{
    FUNCPTR enableFunc;
    FUNCPTR disableFunc;
    FUNCPTR lockFunc;
    FUNCPTR unlockFunc;
    FUNCPTR flushFunc;
    FUNCPTR invalidateFunc;
    FUNCPTR clearFunc;
    FUNCPTR textUpdateFunc;
    FUNCPTR pipeFlushFunc;
    FUNCPTR dmaMallocFunc;
    FUNCPTR dmaFreeFunc;
    FUNCPTR dmaVirtToPhysFunc;
    FUNCPTR dmaPhysToVirtFunc;
} CACHE_LIB;

typedef struct
{
    FUNCPTR flushFunc;
    FUNCPTR invalidateFunc;
    FUNCPTR virtToPhysFunc;
    FUNCPTR physToVirtFunc;
} CACHE_FUNCS;

/* Macros */

/******************************************************************************
 * CACHE_ENABLE - Enable cache
 *
 * RETURNS: OK or ERROR
 */

#define CACHE_ENABLE(cache)                                                   \
        ( (cacheLib.enableFunc == NULL) ? (ERROR) :                           \
          ( (*cacheLib.enableFunc) ((cache)) ) )

/******************************************************************************
 * CACHE_DISABLE - Disable cache
 *
 * RETURNS: OK or ERROR
 */

#define CACHE_DISABLE(cache)                                                  \
        ( (cacheLib.disableFunc == NULL) ? (ERROR) :                          \
          ( (*cacheLib.disableFunc) ((cache)) ) )

/******************************************************************************
 * CACHE_LOCK - Lock cache
 *
 * RETURNS: OK or ERROR
 */

#define CACHE_LOCK(cache, addr, bytes)                                        \
        ( (cacheLib.lockFunc == NULL) ? (ERROR) :                             \
          ( (*cacheLib.lockFunc) ((cache), (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_UNLOCK - Unlock cache
 *
 * RETURNS: OK or ERROR
 */

#define CACHE_UNLOCK(cache, addr, bytes)                                      \
        ( (cacheLib.unlockFunc == NULL) ? (ERROR) :                           \
          ( (*cacheLib.unlockFunc) ((cache), (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_FLUSH - Flush cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_FLUSH(cache, addr, bytes)                                       \
        ( (cacheLib.flushFunc == NULL) ? (OK) :                               \
          ( (*cacheLib.flushFunc) ((cache), (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_INVALIDATE - Invalidate cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_INVALIDATE(cache, addr, bytes)                                  \
        ( (cacheLib.invalidateFunc == NULL) ? (OK) :                          \
          ( (*cacheLib.invalidateFunc) ((cache), (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_CLEAR - Clear cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_CLEAR(cache, addr, bytes)                                       \
        ( (cacheLib.clearFunc == NULL) ? (OK) :                               \
          ( (*cacheLib.clearFunc) ((cache), (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_TEXT_UPDATE - Update cache text segment
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_TEXT_UPDATE(addr, bytes)                                        \
        ( (cacheLib.textUpdateFunc == NULL) ? (OK) :                          \
          ( (*cacheLib.textUpdateFunc) ((addr), (bytes)) ) )

/******************************************************************************
 * CACHE_PIPE_FLUSH - Flush cache pipe
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_PIPE_FLUSH()                                                    \
        ( (cacheLib.pipeFlushFunc == NULL) ? (OK) :                           \
          ( (*cacheLib.pipeFlushFunc) () ) )

/******************************************************************************
 * CACHE_DRV_FLUSH - Flush cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_DRV_FLUSH(pFunc, addr, bytes)                                   \
        ( ((pFunc)->flushFunc == NULL) ? (OK) :                               \
          ( (*(pFunc)->flushFunc) (DATA_CACHE, (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_DRV_INVALIDATE - Invalidate cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_DRV_INVALIDATE(pFunc, addr, bytes)                              \
        ( ((pFunc)->invalidateFunc == NULL) ? (OK) :                          \
          ( (*(pFunc)->invalidateFunc) (DATA_CACHE, (addr), (bytes)) ) )

/******************************************************************************
 * CACHE_DRV_VIRT_TO_PHYS - Virual to physical memory
 *
 * RETURNS: Physical address
 */

#define CACHE_DRV_VIRT_TO_PHYS(pFunc, addr)                                   \
        ( ((pFunc)->virtToPhysFunc == NULL) ? ((void *) addr) :               \
          ( (void *) (*(pFunc)->virtToPhysFunc) ((addr)) ) )

/******************************************************************************
 * CACHE_DRV_PHYS_TO_VIRT - Physical to virtual memory
 *
 * RETURNS: Virtual address
 */

#define CACHE_DRV_PHYS_TO_VIRT(pFunc, addr)                                   \
        ( ((pFunc)->physToVirtFunc == NULL) ? ((void *) addr) :               \
          ( (void *) (*(pFunc)->physToVirtFunc) ((addr)) ) )

/******************************************************************************
 * CACHE_DRV_IS_WRITE_COHERENT - Get write coherency
 *
 * RETURNS: TRUE or FALSE
 */

#define CACHE_DRV_IS_WRITE_COHERENT(pFunc)                                    \
        ( ((pFunc)->flushFunc == NULL) ? TRUE : FALSE )

/******************************************************************************
 * CACHE_DRV_IS_READ_COHERENT - Get read coherency
 *
 * RETURNS: TRUE or FALSE
 */

#define CACHE_DRV_IS_READ_COHERENT(pFunc)                                      \
        ( ((pFunc)->invalidateFunc == NULL) ? TRUE : FALSE )

/******************************************************************************
 * CACHE_DMA_FLUSH - Flush dma
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_DMA_FLUSH(addr, bytes)                                          \
        CACHE_DRV_FLUSH(&cacheDmaFunc, (addr), (bytes))

/******************************************************************************
 * CACHE_DMA_INVALIDATE - Invalidate dma
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_DMA_INVALIDATE(addr, bytes)                                     \
        CACHE_DRV_INVALIDATE(&cacheDmaFunc, (addr), (bytes))

/******************************************************************************
 * CACHE_DMA_VIRT_TO_PHYS - Virtual to physical memory for dma
 *
 * RETURNS: Pointer to physical address
 */

#define CACHE_DMA_VIRT_TO_PHYS(addr)                                          \
        CACHE_DRV_VIRT_TO_PHYS(&cacheDmaFunc, (addr))

/******************************************************************************
 * CACHE_DMA_PHYS_TO_VIRT - Physical to virtual memory for dma
 *
 * RETURNS: Pointer to virtual address
 */

#define CACHE_DMA_PHYS_TO_VIRT(addr)                                          \
        CACHE_DRV_PHYS_TO_VIRT(&cacheDmaFunc, (addr))

/******************************************************************************
 * CACHE_DMA_IS_WRITE_COHERENT - Get write coherency
 *
 * RETURNS: TRUE or FALSE
 */

#define CACHE_DMA_IS_WRITE_COHERENT()                                         \
        CACHE_DRV_IS_WRITE_COHERENT(&cacheDmaFunc)

/******************************************************************************
 * CACHE_DMA_IS_READ_COHERENT - Get read coherency
 *
 * RETURNS: TRUE or FALSE
 */

#define CACHE_DMA_IS_READ_COHERENT()                                          \
        CACHE_DRV_IS_READ_COHERENT(&cacheDmaFunc)

/******************************************************************************
 * CACHE_USR_FLUSH - Flush user cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_USR_FLUSH(addr, bytes)                                          \
        CACHE_DRV_FLUSH(&cacheUsrFunc, (addr), (bytes))

/******************************************************************************
 * CACHE_USR_INVALIDATE - Invalidate user cache
 *
 * RETURNS: OK or driver specific
 */

#define CACHE_USR_INVALIDATE(addr, bytes)                                     \
        CACHE_DRV_INVALIDATE(&cacheUsrFunc, (addr), (bytes))

/******************************************************************************
 * CACHE_USR_IS_WRITE_COHERENT - Get write coherency
 *
 * RETURNS: TRUE or FALSE
 */

#define CACHE_USR_IS_WRITE_COHERENT()                                         \
        CACHE_DRV_IS_WRITE_COHERENT(&cacheUsrFunc)

/******************************************************************************
 * CACHE_USR_IS_READ_COHERENT - Get read coherency
 *
 * RETURNS: TRUE or FALSE
 */

#define CACHE_USR_IS_READ_COHERENT()                                          \
        CACHE_DRV_IS_READ_COHERENT(&cacheUsrFunc)

/* Imports */
IMPORT CACHE_LIB   cacheLib;
IMPORT CACHE_FUNCS cacheNullFunc;
IMPORT CACHE_FUNCS cacheDmaFunc;
IMPORT CACHE_FUNCS cacheUsrFunc;
IMPORT CACHE_MODE  cacheDataMode;
IMPORT BOOL        cacheDataEnabled;
IMPORT BOOL        cacheMmuAvailable;
IMPORT FUNCPTR     dmaMallocFunc;
IMPORT FUNCPTR     dmaFreeFunc;

/* Functions */

/******************************************************************************
 * cacheLibInit - Initalize cache library
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheLibInit(
    CACHE_MODE textMode,
    CACHE_MODE dataMode
    );

/******************************************************************************
 * cacheEnable - Enable cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheEnable(
    CACHE_TYPE cache
    );

/******************************************************************************
 * cacheDisable - Disable cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheDisable(
    CACHE_TYPE cache
    );

/******************************************************************************
 * cacheLock - Lock cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheLock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheUnlock - Unlock cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheUnlock(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheFlush - Flush cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheFlush(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheInvalidate - Invalidate cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheInvalidate(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheClear - Clear cache
 *
 * RETURNS: OK or ERROR
 */

STATUS cacheClear(
    CACHE_TYPE  cache,
    void       *addr,
    size_t      bytes
    );

/******************************************************************************
 * cacheTextUpdate - Text segment update cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheTextUpdate(
    void   *addr,
    size_t  bytes
    );

/******************************************************************************
 * cachePipeFlush - Flush pipe cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cachePipeFlush(
    void
    );

/******************************************************************************
 * cacheDrvFlush - Flush cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDrvFlush(
    CACHE_FUNCS *pFunc,
    void        *addr,
    size_t       bytes
    );

/******************************************************************************
 * cacheDrvInvalidate - Invalidate cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDrvInvalidate(
    CACHE_FUNCS *pFunc,
    void *addr,
    size_t bytes
    );

/******************************************************************************
 * cacheDrvVirtyToPhys - Virtual to physical memory
 * 
 * RETURNS: Physical address
 */
 
void* cacheDrvVirtToPhys(
    CACHE_FUNCS *pFunc,
    void        *addr
    );

/******************************************************************************
 * cacheDrvPhysToVirt - Physical to virtual memory
 *
 * RETURNS: Virtual address
 */

void* cacheDrvPhysToVirt(
    CACHE_FUNCS *pFunc,
    void        *addr
    );

/******************************************************************************
 * cacheDrvIsWriteCoherent - Get cache write coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDrvIsWriteCoherent(
    CACHE_FUNCS *pFunc
    );

/******************************************************************************
 * cacheDrvIsReadCoherent - Get cache read coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDrvIsReadCoherent(
    CACHE_FUNCS *pFunc
    );

/******************************************************************************
 * cacheDmaFlush - Flush dma cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDmaFlush(
    void   *addr,
    size_t  bytes
    );

/******************************************************************************
 * cacheDmaInvalidate - Invalidate dma cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheDmaInvalidate(
    void   *addr,
    size_t  bytes
    );

/******************************************************************************
 * cacheDmaVirtyToPhys - Virtual to physical memory
 *
 * RETURNS: Physical address
 */

void* cacheDmaVirtToPhys(
    void *addr
    );

/******************************************************************************
 * cacheDmaPhysToVirt - Physical to virtual memory
 *
 * RETURNS: Virtual address
 */

void* cacheDmaPhysToVirt(
    void *addr
    );

/******************************************************************************
 * cacheDmaIsWriteCoherent - Get cache write coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDmaIsWriteCoherent(
    void
    );

/******************************************************************************
 * cacheDmaIsReadCoherent - Get cache read coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheDmaIsReadCoherent(
    void
    );

/******************************************************************************
 * cacheUsrFlush - Flush user cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheUsrFlush(
    void   *addr,
    size_t  bytes
    );

/******************************************************************************
 * cacheUsrInvalidate - Invalidate user cache
 *
 * RETURNS: OK or driver specific
 */

STATUS cacheUsrInvalidate(
    void   *addr,
    size_t  bytes
    );

/******************************************************************************
 * cacheUsrIsWriteCoherent - Get cache write coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheUsrIsWriteCoherent(
    void
    );

/******************************************************************************
 * cacheUsrIsReadCoherent - Get cache read coherency
 *
 * RETURNS: TRUE or FALSE
 */

BOOL cacheUsrIsReadCoherent(
    void
    );

/******************************************************************************
 * cacheFuncsSet - Set cache functions
 *
 * RETURNS: N/A
 */

void cacheFuncsSet(
    void
    );

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _cacheLib_h */

