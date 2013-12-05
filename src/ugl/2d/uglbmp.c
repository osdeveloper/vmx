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

/* uglbmp.c - Universal graphics library bitmap support */

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/******************************************************************************
 *
 * uglBitmapCreate - Create bitmap
 *
 * RETURNS: Pointer to device dependent bitmap
 */

UGL_DDB_ID uglBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_COLOR            initValue,
    UGL_MEM_POOL_ID      poolId
    ) {
    UGL_DDB_ID  bmpId;

    /* Validate */
    if (devId == NULL) {
        return (UGL_NULL);
    }

    /* Lock device */
    if (uglOsLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Call driver specific method, should return UGL_NULL on failure */
    bmpId = (*devId->bitmapCreate) (devId, pDib, createMode,
                                    initValue, poolId);

    /* Unlock */
    uglOsUnLock (devId->lockId);

    return (bmpId);
}

/******************************************************************************
 *
 * uglBitmapDestroy - Free bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_DDB *        pDdb,
    UGL_MEM_POOL_ID  poolId
    ) {
    UGL_STATUS  status;
    UGL_DDB_ID  bmpId;

    /* Validate */
    if (devId == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOsLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Call driver specific method, should return UGL_ERROR on failure */
    status = (*devId->bitmapDestroy) (devId, pDdb, poolId);

    /* Unlock */
    uglOsUnLock (devId->lockId);

    return (status);
}

/******************************************************************************
 *
 * uglBitmapBlt - Block transfer for device independet bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBitmapBlt (
    UGL_GC_ID      gc,
    UGL_BITMAP_ID  srcBmpId,
    UGL_POS        srcLeft,
    UGL_POS        srcTop,
    UGL_POS        srcRight,
    UGL_POS        srcBottom,
    UGL_DDB_ID     destBmpId,
    UGL_POS        destX,
    UGL_POS        destY
    ) {
    UGL_STATUS     status;
    UGL_DEVICE_ID  devId;
    UGL_RECT       srcRect;
    UGL_RECT       destRect;

    /* Check if visible */
    if (srcTop > srcBottom || srcLeft > srcRight) {
        return (UGL_STATUS_OK);
    }

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId =  gc->pDriver;

    /* Setup source rectangle */
    srcRect.left   = srcLeft;
    srcRect.top    = srcTop;
    srcRect.right  = srcRight;
    srcRect.bottom = srcBottom;

    /* Setup destination rectangle */
    destRect.left   = destX;
    destRect.top    = destY;
    destRect.right  = destRect.left + UGL_RECT_WIDTH (srcRect) - 1;
    destRect.bottom = destRect.top + UGL_RECT_HEIGHT (srcRect) - 1;

    UGL_GC_SET (devId, gc);

    /* Call driver specific method */
    status = (*devId->bitmapBlt) (devId, srcBmpId, &srcRect,
                                  destBmpId, (UGL_POINT *) &destRect);

    return (status);
}

