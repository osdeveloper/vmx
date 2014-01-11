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

/* ugldib.c - Universal graphics library bitmap support */

#include "ugl.h"
#include "driver/graphics/generic/udgen.h"

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
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Call driver specific method, should return UGL_NULL on failure */
    bmpId = (*devId->bitmapCreate) (devId, pDib, createMode,
                                    initValue, poolId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

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
    UGL_DDB *        pDdb
    ) {
    UGL_STATUS  status;

    /* Validate */
    if (devId == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Call driver specific method, should return UGL_ERROR on failure */
    status = (*devId->bitmapDestroy) (devId, pDdb);

    /* Unlock */
    uglOSUnlock (devId->lockId);

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
    UGL_RECT       vpRect;

    /* Check if visible */
    if (srcTop > srcBottom || srcLeft > srcRight) {
        return (UGL_STATUS_OK);
    }

    /* Start batch job */
    if (uglBatchStart (gc) == UGL_STATUS_ERROR) {
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

    /* Hide cursor during drawing */
    if (devId->cursorHide != UGL_NULL) {
        if (srcBmpId == UGL_DISPLAY_ID) {
            (*devId->cursorHide) (devId, &srcRect);
        }
        else if (srcBmpId == UGL_DEFAULT_ID &&
                 gc->pDefaultBitmap == UGL_DISPLAY_ID) {
            UGL_RECT_COPY (&vpRect, &srcRect);
            vpRect.left   += gc->viewPort.left;
            vpRect.top    += gc->viewPort.top;
            vpRect.right  += gc->viewPort.left;
            vpRect.bottom += gc->viewPort.bottom;
            (*devId->cursorHide) (devId, &vpRect);
        }

        if (destBmpId == UGL_DISPLAY_ID) {
            (*devId->cursorHide) (devId, &destRect);
        }
    }

    if ((destBmpId == UGL_DEFAULT_ID) || (destBmpId == UGL_DISPLAY_ID) ||
        (destBmpId->type == UGL_DDB_TYPE)) {

        if ((srcBmpId == UGL_DEFAULT_ID) || (srcBmpId == UGL_DISPLAY_ID) ||
            (srcBmpId->type == UGL_DDB_TYPE)) {

            /* Call driver specific method */
            status = (*devId->bitmapBlt) (devId, srcBmpId, &srcRect,
                                          destBmpId, (UGL_POINT *) &destRect);
        }
        else if (srcBmpId->type == UGL_MDDB_TYPE) {

            /* Call driver specific method */
            status = (*devId->monoBitmapBlt) (devId, srcBmpId, &srcRect,
                                              destBmpId,
                                              (UGL_POINT *) &destRect);
        }
        else if (srcBmpId->type == UGL_TDDB_TYPE) {
            /* Call driver specific method */
            status = (*devId->transBitmapBlt) (devId, srcBmpId, &srcRect,
                                               destBmpId,
                                               (UGL_POINT *) &destRect);
        }
        else {
            status = UGL_STATUS_ERROR;
        }
    }
    else {
        status = UGL_STATUS_ERROR;
    }

    /* End batch job */
    uglBatchEnd (gc);

    return (status);
}

/******************************************************************************
 *
 * uglBitmapWrite - Write to device dependet bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_DIB *      pDib,
    UGL_POS        srcLeft,
    UGL_POS        srcTop,
    UGL_POS        srcRight,
    UGL_POS        srcBottom,
    UGL_DDB_ID     destBmpId,
    UGL_POS        destX,
    UGL_POS        destY
    ) {
    UGL_STATUS  status;
    UGL_RECT    srcRect;
    UGL_POINT   destPoint;
    UGL_RECT    cursorRect;

    /* Check params */
    if (pDib == UGL_NULL || destBmpId == UGL_NULL ||
        (destBmpId != UGL_DISPLAY_ID && destBmpId->type != UGL_DDB_TYPE)) {
        return (UGL_STATUS_ERROR);
    }

    /* Check if trivial */
    if (srcLeft > srcRight || srcTop > srcBottom) {
        return (UGL_STATUS_OK);
    }

    /* Validate */
    if (devId == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Setup geometry */
    srcRect.left   = srcLeft;
    srcRect.top    = srcTop;
    srcRect.right  = srcRight;
    srcRect.bottom = srcBottom;

    destPoint.x = destX;
    destPoint.y = destY;

    /* Hide cursor */
    if (devId->cursorHide != UGL_NULL && destBmpId == UGL_DISPLAY_ID) {
        cursorRect.left   = destPoint.x;
        cursorRect.top    = destPoint.y;
        cursorRect.top    = cursorRect.left + UGL_RECT_WIDTH (srcRect) - 1;
        cursorRect.bottom = cursorRect.top + UGL_RECT_HEIGHT (srcRect) - 1;
        (*devId->cursorHide) (devId, &cursorRect);
    }

    /* Call driver specific method, should return UGL_NULL on failure */
    status = (*devId->bitmapWrite) (devId, pDib, &srcRect,
                                    destBmpId, &destPoint);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

/******************************************************************************
 *
 * uglBitmapRead - Read from device dependet bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBitmapRead (
    UGL_DEVICE_ID  devId,
    UGL_DDB_ID     srcBmpId,
    UGL_POS        srcLeft,
    UGL_POS        srcTop,
    UGL_POS        srcRight,
    UGL_POS        srcBottom,
    UGL_DIB *      pDib,
    UGL_POS        destX,
    UGL_POS        destY
    ) {
    UGL_STATUS  status;
    UGL_RECT    srcRect;
    UGL_POINT   destPoint;

    /* Check params */
    if (pDib == UGL_NULL || srcBmpId == UGL_NULL ||
        (srcBmpId != UGL_DISPLAY_ID && srcBmpId->type != UGL_DDB_TYPE)) {
        return (UGL_STATUS_ERROR);
    }

    /* Check if trivial */
    if ((pDib->width == 0) || (pDib->height == 0)) {
        return (UGL_STATUS_OK);
    }

    /* Validate */
    if (devId == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Setup geometry */
    srcRect.left   = srcLeft;
    srcRect.top    = srcTop;
    srcRect.right  = srcRight;
    srcRect.bottom = srcBottom;

    destPoint.x = destX;
    destPoint.y = destY;

    /* Hide cursor */
    if (devId->cursorHide != UGL_NULL && srcBmpId == UGL_DISPLAY_ID) {
        (*devId->cursorHide) (devId, &srcRect);
    }

    /* Call driver specific method, should return UGL_NULL on failure */
    status = (*devId->bitmapRead) (devId, srcBmpId, &srcRect,
                                   pDib, &destPoint);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

