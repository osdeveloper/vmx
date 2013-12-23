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
 * uglGeneric8BitBltColorToColor - Blit from memory to memory bitmap
 *
 * RETURNS: N/A
 */

UGL_LOCAL void uglGeneric8BitBltColorToColor (
    UGL_DEVICE_ID  devId,
    UGL_GEN_DDB *  pSrcBmp,
    UGL_RECT *     pSrcRect,
    UGL_GEN_DDB *  pDestBmp,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_RASTER_OP        rasterOp;
    UGL_SIZE             width;
    UGL_SIZE             height;
    UGL_SIZE             srcStride;
    UGL_SIZE             destStride;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;
    UGL_UINT32           i;
    UGL_UINT32           j;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get variables */
    rasterOp   = devId->defaultGc->rasterOp;
    width      = UGL_RECT_WIDTH (*pSrcRect);
    height     = UGL_RECT_HEIGHT (*pSrcRect) - 1;
    srcStride  = pSrcBmp->stride;
    destStride = pDestBmp->stride;

    /* Calculate source address */
    src = ((UGL_UINT8 *) pSrcBmp->pData) +
          pSrcRect->left + (pSrcRect->top * srcStride);

    /* Calculate destination address */
    dest = ((UGL_UINT8 *) pDestBmp->pData) +
           pDestRect->left + (pDestRect->top * destStride);

    /* Blit */
    switch(rasterOp) {
        case UGL_RASTER_OP_COPY:
            for (j = 0; j < height; j++) {
                memcpy (dest, src, width);

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_AND:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] &= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_OR:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] |= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_XOR:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] ^= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;
    }
}

/******************************************************************************
 *
 * uglGeneric8BitBltColorToFrameBuffer - Blit from mem-bitmap to frame buffer
 *
 * RETURNS: N/A
 */

UGL_LOCAL void uglGeneric8BitBltColorToFrameBuffer (
    UGL_DEVICE_ID  devId,
    UGL_GEN_DDB *  pBmp,
    UGL_RECT *     pSrcRect,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_RASTER_OP        rasterOp;
    UGL_SIZE             width;
    UGL_SIZE             height;
    UGL_SIZE             srcStride;
    UGL_SIZE             destStride;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;
    UGL_UINT32           i;
    UGL_UINT32           j;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get variables */
    rasterOp   = devId->defaultGc->rasterOp;
    width      = UGL_RECT_WIDTH (*pSrcRect);
    height     = UGL_RECT_HEIGHT (*pSrcRect) - 1;
    srcStride  = pBmp->stride;
    destStride = devId->pMode->width;

    /* Calculate source address */
    src = ((UGL_UINT8 *) pBmp->pData) +
          pSrcRect->left + (pSrcRect->top * srcStride);

    /* Calculate destination address */
    dest = (UGL_UINT8 *) pDrv->fbAddress +
           pDestRect->left + (pDestRect->top * destStride);

    /* Blit */
    switch(rasterOp) {
        case UGL_RASTER_OP_COPY:
            for (j = 0; j < height; j++) {
                memcpy(dest, src, width);

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_AND:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] &= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_OR:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] |= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_XOR:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] ^= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;
    }
}

/******************************************************************************
 *
 * uglGeneric8BitBltFrameBufferToColor - Blit from frame buffer to memry bitmap
 *
 * RETURNS: N/A
 */

