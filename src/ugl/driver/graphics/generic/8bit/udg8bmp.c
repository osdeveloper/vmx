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

/* udg8bmp.c - 8-Bit general bitmap */

#include "ugl.h"
#include "driver/graphics/common/udcomm.h"
#include "driver/graphics/generic/udgen.h"
#include "driver/graphics/generic/udgen8.h"

/******************************************************************************
 *
 * uglGeneric8BitBitmapCreate - Create 8-bit bitmap
 *
 * RETURNS: Bitmap id or UGL_NULL
 */

UGL_DDB_ID uglGeneric8BitBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_UINT32           initValue,
    UGL_MEM_POOL_ID      poolId
    ) {
    UGL_GEN_DDB *        pGenBmp;
    UGL_UINT32           i;
    UGL_UINT32           size;
    UGL_SIZE             width;
    UGL_SIZE             height;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_UINT8 *          buf;
    UGL_STATUS           status;

    /* Get bitmap info, from screen if NULL DIB */
    if (pDib == UGL_NULL) {
        width  = devId->pMode->width;
        height = devId->pMode->height;
    }
    else {
        width  = pDib->width;
        height = pDib->height;
    }

    /* Check if trivial */
    if (width == 0 || height == 0) {
        return (UGL_NULL);
    }

    /* Calculate image size */
    size = width * height * sizeof(UGL_UINT8);

    /* Allocate memory */
    pGenBmp = (UGL_GEN_DDB *) uglOSMemCalloc (poolId, 1,
                                              sizeof (UGL_GEN_DDB) + size);
    if (pGenBmp == NULL) {
        return (UGL_NULL);
    }

    /* Setup structure */
    pGenBmp->colorDepth    = 8;
    pGenBmp->header.type   = UGL_DDB_TYPE;
    pGenBmp->header.width  = width;
    pGenBmp->header.height = height;
    pGenBmp->stride        = width;
    pGenBmp->pData         = &pGenBmp[1];

    buf = (UGL_UINT8 *) pGenBmp->pData;

    /* Initialize contents of image buffer */
    switch(createMode) {

        /* Fill all planes with initial value */
        case UGL_DIB_INIT_VALUE:
            for (i = 0; i < height; i++) {
                memset (&buf[i * width], initValue, width);
            }
            break;

        /* Init from general bitmap */
        case UGL_DIB_INIT_DATA:
            for (i = 0; i < height; i++) {
                memset (&buf[i * width], initValue, width);
            }

            /* Read from source */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = width - 1;
            srcRect.bottom = height - 1;
            destPoint.x    = 0;
            destPoint.y    = 0;
            status = (*devId->bitmapWrite) (devId, pDib, &srcRect,
                                            (UGL_DDB_ID) pGenBmp, &destPoint);

            if (status != UGL_STATUS_OK) {
                uglOSMemFree (pGenBmp);
                return (UGL_NULL);
            }
            break;

        /* None */
        default:
            break;
    }

    return ((UGL_DDB_ID) pGenBmp);
}

