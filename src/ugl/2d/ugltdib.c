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

/* ugltdib.c - Universal graphics library transparent bitmap support */

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/******************************************************************************
 *
 * uglTransBitmapCreate - Create transparent bitmap
 *
 * RETURNS: Pointer to device dependent transparent bitmap
 */

UGL_DDB_ID uglTransBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_MDIB *           pMdib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_COLOR            initValue,
    UGL_MEM_POOL_ID      poolId
    ) {
    UGL_TDDB_ID  bmpId;

    /* Validate */
    if (devId == NULL) {
        return (UGL_NULL);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Call driver specific method, should return UGL_NULL on failure */
    bmpId = (*devId->transBitmapCreate) (devId, pDib, pMdib, createMode,
                                         initValue, poolId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (bmpId);
}

/******************************************************************************
 *
 * uglTransBitmapDestroy - Free transparent bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglTransBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_TDDB *       pTddb
    ) {
    UGL_STATUS   status;
    UGL_TDDB_ID  bmpId;

    /* Validate */
    if (devId == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Call driver specific method, should return UGL_ERROR on failure */
    status = (*devId->transBitmapDestroy) (devId, pTddb);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