UGL_LOCAL void uglGeneric8BitBltFrameBufferToColor (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pSrcRect,
    UGL_GEN_DDB *  pBmp,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_RASTER_OP        rasterOp;
    UGL_SIZE             width;
    UGL_SIZE             height;
    UGL_SIZE             srcStride;
    UGL_SIZE             destStride;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;
    UGL_UINT32           i;
    UGL_UINT32           j;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get variables */
    rasterOp   = devId->defaultGc->rasterOp;
    width      = UGL_RECT_WIDTH (*pSrcRect);
    height     = UGL_RECT_HEIGHT (*pSrcRect) - 1;
    srcStride  = devId->pMode->width;
    destStride = pBmp->stride;

    /* Calculate source address */
    src = (UGL_UINT8 *) pDrv->fbAddress +
          pSrcRect->left + (pSrcRect->top * srcStride);

    /* Calculate destination address */
    dest = ((UGL_UINT8 *) pBmp->pData) +
          pDestRect->left + (pDestRect->top * destStride);

    /* Blit */
    switch(rasterOp) {
        case UGL_RASTER_OP_COPY:
            for (j = 0; j < height; j++) {
                memcpy(dest, src, width);

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_AND:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] &= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_OR:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] |= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_XOR:
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    dest[i] ^= src[i];
                }

                /* Advance to next line */
                src  += srcStride;
                dest += destStride;
            }
            break;
    }
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
    UGL_RECT             destRect;
    UGL_RECT             clipRect;
    UGL_POINT            destPoint;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get gc */
    gc = pDrv->gc;

    /* Store source and dest */
    pSrcBmp  = (UGL_GEN_DDB *) srcBmpId;
    pDestBmp = (UGL_GEN_DDB *) destBmpId;

    /* Store starting point */
    destPoint.x = pDestPoint->x;
    destPoint.y = pDestPoint->y;

    /* Store source rectangle */
    srcRect.top    = pSrcRect->top;
    srcRect.left   = pSrcRect->left;
    srcRect.right  = pSrcRect->right;
    srcRect.bottom = pSrcRect->bottom;

    /* Store destination rectangle */
    destRect.top    = pDestPoint->y;
    destRect.left   = pDestPoint->x;
    destRect.right  = pDestPoint->x;
    destRect.bottom = pDestPoint->y;

    /* Store clip rectangle */
    if (destBmpId == UGL_DEFAULT_ID) {
        clipRect.top    = 0;
        clipRect.bottom = devId->pMode->height;
        clipRect.left   = 0;
        clipRect.right  = devId->pMode->width;
    }

    /* Clip */
    if (uglGenericClipDdbToDdb (devId, &clipRect,
                           (UGL_BMAP_ID *) &pSrcBmp, &srcRect,
                           (UGL_BMAP_ID *) &pDestBmp, &destPoint) != UGL_TRUE) {
        return UGL_STATUS_ERROR;
    }

    /* Calculate destination */
    UGL_RECT_MOVE_TO_POINT (destRect, destPoint);
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));

    /* Blit */
    if (srcBmpId != UGL_DISPLAY_ID && destBmpId == UGL_DISPLAY_ID) {
        uglGeneric8BitBltColorToFrameBuffer (devId, pSrcBmp, &srcRect,
                                             &destRect);
    }
    else if (srcBmpId == UGL_DISPLAY_ID && destBmpId != UGL_DISPLAY_ID) {
        uglGeneric8BitBltFrameBufferToColor (devId, &srcRect, pDestBmp,
                                             &destRect);
    }
    else {
        uglGeneric8BitBltColorToColor (devId, pSrcBmp, &srcRect, pDestBmp,
                                       &destRect);
    }

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
                                &destPoint) != UGL_TRUE) {
        return (UGL_STATUS_ERROR);
    }

    /* Calculate variables */
    width      = UGL_RECT_WIDTH(srcRect);
    height     = UGL_RECT_HEIGHT(srcRect);
    srcOffset  = srcRect.top * pDib->stride + srcRect.left;
    destStride = pDdb->stride;
    dest       = (UGL_UINT8 *) pDdb->pData +
                 destPoint.y * destStride + destPoint.x;

    /* Handle case when source has a color lookup table */
    if (pDib->imageFormat != UGL_DIRECT) {

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
                                        pDib->clutSize) == UGL_STATUS_ERROR) {
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
                src       = (UGL_UINT8 *) pDib->pData + srcOffset;

                /* While source height */
                while (--height) {
                    memcpy(dest, src, width);

                    /* Advance to next line */
                    src  += srcStride;
                    dest += destStride;
                }
                break;

                default:
                    return (UGL_STATUS_ERROR);
                    break;
            }
    }

    return UGL_STATUS_OK;
}