/******************************************************************************
 *
 * uglGeneric8BitBitmapDestroy - Free bitmap
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglGeneric8BitBitmapDestroy (
    UGL_DEVICE_ID   devId,
    UGL_DDB_ID      ddbId
    ) {

    /* Free memory */
    uglOSMemFree (ddbId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitBitmapBlt - Blit from one bitmap memory area to another
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_DDB_ID     srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_GEN_DDB *        pSrcBmp;
    UGL_GEN_DDB *        pDestBmp;
    UGL_RECT             srcRect;
    UGL_RECT             clipRect;
    UGL_RECT             destRect;
    UGL_POINT            destPoint;
    const UGL_RECT *     pRect;
    UGL_BLT_DIR          rectOrder;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_RASTER_OP        rasterOp;
    UGL_UINT8 *          pSrc;
    UGL_INT32            srcStride;
    UGL_UINT8 *          pDest;
    UGL_INT32            destStride;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get gc and raster mode */
    gc = pDrv->gc;
    rasterOp = gc->rasterOp;

    /* Clear clipping variables */
    pRect     = UGL_NULL;
    rectOrder = 0;

    if (srcBmpId == UGL_DEFAULT_ID) {
        UGL_RECT_MOVE (*pSrcRect, gc->viewPort.left, gc->viewPort.top);
    }

    if (destBmpId == UGL_DEFAULT_ID) {
        UGL_POINT_MOVE (*pDestPoint, gc->viewPort.left, gc->viewPort.top);

        if (pDestPoint->x > pSrcRect->left) {
            rectOrder |= UGL_BLT_RIGHT;
        }

        if (pDestPoint->y > pSrcRect->top) {
            rectOrder |= UGL_BLT_DOWN;
        }

        if (uglClipListSortedGet (gc, &clipRect, &pRect,
                                  rectOrder) != UGL_STATUS_OK) {
            return (UGL_STATUS_OK);
        }
    }

    do {

        /* Store source and dest */
        pSrcBmp  = (UGL_GEN_DDB *) srcBmpId;
        pDestBmp = (UGL_GEN_DDB *) destBmpId;

        /* Store geometry */
        UGL_RECT_COPY (&srcRect, pSrcRect);
        UGL_POINT_COPY (&destPoint, pDestPoint);

        /* Clip */
        if (uglGenericClipDdbToDdb (devId, &clipRect,
                                    (UGL_BMAP_ID *) &pSrcBmp, &srcRect,
                                    (UGL_BMAP_ID *) &pDestBmp,
                                    &destPoint) == UGL_TRUE) {

            /* Setup dimensions for copy */
            width  = UGL_RECT_WIDTH (srcRect);
            height = UGL_RECT_HEIGHT (srcRect);

            if ((UGL_DDB_ID) pSrcBmp == UGL_DISPLAY_ID) {
                pSrcBmp = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
            }

            if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
                pDestBmp = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
            }

            /* If vertical overlap */
            if ((pSrcBmp == pDestBmp) && (srcRect.top < destPoint.y)) {

                /* Setup source for copy */
                pSrc      = (UGL_UINT8 *) pSrcBmp->pData +
                            (srcRect.bottom * pSrcBmp->stride) + srcRect.left;
                srcStride = -pSrcBmp->stride;

                destRect.left = destPoint.x;
                destRect.top = destPoint.y;
                UGL_RECT_SIZE_TO (destRect, width, height);

                /* Setup destination for copy */
                pDest      = (UGL_UINT8 *) pDestBmp->pData +
                             (destRect.bottom * pDestBmp->stride) +
                             destRect.left;
                destStride = -pDestBmp->stride;
            }
            else {

                /* Setup source for copy */
                srcStride  = pSrcBmp->stride;
                pSrc       = (UGL_UINT8 *) pSrcBmp->pData +
                             (srcRect.top * srcStride) + srcRect.left;

                /* Setup destination for copy */
                destStride = pDestBmp->stride;
                pDest      = (UGL_UINT8 *) pDestBmp->pData+
                             (destPoint.y * destStride) + destPoint.x;
            }

            if (pDrv->gpBusy == UGL_TRUE) {
                if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
                    return (UGL_STATUS_ERROR);
                }
            }

            /* Blit */
            while (--height >= 0) {
                uglCommonByteCopy (pSrc, pDest, width, rasterOp);

                /* Advance line */
                pSrc  += srcStride;
                pDest += destStride;
            }
        }

        if (destBmpId != UGL_DEFAULT_ID) {
            break;
        }

    } while (uglClipListSortedGet (gc, &clipRect, &pRect,
                                   rectOrder) == UGL_STATUS_OK);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitBitmapWrite - Write a device independent bitmap to vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_DIB *      pDib,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     ddbId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GEN_DDB *        pDdb;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_UINT8 *          pSrc;
    UGL_UINT8 *          pDest;
    UGL_ORD              srcStride;
    UGL_ORD              destStride;
    UGL_INT32            i;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_INT32            srcOffset;
    UGL_INT32            numBytes;
    UGL_COLOR            color;
    UGL_ARGB_SPEC        spec;
    UGL_UINT8 *          pClut;
    UGL_ARGB *           pARGBClut = UGL_NULL;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Store bitmap */
    pDdb = (UGL_GEN_DDB *) ddbId;

    /* Get geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    UGL_POINT_COPY (&destPoint, pDestPoint);

    /* Clip */
    if (uglGenericClipDibToDdb (devId, pDib, &srcRect, (UGL_BMAP_ID *) &pDdb,
                                &destPoint) == UGL_TRUE) {

        /* Set destination */
        if ((UGL_DDB_ID) pDdb == UGL_DISPLAY_ID) {
            pDdb = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
        }

        /* Calculate variables */
        width      = UGL_RECT_WIDTH(srcRect);
        height     = UGL_RECT_HEIGHT(srcRect);
        srcOffset  = srcRect.top * pDib->stride + srcRect.left;
        destStride = pDdb->stride;
        pDest      = (UGL_UINT8 *) pDdb->pData +
                     destPoint.y * destStride + destPoint.x;

        if (pDrv->gpBusy == UGL_TRUE) {
            if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
                return (UGL_STATUS_ERROR);
            }
        }

        if (pDib->imageFormat == UGL_DIRECT) {

            /* Select color format */
            switch (pDib->colorFormat) {
                case UGL_DEVICE_COLOR:
                    srcStride  = pDib->stride;
                    pSrc = (UGL_UINT8 *) pDib->pData + srcOffset;

                    /* While source height */
                    while (--height >= 0) {
                        memcpy (pDest, pSrc, width);

                        /* Advance to next line */
                        pSrc  += srcStride;
                        pDest += destStride;
                    }
                    break;

                 default:
                    if (pDib->colorFormat == UGL_DEVICE_COLOR_32) {
                        numBytes = sizeof (UGL_COLOR);
                    }
                    else {
                        if ((uglARGBSpecGet (pDib->colorFormat,
                                             &spec)) != UGL_STATUS_OK) {
                            return (UGL_STATUS_ERROR);
                        }
                        numBytes = spec.numBytesPerARGB;
                    }

                    srcStride = pDib->stride * numBytes;
                    pSrc = (UGL_UINT8 *) pDib->pData + (srcOffset * numBytes);

                    /* While source height */
                    while (--height >= 0) {
                        (*devId->colorConvert) (devId, pSrc, pDib->colorFormat,
                                                pDest, UGL_DEVICE_COLOR, width);

                        /* Advance to next line */
                        pSrc  += srcStride;
                        pDest += destStride;
                    }
                    break;
            }
        }
        else {

            /* Check if temporary clut should be generated */
            if (pDib->colorFormat != UGL_DEVICE_COLOR) {

                pARGBClut = (UGL_ARGB *)
                    UGL_MALLOC (pDib->clutSize * sizeof (UGL_ARGB));
                if (pARGBClut == UGL_NULL) {
                    return (UGL_STATUS_ERROR);
                }

                if (pDib->colorFormat == UGL_DEVICE_COLOR_32) {
                    if ((*devId->colorConvert) (devId, pDib->pClut,
                                                pDib->colorFormat, pARGBClut,
                                                UGL_DEVICE_COLOR,
                                                pDib->clutSize
                                                ) != UGL_STATUS_OK) {
                        UGL_FREE (pARGBClut);
                        return (UGL_STATUS_ERROR);
                    }
                }
                else {
                    if ((*devId->colorConvert) (devId, pDib->pClut,
                                                pDib->colorFormat, pARGBClut,
                                                UGL_ARGB8888,
                                                pDib->clutSize
                                                ) != UGL_STATUS_OK) {
                        UGL_FREE (pARGBClut);
                        return (UGL_STATUS_ERROR);
                    }

                    for (i = 0; i < pDib->clutSize; i++) {
                        uglCommonClutMapNearest (pDrv->pClut, &pARGBClut[i],
                                                 UGL_NULL, &color, 1);
                        ((UGL_UINT8 *) pARGBClut)[i] = (UGL_UINT8) color;
                    }
                }
                pClut = (UGL_UINT8 *) pARGBClut;
            }
            else {
                pClut = (UGL_UINT8 *) pDib->pClut;
            }

            /* Select color mode */
            switch(pDib->imageFormat) {
                case UGL_INDEXED_8:
                    srcStride = pDib->stride;
                    pSrc = (UGL_UINT8 *) pDib->pData + srcOffset;

                    /* While source height */
                    while (--height >= 0) {
                        for (i = 0; i < width; i++) {
                            pDest[i] = pClut[pSrc[i]];
                        }

                        /* Advance to next line */
                        pSrc  += srcStride;
                        pDest += destStride;
                    }
                    break;

                    default:
                        return (UGL_STATUS_ERROR);
            }

            if (pARGBClut != UGL_NULL) {
                UGL_FREE (pARGBClut);
            }
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapCreate - Create 8-bit monochrome bitmap
 *
 * RETURNS: Bitmap id or UGL_NULL
 */

UGL_MDDB_ID uglGeneric8BitMonoBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_MDIB *           pMdib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_UINT8            initValue,
    UGL_MEM_POOL_ID      poolId
    ) {
    UGL_GEN_MDDB *       pGenMonoBmp;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_SIZE             width;
    UGL_SIZE             height;
    UGL_INT32            size;

    /* Get bitmap info, from screen if NULL MDIB */
    if (pMdib == UGL_NULL) {
        width  = devId->pMode->width;
        height = devId->pMode->height;
    }
    else {
        width  = pMdib->width;
        height = pMdib->height;
    }

    /* Check if trivial */
    if (width == 0 || height == 0) {
        return (UGL_NULL);
    }

    /* Caclulate size */
    size = ((pMdib->width * pMdib->height) + 7) / 8;

    /* Allocate memory for bitmap */
    pGenMonoBmp = (UGL_GEN_MDDB *) uglOSMemCalloc (poolId, 1,
                                                   sizeof (UGL_GEN_MDDB) +
                                                   size);
    if (pGenMonoBmp == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Initialize structure */
    pGenMonoBmp->header.width  = width;
    pGenMonoBmp->header.height = height;
    pGenMonoBmp->header.type   = UGL_MDDB_TYPE;
    pGenMonoBmp->stride        = width;
    pGenMonoBmp->pData         = &pGenMonoBmp[1];

    /* Intiaialize data */
    switch(createMode) {
        case UGL_DIB_INIT_VALUE:
            memset (pGenMonoBmp->pData, initValue, size);
            break;

        case UGL_DIB_INIT_DATA:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = width - 1;
            srcRect.bottom = height - 1;
            destPoint.x    = 0;
            destPoint.y    = 0;
            (*devId->monoBitmapWrite) (devId, pMdib, &srcRect,
                                       (UGL_MDDB_ID) pGenMonoBmp, &destPoint);
            break;

        case UGL_DIB_INIT_NONE:
        default:
            break;
    }

    return (UGL_MDDB_ID) pGenMonoBmp;
}

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapDestroy - Free monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglGeneric8BitMonoBitmapDestroy (
    UGL_DEVICE_ID   devId,
    UGL_MDDB_ID     mDdbId
    ) {

    /* Free memory */
    uglOSMemFree (mDdbId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapBlt - Blit monochrome bitmap to color bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitMonoBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_MDDB_ID    srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_RASTER_OP        rasterOp;
    UGL_UINT8            fg;
    UGL_UINT8            bg;
    UGL_GEN_MDDB *       pSrcBmp;
    UGL_GEN_DDB *        pDestBmp;
    UGL_RECT             srcRect;
    UGL_RECT             destRect;
    UGL_POINT            destPoint;
    UGL_INT32            srcStride;
    UGL_UINT8 *          pSrc;
    UGL_INT32            srcRowStart;
    UGL_INT32            srcRow;
    UGL_INT32            destStride;
    UGL_UINT8 *          pDest;
    UGL_INT32            width;
    UGL_INT32            x;
    UGL_INT32            y;
    UGL_UINT8 *          src;
    UGL_UINT8            mask;
    UGL_UINT8 *          dest;
    UGL_RECT             clipRect;
    const UGL_RECT *     pRect;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get gc and cache attributes */
    gc = pDrv->gc;
    rasterOp = gc->rasterOp;
    fg       = (UGL_UINT8) gc->foregroundColor;
    bg       = (UGL_UINT8) gc->backgroundColor;

    /* Start from the top of the clip region list */
    pRect = UGL_NULL;

    if (destBmpId == UGL_DEFAULT_ID) {
        UGL_POINT_MOVE (*pDestPoint, gc->viewPort.left, gc->viewPort.top);

        /* If drawing to default bitmap, enable clip region */
        if (uglClipListGet (gc, &clipRect, &pRect) != UGL_STATUS_OK) {
            return (UGL_STATUS_OK);
        }
    }

    do {

        /* Store source and dest */
        pSrcBmp  = (UGL_GEN_MDDB *) srcBmpId;
        pDestBmp = (UGL_GEN_DDB *) destBmpId;

        /* Get geometry */
        UGL_RECT_COPY (&srcRect, pSrcRect);
        UGL_POINT_COPY (&destPoint, pDestPoint);

        /* Clip */
        if (uglGenericClipDdbToDdb (devId, &clipRect,
                                    (UGL_BMAP_ID *) &pSrcBmp, &srcRect,
                                    (UGL_BMAP_ID *) &pDestBmp,
                                    &destPoint) == UGL_TRUE) {

            /* Setup geometry */
            destRect.left = destPoint.x;
            destRect.top  = destPoint.y;
            UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                              UGL_RECT_HEIGHT (srcRect));

            /* Setup source */
            srcStride     = pSrcBmp->stride;
            pSrc          = (UGL_UINT8 *) pSrcBmp->pData;
            srcRowStart   = (srcRect.top * srcStride) + srcRect.left;

            /* Setup destination */
            if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
                pDestBmp = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
            }

            width      = UGL_RECT_WIDTH (destRect);
            destStride = pDestBmp->stride;
            pDest      = (UGL_UINT8 *) pDestBmp->pData +
                         (destRect.top * destStride) + destRect.left;

            if (pDrv->gpBusy == UGL_TRUE) {
                if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
                    return (UGL_STATUS_ERROR);
                }
            }

            if (gc->foregroundColor != UGL_COLOR_TRANSPARENT) {
                srcRow = srcRowStart;
                dest   = pDest;
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) != 0x00) {
                                    dest[x] = fg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;

                    case UGL_RASTER_OP_AND:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) != 0x00) {
                                    dest[x] &= fg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;

                    case UGL_RASTER_OP_OR:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) != 0x00) {
                                    dest[x] |= fg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;

                    case UGL_RASTER_OP_XOR:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) != 0x00) {
                                    dest[x] ^= fg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;
                }
            }

            if (gc->backgroundColor != UGL_COLOR_TRANSPARENT) {
                srcRow = srcRowStart;
                dest   = pDest;
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) == 0x00) {
                                    dest[x] = bg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;

                    case UGL_RASTER_OP_AND:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) == 0x00) {
                                    dest[x] &= bg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;

                    case UGL_RASTER_OP_OR:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) == 0x00) {
                                    dest[x] |= bg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;

                    case UGL_RASTER_OP_XOR:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            src  = pSrc + (srcRow >> 3);
                            mask = (UGL_UINT8) (0x80 >> (srcRow & 0x07));
                            for (x = 0; x < width; x++) {
                                if ((*src & mask) == 0x00) {
                                    dest[x] ^= bg;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    mask = 0x80;
                                    src++;
                                }
                            }

                            /* Advance row */
                            srcRow += srcStride;
                            dest   += destStride;
                        }
                        break;
                }
            }
        }

        if (destBmpId != UGL_DEFAULT_ID) {
            break;
        }

    } while (uglClipListGet (gc, &clipRect, &pRect) == UGL_STATUS_OK);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapWrite - Write monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitMonoBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_MDIB *     pMdib,
    UGL_RECT *     pSrcRect,
    UGL_MDDB_ID    mDdbId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_INT32            srcStride;
    UGL_UINT8 *          pSrc;
    UGL_INT32            srcIndex;
    UGL_INT32            destStride;
    UGL_UINT8 *          pDest;
    UGL_INT32            destIndex;
    UGL_GEN_MDDB *       pMddb;
    UGL_INT32            width;
    UGL_INT32            height;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Store bitmap */
    pMddb = (UGL_GEN_MDDB *) mDdbId;

    /* Clip */
    if (uglGenericClipDibToDdb (devId, (UGL_DIB *) pMdib, pSrcRect,
                                (UGL_BMAP_ID *) &pMddb,
                                pDestPoint) == UGL_TRUE) {

        /* Setup variables for write */
        width      = UGL_RECT_WIDTH (*pSrcRect);
        height     = UGL_RECT_HEIGHT (*pSrcRect);
        srcStride  = pMdib->stride;
        pSrc       = pMdib->pData;
        srcIndex   = (pSrcRect->top * srcStride) + pSrcRect->left;
        destStride = pMddb->stride;
        pDest      = pMddb->pData;
        destIndex  = (pDestPoint->y * destStride) + pDestPoint->x;

        if (pDrv->gpBusy == UGL_TRUE) {
            if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
                return (UGL_STATUS_ERROR);
            }
        }

        /* Write bitmap */
        while (--height >= 0) {
            uglCommonBitCopy (pSrc, srcIndex, pDest, destIndex,
                              width, UGL_RASTER_OP_COPY);

            /* Advance line */
            srcIndex  += srcStride;
            destIndex += destStride;
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapRead - Read monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitMonoBitmapRead (
    UGL_DEVICE_ID  devId,
    UGL_MDDB_ID    mDdbId,
    UGL_RECT *     pSrcRect,
    UGL_MDIB *     pMdib,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GEN_MDDB *       pMddb;
    UGL_INT32            srcStride;
    UGL_UINT8 *          pSrc;
    UGL_INT32            srcIndex;
    UGL_INT32            destStride;
    UGL_UINT8 *          pDest;
    UGL_INT32            destIndex;
    UGL_INT32            width;
    UGL_INT32            height;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Store bitmap */
    pMddb = (UGL_GEN_MDDB *) mDdbId;

    /* Clip */
    if (uglGenericClipDdbToDib (devId, (UGL_BMAP_ID *) &pMddb, pSrcRect,
                                (UGL_DIB *) pMdib, pDestPoint) == UGL_TRUE) {

        /* Setup variables for read */
        width      = UGL_RECT_WIDTH (*pSrcRect);
        height     = UGL_RECT_HEIGHT (*pSrcRect);
        srcStride  = pMddb->stride;
        pSrc       = (UGL_UINT8 *) pMddb->pData;
        srcIndex   = (pSrcRect->top * srcStride) + pSrcRect->left;
        destStride = pMdib->stride;
        pDest      = (UGL_UINT8 *) pMdib->pData;
        destIndex  = (pDestPoint->y * destStride) + pDestPoint->x;

        if (pDrv->gpBusy == UGL_TRUE) {
            if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
                return (UGL_STATUS_ERROR);
            }
        }

        while (--height >= 0) {
            uglCommonBitCopy (pSrc, srcIndex, pDest, destIndex,
                              width, UGL_RASTER_OP_COPY);

            /* Advance row */
            srcIndex  += srcStride;
            destIndex += destStride;
        }
    }

    return (UGL_STATUS_OK);
}

