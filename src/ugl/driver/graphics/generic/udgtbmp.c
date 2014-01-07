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

#include "ugl.h"
#include "driver/graphics/generic/udgen.h"

/* Globals */

UGL_STATUS uglGenericScratchBitmapCreate (
    UGL_DEVICE_ID  devId,
    UGL_SIZE       width,
    UGL_SIZE       height
    );

/* Locals */

UGL_LOCAL UGL_STATUS uglGenericScratchBitmapDestroy (
    UGL_DEVICE_ID  devId
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
    if (uglGenericScratchBitmapCreate (devId, pDib->width, pDib->height) ==
        UGL_STATUS_ERROR) {
        return (UGL_NULL);
    }

    /* Allocate transparent bitmap */
    pTddb = (UGL_GEN_TDDB *) uglOSMemAlloc (poolId, sizeof (UGL_GEN_TDDB));
    if (pTddb != UGL_NULL) {
        pTddb->header.width  = pDib->width;
        pTddb->header.height = pDib->height;
        pTddb->header.type   = UGL_TDDB_TYPE;

        /* Create bitmap */
        pTddb->ddb = (*devId->bitmapCreate) (devId, pDib, createMode, initValue,
                                             poolId);
        if (pTddb->ddb == UGL_NULL) {
            uglOSMemFree (pTddb);
            return (UGL_NULL);
        }

        /* Create bitmask */
        pTddb->mask = (*devId->monoBitmapCreate) (devId, pMdib, createMode,
                                                  0xff, poolId);
        if (pTddb->mask == UGL_NULL) {
            (*devId->bitmapDestroy) (devId, (UGL_DDB_ID) pTddb->ddb);
            uglOSMemFree (pTddb);
            return (UGL_NULL);
        }
    }

    return (UGL_TDDB_ID) pTddb;
}

/******************************************************************************
 *
 * uglGenericTransBitmapCreateFromDdb - Create generic transparent bitmap ddb
 *
 * RETURNS: UGL_TDDB_ID or UGL_NULL
 */

