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

/* uglmem.c - Universal graphics library memory pool management support */

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/* Locals */

UGL_LOCAL UGL_MEM_POOL_ID uglMemPoolDefaultId = UGL_NULL;

/******************************************************************************
 *
 * uglMemDefaultPoolGet - Get default memory pool
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglMemDefaultPoolGet (
    UGL_MEM_POOL_ID * pPoolId
    ) {

    /* Check args */
    if (pPoolId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get memory pool */
    *pPoolId = uglMemPoolDefaultId;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglMemDefaultPoolSet - Set default memory pool
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglMemDefaultPoolSet (
    UGL_MEM_POOL_ID  poolId
    ) {

    uglMemPoolDefaultId = poolId;

    return (UGL_STATUS_OK);
}

