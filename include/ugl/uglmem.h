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

/* uglmem.h - Universal graphics library memory pool management support */

#ifndef _uglmem_h
#define _uglmem_h

#ifndef _ASMLANGUAGE

#include "ugltypes.h"
#include "uglos.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

#define UGL_DEFAULT_MEM_POOL_ID         ((UGL_MEM_POOL_ID) UGL_NULL)

/******************************************************************************
 *
 * UGL_MALLOC - Allocate memory from default pool
 *
 * RETURNS: Pointer to memory, or UGL_NULL
 */

#define UGL_MALLOC(s)       uglOSMemAlloc (UGL_DEFAULT_MEM_POOL_ID, (s))

/******************************************************************************
 *
 * UGL_CALLOC - Allocate memory objects from default pool and clear
 *
 * RETURNS: Pointer to memory, or UGL_NULL
 */

#define UGL_CALLOC(n, s)    uglOSMemCalloc (UGL_DEFAULT_MEM_POOL_ID, (n), (s))

/******************************************************************************
 *
 * UGL_REALLOC - Change size of memory allocated from default pool
 *
 * RETURNS: Pointer to memory, or UGL_NULL
 */

#define UGL_REALLOC(m, s)   uglOSMemRealloc (UGL_DEFAULT_MEM_POOL_ID, (m), (s))

/******************************************************************************
 *
 * UGL_FREE - Free memory from default pool
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

#define UGL_FREE(pMem)      uglOSMemFree ((pMem))

/* Functions */

/******************************************************************************
 *
 * uglMemDefaultPoolGet - Get default memory pool
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglMemDefaultPoolGet (
    UGL_MEM_POOL_ID * pPoolId
    );

/******************************************************************************
 *
 * uglMemDefaultPoolSet - Set default memory pool
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglMemDefaultPoolSet (
    UGL_MEM_POOL_ID  poolId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglmem_h */