UGL_TDDB_ID uglGenericTransBitmapCreateFromDdb (
    UGL_DEVICE_ID    devId,
    UGL_DDB_ID       ddbId,
    UGL_MDDB_ID      mDdbId,
    UGL_MEM_POOL_ID  poolId
    ) {
    UGL_GEN_DDB *  pDdb;
    UGL_GEN_TDDB * pTddb;

    /* Get generic bitmap */
    pDdb = (UGL_GEN_DDB *) ddbId;

    /* Create scratch bitmap */
    if (uglGenericScratchBitmapCreate (devId, pDdb->header.width,
                                       pDdb->header.height) ==
        UGL_STATUS_ERROR) {
        return (UGL_NULL);
    }

    /* Allocate transparent bitmap */
    pTddb = (UGL_GEN_TDDB *) uglOSMemAlloc (poolId, sizeof (UGL_GEN_TDDB));
    if (pTddb != UGL_NULL) {
        pTddb->header.width  = pDdb->header.width;
        pTddb->header.height = pDdb->header.height;
        pTddb->header.type   = UGL_TDDB_TYPE;
        pTddb->ddb           = ddbId;
        pTddb->mask          = mDdbId;
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
    UGL_TDDB_ID      tDdbId
    ) {
    UGL_GEN_TDDB * pTddb;

    /* Get generic transparent bitmap */
    pTddb = (UGL_GEN_TDDB *) tDdbId;

    /* Destroy bitmap and bitmask */
    (*devId->bitmapDestroy) (devId, (UGL_DDB_ID) pTddb->ddb);
    (*devId->monoBitmapDestroy) (devId, (UGL_MDDB_ID) pTddb->mask);

    /* Free transparent bitmap */
    uglOSMemFree (pTddb);

    /* Release scratch bitmap */
    uglGenericScratchBitmapDestroy (devId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericTransBitmapLinearBlt - Draw transparent bitmap for linear display
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericTransBitmapLinearBlt (
    UGL_DEVICE_ID  devId,
    UGL_TDDB_ID    srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GEN_TDDB * pSrcTddb;
    UGL_DDB *      pSrcDdb;
    UGL_MDDB *     pSrcMddb;
    UGL_DDB *      pDestDdb;
    UGL_RECT       srcRect;
    UGL_POINT      destPoint;
    UGL_INT32      y;
    UGL_INT32      width;
    UGL_INT32      height;
    UGL_INT32      numColumns;
    UGL_INT32      numPixels;
    UGL_INT32      dataWidth;
    UGL_INT32      srcLeft;
    UGL_INT32      srcRight;
    UGL_INT32      destLeft;
    UGL_UINT8 *    pBuf;
    UGL_UINT8 *    pMaskData;
    UGL_UINT8      mask;
    UGL_POINT      mDibPoint;
    UGL_MDIB       mDib;
    UGL_RECT       drawRect;
    UGL_POINT      drawPoint;

    /* Get generic transparent bitmap and components */
    pSrcTddb = (UGL_GEN_TDDB *) srcBmpId;
    pSrcDdb  = (UGL_DDB *) pSrcTddb->ddb;
    pSrcMddb = (UGL_MDDB *) pSrcTddb->mask;

    /* Get destination */
    pDestDdb = (UGL_DDB *) destBmpId;

    /* Get geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    UGL_POINT_COPY (&destPoint, pDestPoint);
    width  = UGL_RECT_WIDTH (srcRect);
    height = UGL_RECT_HEIGHT (srcRect);
    mDibPoint.x = 0;
    mDibPoint.y = 0;

    /* Get number columne in a row */
    numColumns = (width + 7) / 8;

    pBuf = (UGL_UINT8 *) uglScratchBufferAlloc (devId, numColumns *
                                                       sizeof (UGL_UINT8));
    if (pBuf == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Setup device independent bitmask for one row */
    mDib.width  = width;
    mDib.height = 1;
    mDib.stride = width;
    mDib.pData  = pBuf;

    /* Setup geometry */
    srcLeft  = srcRect.left;
    srcRight = srcRect.right;
    destLeft = destPoint.x;

    /* Start at first row */
    srcRect.bottom = srcRect.top;

    /* Over height */
    for (y = 0; y < height; y++) {
        srcRect.left  = srcLeft;
        srcRect.right = srcRight;
        destPoint.x   = destLeft;

        /* Read row from bitmask */
        (*devId->monoBitmapRead) (devId, pSrcMddb, &srcRect, &mDib, &mDibPoint);

        /* Cache mask pointer */
        pMaskData = (UGL_UINT8 *) mDib.pData;

        /* Setup row run */
        dataWidth = 0;
        numPixels = width;

        /* Over mask chunks */
        mask = 0x80;
        while (1) {

            if (mask == 0x80 && numPixels >= 8 && *pMaskData == 0xff) {
                /* All pixels are visible for chunk */
                dataWidth += 8;
                numPixels -= 8;
                pMaskData++;
            }
            else if ((*pMaskData & mask) != 0x00 && numPixels > 0) {
                /* At least one pixel visible */
                dataWidth++;
                numPixels--;

                /* Advance mask */
                if ((mask >>= 1) == 0x00) {
                    mask = 0x80;
                    pMaskData++;
                }
            }
            else {
                /* Masked out pixel or end of line */
                if (dataWidth > 0) {
                    srcRect.right = srcRect.left + dataWidth - 1;
                    UGL_RECT_COPY (&drawRect, &srcRect);
                    UGL_POINT_COPY (&drawPoint, &destPoint);

                    /* Draw run */
                    (*devId->bitmapBlt) (devId, pSrcDdb, &drawRect,
                                         pDestDdb, &drawPoint);

                    /* Advance */
                    srcRect.left += dataWidth;
                    destPoint.x  += dataWidth;
                    dataWidth     = 0;
                }

                /* Check if anything more to draw */
                if (numPixels == 0) {
                    break;
                }

                while (numPixels > 0) {
                    if (mask == 0x80 && numPixels >= 8 && *pMaskData == 0x00) {
                        dataWidth += 8;
                        numPixels -= 8;
                        pMaskData++;

                        /* If end of line */
                        if (numPixels == 0) {
                            dataWidth = 0;
                        }
                    }
                    else if ((*pMaskData & mask) == 0x00 && numPixels > 0) {
                        dataWidth++;
                        numPixels--;

                        /* Advance mask */
                        if ((mask >>= 1) == 0x00) {
                            mask = 0x80;
                            pMaskData++;
                        }

                        /* If end of line */
                        if (numPixels == 0) {
                            dataWidth = 0;
                        }
                    }
                    else {
                        /* Pixel is not masked, break out of loop */
                        srcRect.left += dataWidth;
                        destPoint.x  += dataWidth;
                        dataWidth     = 0;
                        break;
                    }
                }
            }
        }

        /* Advance row */
        srcRect.top++;
        srcRect.bottom++;
        destPoint.y++;
    }

    uglScratchBufferFree (devId, pBuf);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericScratchBitmapCreate - Create generic scratch bitmap for device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericScratchBitmapCreate (
    UGL_DEVICE_ID  devId,
    UGL_SIZE       width,
    UGL_SIZE       height
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
            (*devId->bitmapDestroy) (devId, (UGL_DDB_ID) scratchBitmap);
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
                                                0, UGL_NULL);
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
    UGL_DEVICE_ID  devId
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_STATUS           status;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    if (--pDrv->transBitmapCount == 0) {
        status = (*devId->bitmapDestroy) (devId,
                                          (UGL_DDB_ID) pDrv->scratchBitmap);
        pDrv->scratchBitmap = UGL_NULL;
    }

    return (status);
}

