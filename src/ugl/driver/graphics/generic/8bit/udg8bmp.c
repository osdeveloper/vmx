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

#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcomm.h>
#include <ugl/driver/graphics/generic/udgen8.h>
#include <ugl/driver/graphics/generic/udgen.h>

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
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GEN_DDB *        pGenBmp;
    UGL_UINT32           i;
    UGL_UINT32           size;
    UGL_SIZE             width;
    UGL_SIZE             height;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_UINT8 *          buf;
    UGL_STATUS           status;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get bitmap info, from screen if NULL DIB */
    if (pDib == UGL_NULL) {
        width  = devId->pMode->width;
        height = devId->pMode->height;
    }
    else {
        width  = pDib->width;
        height = pDib->height;
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
    pGenBmp->pData         = (void *) ((int) pGenBmp + sizeof (UGL_GEN_DDB));

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
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;
    UGL_ORD              srcStride;
    UGL_ORD              destStride;
    UGL_INT32            i;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_INT32            srcOffset;
    UGL_INT32            numBytes;
    UGL_CLUT *           pClut;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Store bitmap */
    pDdb = (UGL_GEN_DDB *) ddbId;

    /* Store source rect */
    srcRect.top    = pSrcRect->top;
    srcRect.bottom = pSrcRect->bottom;
    srcRect.left   = pSrcRect->left;
    srcRect.right  = pSrcRect->right;

    /* Store destination point */
    destPoint.x = pDestPoint->x;
    destPoint.y = pDestPoint->y;

    /* Clip */
    if (uglGenericClipDibToDdb (devId, pDib, &srcRect, (UGL_BMAP_ID *) &pDdb,
                                &destPoint) == UGL_TRUE) {

        /* Calculate variables */
        width      = UGL_RECT_WIDTH(srcRect);
        height     = UGL_RECT_HEIGHT(srcRect);
        srcOffset  = srcRect.top * pDib->stride + srcRect.left;
        destStride = pDdb->stride;
        dest       = (UGL_UINT8 *) pDdb->pData +
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
                    src = (UGL_UINT8 *) pDib->pData + srcOffset;

                    /* While source height */
                    while (--height >= 0) {
                        memcpy (dest, src, width);

                        /* Advance to next line */
                        src  += srcStride;
                        dest += destStride;
                    }
                    break;

                 default:
                    if (pDib->colorFormat == UGL_DEVICE_COLOR_32) {
                        numBytes = sizeof (UGL_COLOR);
                    }
                    else {
                        /* TODO */ numBytes = 1;
                    }

                    srcStride = pDib->stride * numBytes;
                    src = (UGL_UINT8 *) pDib->pData + (srcOffset * numBytes);

                    /* While source height */
                    while (--height >= 0) {
                        (*devId->colorConvert) (devId, src, pDib->colorFormat,
                                                dest, UGL_DEVICE_COLOR, width);

                        /* Advance to next line */
                        src  += srcStride;
                        dest += destStride;
                    }
                    break;
            }
        }
        else {

            /* Check if temporary clut should be generated */
            if (pDib->colorFormat != UGL_DEVICE_COLOR_32) {

                pClut = UGL_MALLOC (pDib->clutSize * sizeof (UGL_COLOR));
                if (pClut == UGL_NULL) {
                    return (UGL_STATUS_ERROR);
                }

                /* Convert to 32-bit color */
                if ((*devId->colorConvert) (devId, pDib->pClut,
                                            pDib->colorFormat, pClut,
                                            UGL_DEVICE_COLOR_32,
                                            pDib->clutSize
                                            ) == UGL_STATUS_ERROR) {
                    UGL_FREE (pClut);
                    return (UGL_STATUS_ERROR);
                }
            }
            else {
                pClut = pDib->pClut;
            }

            /* Select color mode */
            switch(pDib->imageFormat) {
                case UGL_INDEXED_8:
                    srcStride = pDib->stride;
                    src = (UGL_UINT8 *) pDib->pData + srcOffset;

                    /* While source height */
                    while (--height >= 0) {
                        memcpy(dest, src, width);

                        /* Advance to next line */
                        src  += srcStride;
                        dest += destStride;
                    }
                    break;

                    default:
                        return (UGL_STATUS_ERROR);
            }
        }
    }

    return (UGL_STATUS_OK);
}

