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

/* udgtbmp.c - Universal graphics library generic transparent bitmap */

#include <stdlib.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/* Locals */

UGL_LOCAL UGL_STATUS uglGenericScratchBitmapCreate (
    UGL_DEVICE_ID    devId,
    UGL_SIZE         width,
    UGL_SIZE         height,
    UGL_MEM_POOL_ID  poolId
    );

UGL_LOCAL UGL_STATUS uglGenericScratchBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_MEM_POOL_ID  poolId
    );

/******************************************************************************
 *
 * uglGenericTransBitmapCreate - Create generic transparent bitmap
 *
 * RETURNS: UGL_TDDB_ID or UGL_NULL
 */

UGL_TDDB_ID uglGenericTransBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_MDIB *           pMdib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_COLOR            initValue,
    UGL_MEM_POOL_ID      poolId
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GEN_TDDB *       pTddb;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Create scratch bitmap */
    if (uglGenericScratchBitmapCreate (devId, pDib->width, pDib->height,
                                       poolId) == UGL_STATUS_ERROR) {
        return (UGL_NULL);
    }

    /* Allocate transparent bitmap */
    pTddb = (UGL_GEN_TDDB *) UGL_PART_MALLOC (poolId, sizeof (UGL_GEN_TDDB));
    if (pTddb != UGL_NULL) {
        pTddb->header.width  = pDib->width;
        pTddb->header.height = pDib->height;
        pTddb->header.type   = UGL_TDDB_TYPE;

        /* Create bitmap */
        pTddb->ddb = (*devId->bitmapCreate) (devId, pDib, createMode, initValue,
                                             poolId);
        if (pTddb->ddb == UGL_NULL) {
            UGL_PART_FREE (poolId, pTddb);
            return (UGL_NULL);
        }

        /* Create bitmask */
        pTddb->mask = (*devId->monoBitmapCreate) (devId, pMdib, createMode,
                                            0xff, poolId);
        if (pTddb->mask == UGL_NULL) {
            (*devId->bitmapDestroy) (devId, (UGL_DDB_ID) pTddb->ddb, poolId);
            UGL_PART_FREE (poolId, pTddb);
            return (UGL_NULL);
        }
    }

    return (UGL_TDDB_ID) pTddb;
}

/******************************************************************************
 *
 * uglGenericTransBitmapDestroy - Destroy generic transparent bitmap
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglGenericTransBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_TDDB_ID      tDdbId,
    UGL_MEM_POOL_ID  poolId
    ) {
    UGL_GEN_TDDB * pTddb;

    /* Get generic transparent bitmap */
    pTddb = (UGL_GEN_TDDB *) tDdbId;

    /* Destroy bitmap and bitmask */
    (*devId->bitmapDestroy) (devId, (UGL_DDB_ID) pTddb->ddb, poolId);
    (*devId->monoBitmapDestroy) (devId, (UGL_MDDB_ID) pTddb->mask, poolId);

    /* Free transparent bitmap */
    UGL_PART_FREE (poolId, pTddb);

    /* Release scratch bitmap */
    uglGenericScratchBitmapDestroy (devId, poolId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericTransBitmapBlt - Draw generic transparent bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericTransBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_TDDB_ID    srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GEN_TDDB * pTddb;
    UGL_STATUS     status;

    /* Get generic transparent bitmap */
    pTddb = (UGL_GEN_TDDB *) srcBmpId;

    /* TODO: transparency not supported yet */
    status = (*devId->bitmapBlt) (devId, pTddb->ddb, pSrcRect,
                                  destBmpId, pDestPoint);

    return (status);
}

/******************************************************************************
 *
 * uglGenericScratchBitmapCreate - Create generic scratch bitmap for device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglGenericScratchBitmapCreate (
    UGL_DEVICE_ID    devId,
    UGL_SIZE         width,
    UGL_SIZE         height,
    UGL_MEM_POOL_ID  poolId
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GEN_DDB *        scratchBitmap;
    UGL_DIB              scratchDib;
    UGL_SIZE             maxWidth;
    UGL_SIZE             maxHeight;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Cache scratch bitmap */
    scratchBitmap = pDrv->scratchBitmap;

    /* Check if a new bitmap shall be created */
    if (scratchBitmap == UGL_NULL || width > scratchBitmap->header.width ||
        height > scratchBitmap->header.height) {

        if (scratchBitmap == UGL_NULL) {
            maxWidth  = width;
            maxHeight = height;
        }
        else {
            maxWidth  = max (width, scratchBitmap->header.width);
            maxHeight = max (height, scratchBitmap->header.height);
            (*devId->bitmapDestroy) (devId, (UGL_DDB_ID) scratchBitmap, poolId);
        }

        /* Create device independent bitmap */
        scratchDib.width       = maxWidth;
        scratchDib.height      = maxHeight;
        scratchDib.stride      = maxWidth;
        scratchDib.clutSize    = 0;
        scratchDib.colorFormat = UGL_DEVICE_COLOR_32;
        scratchDib.imageFormat = UGL_DIRECT;
        pDrv->scratchBitmap = (UGL_GEN_DDB *) (*devId->bitmapCreate) (
                                                devId,
                                                &scratchDib,
                                                UGL_DIB_INIT_NONE,
                                                0, poolId);
        if (pDrv->scratchBitmap == UGL_NULL) {
            return (UGL_STATUS_ERROR);
        }

        /* Increase reference count */
        pDrv->transBitmapCount++;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericScratchBitmapDestroy - Destroy generic scratch bitmap for device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglGenericScratchBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_MEM_POOL_ID  poolId
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_STATUS           status;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    if (--pDrv->transBitmapCount == 0) {
        status = (*devId->bitmapDestroy) (devId,
                                          (UGL_DDB_ID) pDrv->scratchBitmap,
                                          poolId);
        pDrv->scratchBitmap = UGL_NULL;
    }

    return (status);
}

