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

/* udvgabmp.c - Vga bitmap support */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <vmx.h>
#include <arch/sysArchLib.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/graphics/vga/udvga.h>

/******************************************************************************
 *
 * uglVgaBitmapCreate - Create vga bitmap
 *
 * RETURNS: Pointer to bitmap
 */

UGL_DDB_ID uglVgaBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_UINT32           initValue,
    UGL_MEM_POOL_ID      poolId
    ) {
    UGL_VGA_DRIVER * pDrv;
    UGL_VGA_DDB *    pVgaBmp;
    UGL_INT32        i;
    UGL_SIZE         numPlanes;
    UGL_UINT32       planeSize;
    UGL_UINT32       bmpSize;
    UGL_SIZE         width;
    UGL_SIZE         height;
    UGL_UINT8 *      pPlane;
    UGL_UINT32       planeShift;
    UGL_RECT         srcRect;
    UGL_POINT        destPoint;
    UGL_STATUS       status;

    /* Get driver first in device struct */
    pDrv = (UGL_VGA_DRIVER *) devId;

    /* Get number of bitplanes */
    numPlanes = pDrv->colorPlanes;

    /* Get bitmap info, from screen if NULL DIB */
    if (pDib == UGL_NULL) {
        width  = devId->pMode->width;
        height = devId->pMode->height;
    }
    else {
        width  = pDib->width;
        height = pDib->height;
    }

    /* Calculate plane size including 1 shift byte for each scanline */
    planeSize = ((width + 7) / 8 + 1) * height;

    /* Calculate size */
    bmpSize = sizeof(UGL_VGA_DDB) + (numPlanes * sizeof(UGL_UINT8 *)) +
              (numPlanes * planeSize);

    /* Allocate bitmap */
    pVgaBmp = (UGL_VGA_DDB *) uglOSMemCalloc (poolId, 1, bmpSize);
    if (pVgaBmp == NULL) {
        return (UGL_NULL);
    }

    /* Setup bitmap header */
    pVgaBmp->header.type   = UGL_DDB_TYPE;
    pVgaBmp->header.width  = width;
    pVgaBmp->header.height = height;
    pVgaBmp->colorDepth    = numPlanes;
    pVgaBmp->stride        = width;
    pVgaBmp->shiftValue    = 0;

    /* Setup color planes */
    pVgaBmp->pPlaneArray = (UGL_UINT8 **) (((UGL_UINT8 *) pVgaBmp) +
                           sizeof (UGL_VGA_DDB));
    pPlane = (UGL_UINT8 *) &pVgaBmp->pPlaneArray[numPlanes];
    for (i = 0; i < numPlanes; i++) {
        pVgaBmp->pPlaneArray[i] = pPlane;
        pPlane += planeSize;
    }

    /* Initialize contents of color planes */
    switch(createMode) {
        case UGL_DIB_INIT_VALUE:
            planeShift = 0x01;

            for (i = 0; i < numPlanes; i++) {
                if ((UGL_UINT8) (initValue & planeShift) != 0) {
                    memset (pVgaBmp->pPlaneArray[i], 0xff, planeSize);
                }
                else {
                    memset (pVgaBmp->pPlaneArray[i], 0x00, planeSize);
                }
                planeShift <<= 1;
            }
            break;

        case UGL_DIB_INIT_DATA:

            /* Read from source */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = width - 1;
            srcRect.bottom = height - 1;
            destPoint.x = 0;
            destPoint.y = 0;

            status = (*devId->bitmapWrite) (devId, pDib, &srcRect,
                                            (UGL_DDB_ID) pVgaBmp, &destPoint);

            if (status != UGL_STATUS_OK) {
                uglOSMemFree (pVgaBmp);
                return (UGL_NULL);
            }
            break;

            case UGL_DIB_INIT_NONE:
            default:
                break;
    }

    return (UGL_DDB_ID) pVgaBmp;
}

/******************************************************************************
 *
 * uglVgaBitmapDestroy - Free vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_DDB_ID       ddbId
    ) {

    /* Free memory */
    uglOSMemFree (ddbId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaShiftBitmap - Shift bitmap
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaShiftBitmap (
    UGL_DEVICE_ID     devId,
    UGL_BMAP_HEADER * pBmp,
    UGL_UINT8 **      pPlaneArray,
    UGL_INT32         prevShift,
    UGL_INT32         shift
    ) {
    UGL_VGA_DRIVER * pDrv;
    UGL_INT32        x;
    UGL_INT32        y;
    UGL_SIZE         planeIndex;
    UGL_SIZE         numPlanes;
    UGL_INT32        bytesPerLine;
    UGL_UINT8 *      ptr;
    UGL_UINT16       shiftReg;
    UGL_UINT8        shiftRest;
    UGL_INT16        shiftDelta;

    /* Get driver first in devide struct */
    pDrv = (UGL_VGA_DRIVER *) devId;

    /* Setup variables for shift */
    shiftDelta   = (UGL_INT16) (shift - prevShift);
    numPlanes    = pDrv->colorPlanes;
    bytesPerLine = (pBmp->width + 7) / 8 + 1;

    /* shiftDelta > 0 */
    if (shiftDelta > 0) {

        /* Recalculate shift delta */
        shiftDelta = 8 - shiftDelta;

        /* For all planes */
        for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

            /* Get next plane pointer */
            ptr = pPlaneArray[planeIndex];
            if (ptr == UGL_NULL) {
                break;
            }

            /* Calculate height */
            y = pBmp->height + 1;

            /* Over y */
            while (--y) {
                shiftRest = 0;

                /* Calculate length */
                x = bytesPerLine + 1;

                /* Over x */
                while (--x) {
                    shiftReg = *ptr;
                    shiftReg <<= shiftDelta;
                    *(ptr++) = shiftRest | UGL_HI_BYTE (shiftReg);
                    shiftRest = UGL_LO_BYTE (shiftReg);
                }
            }
        }
    }
    else {

        /* Recalculate shift delta */
        shiftDelta = -shiftDelta;

        /* For all planes */
        for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

            /* Get next plane pointer */
            ptr = pPlaneArray[planeIndex];
            if (ptr == UGL_NULL) {
                break;
            }

            /* Calculate height */
            y = pBmp->height + 1;

            /* Over y */
            while (--y) {
                shiftReg = *ptr;
                shiftReg <<= shiftDelta;
                shiftRest = UGL_LO_BYTE (shiftReg);

                /* Calculate length */
                x = bytesPerLine;

                /* Over x */
                while (--x) {
                    shiftReg = ptr[1];
                    shiftReg <<= shiftDelta;
                    *(ptr++) = shiftRest | UGL_HI_BYTE (shiftReg);
                    shiftRest = UGL_LO_BYTE (shiftReg);
                }

                /* One extra for each line */
                *(ptr++) = shiftRest;
            }
        }
    }
}

/*****************************************************************************
 *
 * uglVgaBltAlign - Align source bitmap with destination
 *
 * RETURNS: N/A
 */

UGL_LOCAL uglVgaBltAlign (
    UGL_DEVICE_ID     devId,
    UGL_BMAP_HEADER * pSrcBmp,
    UGL_RECT *        pSrcRect,
    UGL_VGA_DDB *     pDestBmp,
    UGL_RECT *        pDestRect
    ) {
    UGL_INT32     shift;
    UGL_INT32     prevShift;
    UGL_VGA_DDB * pDdb;
    UGL_UINT8 **  img;

    /* Setup variables */
    pDdb = (UGL_VGA_DDB *) pSrcBmp;

    /* Calculate shift value */
    if (pDestBmp != UGL_NULL) {
        shift = pDestBmp->shiftValue +
                (pDestRect->left & 0x07) - (pSrcRect->left & 0x07);
        pDestRect->left  += pDestBmp->shiftValue;
        pDestRect->right += pDestBmp->shiftValue;
    }
    else {
        shift = (pDestRect->left & 0x07) - (pSrcRect->left & 0x07);
    }

    /* Check shift value range */
    if (shift < 0) {
        shift += 8;
    }
    else if (shift > 7) {
        shift -= 8;
    }

    prevShift = pDdb->shiftValue;
    img = pDdb->pPlaneArray;
    pDdb->shiftValue = shift;

    if (prevShift != shift) {
        uglVgaShiftBitmap (devId, pSrcBmp, img, prevShift, shift);
    }

    pSrcRect->left  += shift;
    pSrcRect->right += shift;
}

/******************************************************************************
 *
 * uglVgaBltPlane - Blit to one bitplane
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaBltPlane (
    UGL_DEVICE_ID  devId,
    UGL_UINT8 *    pSrc,
    UGL_RECT *     pSrcRect,
    UGL_SIZE       srcStride,
    UGL_UINT8 *    pDest,
    UGL_RECT *     pDestRect,
    UGL_SIZE       destStride,
    UGL_RASTER_OP  rasterOp
    ) {
    UGL_INT32   i;
    UGL_INT32   y;
    UGL_UINT32  width;
    UGL_UINT32  height;
    UGL_UINT8 * src;
    UGL_UINT8 * dest;
    UGL_UINT8   startMask;
    UGL_UINT8   endMask;

    /* Calculate vars */
    width  = (pDestRect->right >> 3) - (pDestRect->left >> 3) + 1;
    height = UGL_RECT_HEIGHT (*pDestRect);
    src    = pSrc + pSrcRect->top * srcStride + (pSrcRect->left >> 3);
    dest   = pDest + pDestRect->top * destStride + (pDestRect->left >> 3);

    /* Generate masks */
    startMask = 0xff >> (pDestRect->left & 0x07);
    endMask = 0xff << (7 - (pDestRect->right & 0x07));

    /* if pSrc == 0 => Fill entire plane with zeros */
    if (pSrc == (UGL_UINT8 *) 0) {

        /* Select rasterOp */
        switch(rasterOp) {
            case UGL_RASTER_OP_COPY:
            case UGL_RASTER_OP_AND:

                /* Over height */
                for (y = 0; y < height; y++) {

                    /* Just one pixel */
                    if (width == 1) {
                        dest[0] &= ~(startMask & endMask);
                    }
                    else {

                        /* Fill start start */
                        dest[0] &= ~startMask;

                        /* Fill middle */
                        if (width > 2) {
                            memset (&dest[1], 0, width - 2);
                        }

                        /* Fill end mask */
                        dest[width - 1] &= ~endMask;
                    }

                    /* Advance to next line */
                    dest += destStride;
                }
                break;
        }

        /* Done */
        return;
    }

    /* if pSrc == -1 => Fill entire plane with ones */
    if (pSrc == (UGL_UINT8 *) -1) {

        /* Select rasterOp */
        switch(rasterOp) {
            case UGL_RASTER_OP_COPY:
            case UGL_RASTER_OP_OR:

                /* Over height */
                for (y = 0; y < height; y++) {

                    /* Just one pixel */
                    if (width == 1) {
                        dest[0] |= startMask & endMask;
                    }
                    else {

                        /* Fill start mask */
                        dest[0] |= startMask;

                        /* Fill middle */
                        if (width > 2) {
                            memset (&dest[1], 0xff, width - 2);
                        }

                        /* Fill end mask */
                        dest[width - 1] |= endMask;
                    }

                    /* Advance to next line */
                    dest += destStride;
                }
                break;

            case UGL_RASTER_OP_XOR:

                /* Over height */
                for (y = 0; y < height; y++) {

                    /* Just one pixel */
                    if (width == 1) {
                        dest[0] ^= startMask & endMask;
                    }
                    else {

                        /* Fill start mask */
                        dest[0] ^= startMask;

                        /* Fill middle */
                        if (width > 2) {
                            for (i = 1; i < width - 1; i++) {
                                dest[i] ^= 0xff;
                            }
                        }

                        /* Fill end mask */
                        dest[i] ^= endMask;
                    }

                    /* Advance to next line */
                    dest += destStride;
                }
                break;
        }

        /* Done */
        return;
    }

    /* If here this is a plane copy */
    /* Select raster op */
    switch(rasterOp) {
        case UGL_RASTER_OP_COPY:

            /* Over height */
            for (y = 0; y < height; y++) {

                /* Check if just one pixel */
                if (width == 1) {
                    dest[0] |= src[0] & (startMask & endMask);
                    dest[0] &= src[0] | ~(startMask & endMask);
                }
                else {

                    /* Blit start mask */
                    dest[0] |= src[0] & startMask;
                    dest[0] &= src[0] | ~startMask;

                    /* Blit middle */
                    if (width > 2) {
                        memcpy (&dest[1], &src[1], width - 2);
                    }

                    /* Blit end mask */
                    dest[width - 1] |= src[width - 1] & endMask;
                    dest[width - 1] &= src[width - 1] | ~endMask;
                }

                /* Advance one line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_AND:

            /* Over height */
            for (y = 0; y < height; y++) {

                /* Check if just one pixel */
                if (width == 1) {
                    dest[0] &= src[0] | ~(startMask & endMask);
                }
                else {

                    /* Blit start mask */
                    dest[0] &= src[0] | ~startMask;

                    /* Blit middle */
                    if (width > 2) {
                        for (i = 1; i < width - 1; i++) {
                            dest[i] &= src[i];
                        }
                    }

                    /* Blit end mask */
                    dest[i] &= src[i] | ~endMask;
                }

                /* Advance one line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_OR:

            /* Over height */
            for (y = 0; y < height; y++) {

                /* Check if just one pixel */
                if (width == 1) {
                    dest[0] |= src[0] & (startMask & endMask);
                }
                else {

                    /* Blit start mask */
                    dest[0] |= src[0] & startMask;

                    /* Blit middle */
                    if (width > 2) {
                        for (i = 1; i < width - 1; i++) {
                            dest[i] |= src[i];
                        }
                    }

                    /* Blit end mask */
                    dest[i] |= src[i] & endMask;
                }

                /* Advance one line */
                src  += srcStride;
                dest += destStride;
            }
            break;

        case UGL_RASTER_OP_XOR:

            /* Over height */
            for (y = 0; y < height; y++) {

                /* Check if just one pixel */
                if (width == 1) {
                    dest[0] ^= src[0] & (startMask & endMask);
                }
                else {

                    /* Blit start mask */
                    dest[0] ^= src[0] & startMask;

                    /* Blit middle */
                    if (width > 2) {
                        for (i = 1; i < width - 1; i++) {
                            dest[i] ^= src[i];
                        }
                    }

                    /* Blit end mask */
                    dest[i] ^= src[i] & endMask;
                }

                /* Advance one line */
                src  += srcStride;
                dest += destStride;
            }
            break;
    }
}

/******************************************************************************
 *
 * uglVgaBltColorToColor - Blit from memory to memory bitmap
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaBltColorToColor (
    UGL_DEVICE_ID  devId,
    UGL_VGA_DDB *  pSrcBmp,
    UGL_RECT *     pSrcRect,
    UGL_VGA_DDB *  pDestBmp,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_RASTER_OP        rasterOp;
    UGL_SIZE             planeIndex;
    UGL_SIZE             numPlanes;
    UGL_SIZE             srcStride;
    UGL_SIZE             destStride;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Cache raster op */
    rasterOp = pDrv->gc->rasterOp;

    /* Calculate vars */
    numPlanes  = pDestBmp->colorDepth;
    srcStride  = (pSrcBmp->header.width + 7) / 8 + 1;
    destStride = (pDestBmp->header.width + 7) / 8 + 1;

    /* Align source bitmap to dest */
    uglVgaBltAlign (devId, (UGL_BMAP_HEADER *) pSrcBmp, pSrcRect,
                    pDestBmp, pDestRect);

    /* Over all blit planes */
    for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {
        src  = pSrcBmp->pPlaneArray[planeIndex];
        dest = pDestBmp->pPlaneArray[planeIndex];

        uglVgaBltPlane (devId,
                        src, pSrcRect, srcStride,
                        dest, pDestRect, destStride,
                        pDrv->gc->rasterOp);
    }
}

/******************************************************************************
 *
 * uglVgaBltColorToFrameBuffer - Blit from memory bitmap to frame buffer
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaBltColorToFrameBuffer (
    UGL_DEVICE_ID  devId,
    UGL_VGA_DDB *  pBmp,
    UGL_RECT *     pSrcRect,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_VGA_DRIVER *     pVgaDrv;
    UGL_RASTER_OP        rasterOp;
    volatile UGL_UINT8   tmp;
    UGL_INT32            i;
    UGL_INT32            y;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_SIZE             planeIndex;
    UGL_SIZE             numPlanes;
    UGL_INT32            destBytesPerLine;
    UGL_INT32            srcBytesPerLine;
    UGL_INT32            srcOffset;
    UGL_UINT8 *          destStart;
    UGL_UINT8            startMask;
    UGL_UINT8            endMask;
    UGL_UINT8            regValue;
    UGL_UINT8            byteValue;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;

    /* Get driver first in device struct */
    pDrv    = (UGL_GENERIC_DRIVER *) devId;
    pVgaDrv = (UGL_VGA_DRIVER *) devId;

    /* Cache raster op */
    rasterOp = pDrv->gc->rasterOp;

    /* Align source bitmap to dest */
    uglVgaBltAlign (devId, (UGL_BMAP_HEADER *) pBmp, pSrcRect,
                    UGL_NULL, pDestRect);

    /* Setup variables for blit */
    width            = (pDestRect->right >> 3) - (pDestRect->left >> 3) + 1;
    height           = UGL_RECT_HEIGHT (*pDestRect);
    numPlanes        = devId->pMode->depth;
    destBytesPerLine = pVgaDrv->bytesPerLine;
    srcBytesPerLine  = (pBmp->header.width + 7) / 8 + 1;
    destStart        = (UGL_UINT8 *) pDrv->fbAddress +
                       (pDestRect->top * destBytesPerLine) +
                       (pDestRect->left >> 3);
    srcOffset        = pSrcRect->top * srcBytesPerLine +
                       (pSrcRect->left >> 3);

    /* Generate masks */
    startMask = 0xff >> (pDestRect->left & 0x07);
    endMask   = 0xff << (7 - (pDestRect->right & 0x07));
    if (width == 1) {
        startMask &= endMask;
    }

    /* Setup registers */

    /* Write mode 0 */
    UGL_OUT_BYTE (0x3ce, 0x05);
    regValue  = UGL_IN_BYTE (0x3cf);
    regValue &= 0xf8;
    UGL_OUT_BYTE (0x3cf, regValue);

    /* Disable set/reset registers */
    UGL_OUT_WORD (0x3ce, 0x0001);

    /* Select bit mask register */
    UGL_OUT_BYTE (0x3ce, 0x08);

    /* Select write plane */
    UGL_OUT_BYTE (0x3c4, 0x02);

    /* Blit */

    /* Over height */
    for (y = 0; y < height; y++) {

        /* For all planes */
        for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

            /* Calulcate plane vars */
            src  = pBmp->pPlaneArray[planeIndex] + srcOffset;
            dest = destStart;
            i    = 0;

            /* Select plane */
            regValue = 0x01 << planeIndex;
            UGL_OUT_BYTE (0x3c5, regValue);

            /* Write start mask */
            UGL_OUT_BYTE (0x3cf, startMask);
            tmp = dest[i];
            dest[i] = src[i];
            i++;

            /* Check if anything to more blit */
            if (width > 1) {

                /* Check if bigger blit */
                if (width > 2) {

                    /* Set register to all bits to visible */
                    UGL_OUT_BYTE (0x3cf, 0xff);

                    /* Check if just a straight copy */
                    if (rasterOp == UGL_RASTER_OP_COPY) {
                        memcpy (&dest[i], &src[i], width - 2);
                        i += width - 2;
                    }
                    else {
                        for (i = 1; i < width - 1; i++) {
                            tmp = dest[i];
                            dest[i] = src[i];
                        }
                    }
                }

                /* Write end mask */
                UGL_OUT_BYTE (0x3cf, endMask);
                tmp = dest[i];
                dest[i] = src[i];
            }
        }

        /* Advance line */
        srcOffset += srcBytesPerLine;
        destStart += destBytesPerLine;
    }

    /* Restore registers */

    /* Restore mask register */
    UGL_OUT_BYTE (0x3c5, 0x0f);

    /* Restore bitmask register */
    UGL_OUT_BYTE (0x3cf, 0xff);

    /* Set write mode 3 */
    UGL_OUT_BYTE (0x3ce, 0x05);
    byteValue = UGL_IN_BYTE (0x3cf);
    UGL_OUT_BYTE (0x3cf, byteValue | 0x03);

    /* Enable set reset registers */
    UGL_OUT_WORD (0x3ce, 0x0f01);
}

/******************************************************************************
 *
 * uglVgaBltFrameBufferToColor - Blit from frame buffer to memry bitmap
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaBltFrameBufferToColor (
    UGL_DEVICE_ID devId,
    UGL_RECT *    pSrcRect,
    UGL_VGA_DDB * pBmp,
    UGL_RECT *    pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_VGA_DRIVER *     pVgaDrv;
    UGL_RASTER_OP        rasterOp;
    UGL_INT32            i;
    UGL_INT32            y;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_SIZE             planeIndex;
    UGL_SIZE             numPlanes;
    UGL_INT32            destBytesPerLine;
    UGL_INT32            srcBytesPerLine;
    UGL_INT32            destOffset;
    UGL_UINT8            startMask;
    UGL_UINT8            endMask;
    UGL_UINT8            byteValue;
    UGL_UINT8 *          src;
    UGL_UINT8 *          srcStart;
    UGL_UINT8 *          dest;

    /* Get driver first in device struct */
    pDrv    = (UGL_GENERIC_DRIVER *) devId;
    pVgaDrv = (UGL_VGA_DRIVER *) devId;

    /* Cache raster op */
    rasterOp = pDrv->gc->rasterOp;

    /* Align source bitmap to dest */
    uglVgaBltAlign (devId, (UGL_BMAP_HEADER *) pBmp, pDestRect,
                    UGL_NULL, pSrcRect);

    /* Setup variables for blit */
    width            = (pSrcRect->right >> 3) - (pSrcRect->left >> 3) + 1;
    height           = UGL_RECT_HEIGHT (*pDestRect);
    numPlanes        = pBmp->colorDepth;
    srcBytesPerLine  = pVgaDrv->bytesPerLine;
    destBytesPerLine = (pBmp->header.width + 7) / 8 + 1;
    srcStart         = (UGL_UINT8 *) pDrv->fbAddress +
                       (pSrcRect->top * srcBytesPerLine) +
                       (pSrcRect->left >> 3);
    destOffset       = pDestRect->top * destBytesPerLine +
                       (pDestRect->left >> 3);

    /* Generate masks */
    startMask = 0xff >> (pDestRect->left & 0x07);
    endMask   = 0xff << (7 - (pDestRect->right & 0x07));
    if (width == 1) {
        startMask &= endMask;
    }

    /* Select read plane */
    UGL_OUT_BYTE (0x3ce, 0x04);

    /* Blit */

    /* For all planes */
    for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

        /* Calulcate plane vars */
        src  = srcStart;
        dest = pBmp->pPlaneArray[planeIndex] + destOffset;

        /* Select plane */
        UGL_OUT_BYTE (0x3cf, planeIndex);

        /* Select raster op */
        switch(rasterOp) {
            case UGL_RASTER_OP_COPY:

                /* Over height */
                for (y = 0; y < height; y++) {
                    i = 0;

                    /* Blit start */
                    byteValue = src[i];
                    dest[i] |= byteValue & startMask;
                    dest[i] &= byteValue | ~startMask;
                    i++;

                    /* Check if anything to more blit */
                    if (width > 1) {

                        /* Check if bigger blit */
                        if (width > 2) {
                            memcpy (&dest[i], &src[i], width - 2);
                            i += width - 2;
                        }

                        /* Blit end */
                        byteValue = src[i];
                        dest[i] |= byteValue & endMask;
                        dest[i] &= byteValue | ~endMask;
                    }

                    /* Advance to next line */
                    src  += srcBytesPerLine;
                    dest += destBytesPerLine;
                }
                break;

            case UGL_RASTER_OP_AND:

                /* Over height */
                for (y = 0; y < height; y++) {
                    i = 0;

                    /* Blit start */
                    dest[i] &= src[i] | ~startMask;
                    i++;

                    /* Check if anything to more blit */
                    if (width > 1) {

                        /* Check if bigger blit */
                        if (width > 2) {
                            for (i = 1; i < width - 1; i++) {
                                dest[i] &= src[i];
                            }
                        }

                        /* Blit end */
                        dest[i] &= src[i] | ~endMask;
                    }

                    /* Advance to next line */
                    src  += srcBytesPerLine;
                    dest += destBytesPerLine;
                }
                break;

            case UGL_RASTER_OP_OR:

                /* Over height */
                for (y = 0; y < height; y++) {
                    i = 0;

                    /* Blit start */
                    dest[i] |= src[i] & startMask;
                    i++;

                    /* Check if anything to more blit */
                    if (width > 1) {

                        /* Check if bigger blit */
                        if (width > 2) {
                            for (i = 1; i < width - 1; i++) {
                                dest[i] |= src[i];
                            }
                        }

                        /* Blit end */
                        dest[i] |= src[i] & endMask;
                    }

                    /* Advance to next line */
                    src  += srcBytesPerLine;
                    dest += destBytesPerLine;
                }
                break;

            case UGL_RASTER_OP_XOR:

                /* Over height */
                for (y = 0; y < height; y++) {
                   i = 0;

                   /* Blit start */
                   dest[i] ^= src[i] & startMask;
                   i++;

                   /* Check if anything to more blit */
                   if (width > 1) {

                       /* Check if bigger blit */
                       if (width > 2) {
                           for (i = 1; i < width - 1; i++) {
                               dest[i] ^= src[i];
                           }
                       }

                       /* Blit end */
                       dest[i] ^= src[i] & endMask;
                   }

                   /* Advance to next line */
                   src  += srcBytesPerLine;
                   dest += destBytesPerLine;
               }
               break;

               default:
                   break;
       }
   }
}

/******************************************************************************
 *
 * uglVgaBitmapBlt - Blit from one bitmap memory area to another
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_DDB_ID     srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_VGA_DDB *        pSrcBmp;
    UGL_VGA_DDB *        pDestBmp;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_RECT             destRect;
    UGL_RECT             clipRect;
    const UGL_RECT *     pRect;
    UGL_BLT_DIR          rectOrder;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get gc */
    gc = pDrv->gc;

    /* Start at beginning of clip region list */
    pRect     = UGL_NULL;
    rectOrder = 0;

    if (srcBmpId == UGL_DEFAULT_ID) {
        UGL_RECT_MOVE (*pSrcRect, gc->viewPort.left, gc->viewPort.top);
    }

    if (destBmpId == UGL_DEFAULT_ID) {

        /* Enable clipping region rectnagles if destination is default bitmap */
        if  (pDestPoint->x > pSrcRect->left) {
            rectOrder |= UGL_BLT_RIGHT;
        }

        if (pDestPoint->y > pSrcRect->top) {
            rectOrder |= UGL_BLT_DOWN;
        }

        if (uglClipListSortedGet (gc, &clipRect, &pRect,
                                  rectOrder) != UGL_STATUS_OK) {
            return (UGL_STATUS_OK);
        }

        UGL_POINT_MOVE (*pDestPoint, gc->viewPort.left, gc->viewPort.top);
    }

    do {
        /* Store source and dest */
        pSrcBmp  = (UGL_VGA_DDB *) srcBmpId;
        pDestBmp = (UGL_VGA_DDB *) destBmpId;

        /* Store source rectangle */
        UGL_RECT_COPY (&srcRect, pSrcRect);

        /* Store starting point */
        UGL_POINT_COPY (&destPoint, pDestPoint);

        /* Clip */
        if (uglGenericClipDdbToDdb (devId, &clipRect,
                                    (UGL_BMAP_ID *) &pSrcBmp,
                                    &srcRect,
                                    (UGL_BMAP_ID *) &pDestBmp,
                                    &destPoint) == UGL_TRUE) {

            /* Calculate destination */
            destRect.left = 0;
            destRect.top  = 0;
            UGL_RECT_MOVE_TO_POINT (destRect, destPoint);
            UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH(srcRect),
                              UGL_RECT_HEIGHT(srcRect));

            /* Blit */
            if ((UGL_DDB_ID) pSrcBmp == UGL_DISPLAY_ID) {
                uglVgaBltFrameBufferToColor (devId, &srcRect,
                                             pDestBmp, &destRect);
            }
            else if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
                uglVgaBltColorToFrameBuffer (devId, pSrcBmp, &srcRect,
                                             &destRect);
            }
            else {
                uglVgaBltColorToColor (devId, pSrcBmp, &srcRect,
                                       pDestBmp, &destRect);
            }
        }

        /* Only process clipping region rectangles if default bitmap */
        if (destBmpId != UGL_DEFAULT_ID) {
            break;
        }

    } while (uglClipListSortedGet (gc, &clipRect, &pRect,
                                   rectOrder) == UGL_STATUS_OK);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaBitmapWrite - Write a device independet bitmap to vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_DIB *      pDib,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     ddbId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_VGA_DDB *        pVgaBmp;
    UGL_RECT             srcRect;
    UGL_RECT             destRect;
    UGL_POINT            destPoint;
    UGL_INT32            x;
    UGL_INT32            y;
    UGL_INT32            left;
    UGL_SIZE             planeIndex;
    UGL_SIZE             numPlanes;
    UGL_INT32            destBytesPerLine;
    UGL_INT32            startIndex;
    UGL_INT32            srcOffset;
    UGL_UINT8            startMask;
    UGL_UINT8 **         pPlaneArray;
    UGL_UINT8 *          pSrc;
    UGL_INT32            bpp;
    UGL_INT32            ppb;
    UGL_INT32            nPixels;
    UGL_INT32            shift;
    UGL_INT32            srcMask;
    UGL_INT32            destIndex;
    UGL_INT32            destMask;
    UGL_COLOR            pixel;
    UGL_COLOR            planeMask;
    UGL_INT32            index;
    UGL_COLOR *          pSrcColor;
    UGL_UINT8            mask;
    UGL_RGB              rgb888;
    UGL_INT32            rgb565;
    UGL_COLOR *          pClut;
    UGL_COLOR_FORMAT     colorFormat;
    UGL_VGA_DDB          tmpDdb;
    UGL_RECT             ddbRect;
    UGL_POINT            ddbPoint;
    UGL_RECT             tmpSrcRect;
    UGL_POINT            tmpDestPoint;
    UGL_INT32            planeSize;
    UGL_UINT8 *          pPlaneData;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get device dependent bitmap */
    pVgaBmp = (UGL_VGA_DDB *) ddbId;

    /* Get dimensions */
    srcRect.top    = pSrcRect->top;
    srcRect.bottom = pSrcRect->bottom;
    srcRect.left   = pSrcRect->left;
    srcRect.right  = pSrcRect->right;
    destPoint.x = pDestPoint->x;
    destPoint.y = pDestPoint->y;

    if (uglGenericClipDibToDdb (devId, pDib, &srcRect,
                                (UGL_BMAP_ID *) &pVgaBmp,
                                &destPoint) == UGL_TRUE) {

        /* Calulcate destination dimensions */
        destRect.left = 0;
        destRect.top  = 0;
        UGL_RECT_MOVE_TO_POINT (destRect, destPoint);
        UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                          UGL_RECT_HEIGHT (srcRect));

        /* Precaculate variables */
        numPlanes = devId->pMode->depth;
        srcOffset = (srcRect.top * pDib->stride) + srcRect.left;

        if (ddbId == UGL_DISPLAY_ID) {

            /* Destination is display */
            tmpDdb.header.type   = UGL_DDB_TYPE;
            tmpDdb.header.width  = UGL_RECT_WIDTH (destRect);
            tmpDdb.header.height = 1;
            tmpDdb.stride        = tmpDdb.header.width;
            tmpDdb.shiftValue    = 0;
            planeSize = ((tmpDdb.header.width + 7) / 8 + 1) * pDib->height;
            tmpDdb.pPlaneArray   = (UGL_UINT8 **) uglScratchBufferAlloc (devId,
                                     (planeSize + sizeof (UGL_UINT8)) * 4);
            if (tmpDdb.pPlaneArray == UGL_NULL) {
                return (UGL_STATUS_ERROR);
            }

            pPlaneData = (UGL_UINT8 *) &tmpDdb.pPlaneArray[4];
            for (planeIndex = 0; planeIndex < 4; planeIndex++) {
                tmpDdb.pPlaneArray[planeIndex] = pPlaneData;
                pPlaneData += planeSize;
            }

            colorFormat = pDib->colorFormat;
            pClut = UGL_NULL;
            if (pDib->imageFormat != UGL_DIRECT &&
                pDib->colorFormat != UGL_DEVICE_COLOR_32) {
                pClut = pDib->pClut;
                pDib->pClut = UGL_MALLOC (pDib->clutSize * sizeof (UGL_COLOR));
                if (pDib->pClut != UGL_NULL) {
                    uglVga4BitColorConvert (devId, pClut, colorFormat,
                                            pDib->pClut, UGL_DEVICE_COLOR_32,
                                            pDib->clutSize);
                    pDib->colorFormat = UGL_DEVICE_COLOR_32;
                }
                else {
                    pDib->pClut = pClut;
                    pClut = UGL_NULL;
                }
            }

            UGL_RECT_COPY (&tmpSrcRect, &srcRect);
            tmpSrcRect.bottom = tmpSrcRect.top;

            tmpDestPoint.x = destRect.left;
            tmpDestPoint.y = destRect.top;

            ddbRect.left   = 0;
            ddbRect.top    = 0;
            ddbRect.right  = UGL_RECT_WIDTH (destRect);
            ddbRect.bottom = 0;

            ddbPoint.x = ddbRect.left;
            ddbPoint.y = ddbRect.top;

            while (tmpSrcRect.top <= srcRect.bottom) {
                uglVgaBitmapWrite (devId, pDib, &tmpSrcRect,
                                   (UGL_DDB_ID) &tmpDdb, &ddbPoint);
                uglVgaBitmapBlt (devId, (UGL_DDB_ID) &tmpDdb, &ddbRect,
                                 UGL_DISPLAY_ID, &tmpDestPoint);

                /* Advance scanline */
                tmpSrcRect.top++;
                tmpSrcRect.bottom++;
                tmpDestPoint.y++;
            }

            if (pClut != UGL_NULL) {
                UGL_FREE (pDib->pClut);
                pDib->pClut = pClut;
                pDib->colorFormat = colorFormat;
            }

            uglScratchBufferFree (devId, tmpDdb.pPlaneArray);
        }
        else {

            /* Destination is bitmap */
            left             = destRect.left + pVgaBmp->shiftValue;
            destBytesPerLine = (pVgaBmp->header.width + 7) / 8 + 1;
            pPlaneArray      = pVgaBmp->pPlaneArray;
            startIndex       = destRect.top * destBytesPerLine + (left >> 3);
            startMask        = 0x80 >> (left & 0x07);

            if (pDib->imageFormat == UGL_DIRECT) {

                /* No color lookup table direct color */
                switch (pDib->colorFormat) {
                    case UGL_DEVICE_COLOR_32:
                        pSrcColor = (UGL_COLOR *) pDib->pData + srcRect.top *
                                    pDib->stride;

                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            index = startIndex;
                            mask  = startMask;

                            for (x = srcRect.left; x <= srcRect.right; x++) {
                                pixel     = pSrcColor[x];
                                planeMask = 0x01;

                                for (planeIndex = 0;
                                     planeIndex < numPlanes;
                                     planeIndex++) {
                                    if ((pixel & planeMask) != 0x00) {
                                        pPlaneArray[planeIndex][index] |= mask;
                                    }
                                    else {
                                        pPlaneArray[planeIndex][index] &= ~mask;
                                    }

                                    /* Advance plane */
                                    planeMask <<= 1;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    index++;
                                    mask = 0x80;
                                }
                            }

                            /* Advance row */
                            pSrcColor  += pDib->stride;
                            startIndex += destBytesPerLine;
                        }
                        break;

                    case UGL_ARGB8888:
                    case UGL_RGB565:
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {
                            index = startIndex;
                            mask  = startMask;

                            for (x = srcRect.left; x <= srcRect.right; x++) {
                                if (pDib->colorFormat == UGL_ARGB8888) {
                                    rgb888 = *((UGL_RGB *) pDib->pData +
                                               srcRect.top * pDib->stride + x);
                                }
                                else {
                                    rgb565 = *((UGL_UINT16 *) pDib->pData +
                                               srcRect.top * pDib->stride + x);

                                    rgb888 = ((rgb565 & 0x001f) << 3) |
                                             ((rgb565 & 0x07e0) << 5) |
                                             ((rgb565 & 0xf800) << 8);
                                }

                                uglGenericClutMapNearest(pDrv, &rgb888,
                                                         UGL_NULL, &pixel, 1);

                                planeMask = 0x01;
                                for (planeIndex = 0;
                                     planeIndex < numPlanes;
                                     planeIndex++) {
                                    if ((pixel & planeMask) != 0x00) {
                                        pPlaneArray[planeIndex][index] |= mask;
                                    }
                                    else {
                                        pPlaneArray[planeIndex][index] &= ~mask;
                                    }

                                    /* Advance plane */
                                    planeMask <<= 1;
                                }

                                /* Advance column */
                                if ((mask >>= 1) == 0x00) {
                                    index++;
                                    mask = 0x80;
                                }
                            }

                            /* Advance row */
                            startIndex += destBytesPerLine;
                        }
                        break;

                    default:
                        return (UGL_STATUS_ERROR);
                }
            }
            else {
                pClut = UGL_NULL;

                /* Color lookup table for color data */
                if (pDib->colorFormat != UGL_DEVICE_COLOR_32) {

                    /* Generate color lookup table */
                    pClut = UGL_MALLOC (pDib->clutSize * sizeof (UGL_COLOR));
                    if (pClut == UGL_NULL) {
                        return (UGL_STATUS_ERROR);
                    }

                    /* Convert to 32-bit color */
                    (*devId->colorConvert) (devId, pDib->pClut,
                                            pDib->colorFormat,
                                            pClut, UGL_DEVICE_COLOR_32,
                                            pDib->clutSize);
                }
                else {
                    pClut = pDib->pClut;
                }

                /* Select image format */
                switch(pDib->imageFormat) {
                    case UGL_INDEXED_8:

                        /* Calculate source pointer */
                        pSrc = (UGL_UINT8 *) pDib->pData +
                               srcRect.top * pDib->stride;

                        /* For source height */
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {

                            /* Variable recalculate for each line */
                            destIndex = startIndex;
                            destMask  = startMask;

                            /* For source width */
                            for (x = srcRect.left; x <= srcRect.right; x++) {

                                /* Initialize pixel */
                                pixel = ((UGL_COLOR *) pClut) [pSrc[x]];
                                planeMask = 0x01;

                                /* For each plane */
                                for (planeIndex = 0;
                                     planeIndex < numPlanes;
                                     planeIndex++) {

                                    if ((pixel & planeMask) != 0) {
                                        pPlaneArray[planeIndex][destIndex] |=
                                            destMask;
                                    }
                                    else {
                                        pPlaneArray[planeIndex][destIndex] &=
                                            ~destMask;
                                    }

                                    /* Advance plane mask */
                                    planeMask <<= 1;
                                }

                                /* Advance to next pixel */
                                destMask >>= 1;

                                /* Check if a new byte was reached */
                                if (destMask == 0) {
                                    destIndex++;
                                    destMask = 0x80;
                                }
                            }

                            /* Advance to next line */
                            pSrc       += pDib->stride;
                            startIndex += destBytesPerLine;
                        }
                        break;

                    case UGL_INDEXED_4:
                    case UGL_INDEXED_2:
                    case UGL_INDEXED_1:

                        /* Precalulate pixel vars */
                        bpp = pDib->imageFormat & UGL_INDEX_MASK;
                        ppb = bpp / 8;

                        /* For source height */
                        for (y = srcRect.top; y <= srcRect.bottom; y++) {

                            /* Variable recalculate for each line */
                            pSrc      = (UGL_UINT8 *) pDib->pData +
                                        srcOffset / ppb;
                            nPixels   = srcOffset & ppb;
                            shift     = (ppb - nPixels - 1) * bpp;
                            srcMask   = (0xff >> (8 - bpp)) << shift;
                            destIndex = startIndex;
                            destMask  = startMask;

                            /* For source width */
                            for (x = srcRect.left; x <= srcRect.right; x++) {

                                /* Initialize pixel */
                                pixel     = (*pSrc & srcMask) >> shift;
                                planeMask = 0x01;

                                /* For each plane */
                                for (planeIndex = 0;
                                     planeIndex < numPlanes;
                                     planeIndex++) {

                                    if ((pixel & planeMask) != 0) {
                                        pPlaneArray[planeIndex][destIndex] |=
                                            destMask;
                                    }
                                    else {
                                        pPlaneArray[planeIndex][destIndex] &=
                                            ~destMask;
                                    }

                                    /* Advance plane mask */
                                    planeMask <<= 1;
                                }

                                /* Advance to next pixel */
                                srcMask  >>= bpp;
                                destMask >>= 1;
                                shift     -= bpp;

                                if (srcMask == 0) {
                                    pSrc++;
                                    shift   = (bpp - 1) * bpp;
                                    srcMask = (0xff >> (8 - bpp)) << shift;
                                }

                                if (destMask == 0) {
                                    destIndex++;
                                    destMask = 0x80;
                                }
                            }

                            /* Advance to next line */
                            srcOffset  += pDib->stride;
                            startIndex += destBytesPerLine;
                        }
                        break;

                    default:
                        return (UGL_STATUS_ERROR);
                }

                if (pClut != UGL_NULL) {
                    UGL_FREE (pClut);
                }
            }
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaMonoBitmapCreate - Create vga monocrhome bitmap
 *
 * RETURNS: Pointer to monochrome bitmap
 */

UGL_MDDB_ID uglVgaMonoBitmapCreate (
    UGL_DEVICE_ID       devId,
    UGL_MDIB *          pMdib,
    UGL_DIB_CREATE_MODE createMode,
    UGL_UINT8           initValue,
    UGL_MEM_POOL_ID     poolId
    ) {
    UGL_VGA_DRIVER * pDrv;
    UGL_VGA_MDDB *   pVgaMonoBmp;
    UGL_SIZE         width;
    UGL_SIZE         height;
    UGL_RECT         srcRect;
    UGL_POINT        destPoint;
    UGL_UINT32       bmpSize;
    UGL_UINT8 *      ptr;
    UGL_SIZE         stride;
    UGL_SIZE         planeSize;

    /* Get driver first in device struct */
    pDrv = (UGL_VGA_DRIVER *) devId;

    /* Get bitmap info, from screen if NULL MDIB */
    if (pMdib == UGL_NULL) {
        width  = devId->pMode->width;
        height = devId->pMode->height;
    }
    else {
        width  = pMdib->width;
        height = pMdib->height;
    }

    /* Calcualte stride */
    stride = ((width + 7) / 8 + 1) * 8;

    /* Calculate plane size including 1 shift byte for each scanline */
    planeSize = (stride >> 3) * height;

    /* Calculate size */
    bmpSize = sizeof (UGL_VGA_MDDB) + (4 * sizeof (UGL_UINT8 *)) +
              (2 * planeSize);

    /* Allocate bitmap */
    pVgaMonoBmp = (UGL_VGA_MDDB *) uglOSMemCalloc (poolId, 1, bmpSize);
    if (pVgaMonoBmp == NULL) {
        return (UGL_NULL);
    }

    /* Initialize header */
    pVgaMonoBmp->header.width  = width;
    pVgaMonoBmp->header.height = height;
    pVgaMonoBmp->header.type   = UGL_MDDB_TYPE;
    pVgaMonoBmp->stride        = stride;
    pVgaMonoBmp->shiftValue    = 0;
    pVgaMonoBmp->pPlaneArray   = (UGL_UINT8 **) (((UGL_UINT8 *) pVgaMonoBmp) +
                                 sizeof (UGL_VGA_MDDB));

    /* Initialize plane array */
    ptr = (UGL_UINT8 *) &pVgaMonoBmp->pPlaneArray[4];
    pVgaMonoBmp->pPlaneArray[0] = ptr;
    ptr += planeSize;
    pVgaMonoBmp->pPlaneArray[1] = ptr;
    pVgaMonoBmp->pPlaneArray[2] = UGL_NULL;
    pVgaMonoBmp->pPlaneArray[3] = UGL_NULL;

    /* Intiaialize data */
    switch(createMode) {
        case UGL_DIB_INIT_VALUE:
            memset (pVgaMonoBmp->pPlaneArray[0], initValue, planeSize);
            memset (pVgaMonoBmp->pPlaneArray[1], ~initValue, planeSize);
            break;

        case UGL_DIB_INIT_DATA:
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = width - 1;
            srcRect.bottom = height - 1;
            destPoint.x    = 0;
            destPoint.y    = 0;
            (*devId->monoBitmapWrite) (devId, pMdib, &srcRect,
                                       (UGL_MDDB_ID) pVgaMonoBmp, &destPoint);
            break;

        case UGL_DIB_INIT_NONE:
        default:
            break;
    }

    return (UGL_MDDB_ID) pVgaMonoBmp;
}

/******************************************************************************
 *
 * uglVgaMonoBitmapDestroy - Free monochrome vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaMonoBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_MDDB_ID      mDdbId
    ) {

    /* Free memory */
    uglOSMemFree (mDdbId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaMonoBitmapBltToFramBuffer - Blit from mono-bitmap to framebuffer
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaBltMonoToFrameBuffer (
    UGL_DEVICE_ID  devId,
    UGL_VGA_MDDB * pBmp,
    UGL_RECT *     pSrcRect,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_VGA_DRIVER *     pVgaDrv;
    UGL_GC_ID            gc;
    volatile UGL_UINT8   tmp;
    UGL_INT32            x;
    UGL_INT32            y;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_SIZE             planeIndex;
    UGL_SIZE             numPlanes;
    UGL_INT32            destBytesPerLine;
    UGL_INT32            srcBytesPerLine;
    UGL_INT32            srcOffset;
    UGL_UINT8 *          destStart;
    UGL_UINT8            startMask;
    UGL_UINT8            endMask;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;

    /* Get driver first in device struct */
    pDrv    = (UGL_GENERIC_DRIVER *) devId;
    pVgaDrv = (UGL_VGA_DRIVER *) devId;

    /* Get gc */
    gc = pDrv->gc;

    /* Store height */
    height = UGL_RECT_HEIGHT (*pDestRect);

    /* Align source bitmap to dest */
    uglVgaBltAlign (devId, (UGL_BMAP_HEADER *) pBmp, pSrcRect,
                    UGL_NULL, pDestRect);

    /* Setup variables for blit */
    width            = (pDestRect->right >> 3) - (pDestRect->left >> 3) + 1;
    numPlanes        = devId->pMode->depth;
    destBytesPerLine = pVgaDrv->bytesPerLine;
    srcBytesPerLine  = (pBmp->header.width + 7) / 8 + 1;
    destStart        = (UGL_UINT8 *) pDrv->fbAddress +
                       (pDestRect->top * destBytesPerLine) +
                       (pDestRect->left >> 3);
    srcOffset        = pSrcRect->top * srcBytesPerLine +
                       (pSrcRect->left >> 3);

    /* Generate masks */
    startMask = 0xff >> (pDestRect->left & 0x07);
    endMask   = 0xff << (7 - (pDestRect->right & 0x07));
    if (width == 1) {
        startMask &= endMask;
    }

    /* If draw foreground */
    if (gc->foregroundColor != UGL_COLOR_TRANSPARENT) {

        /* Setup source and destination */
        src = pBmp->pPlaneArray[0] + srcOffset;
        dest = destStart;

        /* Set foreground color */
        UGL_OUT_WORD (0x3ce, (u_int16_t) (gc->foregroundColor << 8));

        /* Set bitmask register */
        UGL_OUT_BYTE (0x3ce, 0x08);
        UGL_OUT_BYTE (0x3cf, startMask);

        /* Blit start */
        for (y = height; y != 0; --y) {
            tmp = *dest;
            *dest = *src;

            /* Advance to next line */
            src  += srcBytesPerLine;
            dest += destBytesPerLine;
        }

        /* Blit middle */
        if (width > 2) {

            /* Set bitmask register */
            UGL_OUT_BYTE (0x3cf, 0xff);

            src = pBmp->pPlaneArray[0] + srcOffset + 1;
            dest = destStart + 1;

            for (y = height; y != 0; --y) {
                for (x = 0; x < width - 2; x++) {
                    tmp = dest[x];
                    dest[x] = src[x];
                }

                /* Advance to next line */
                src  += srcBytesPerLine;
                dest += destBytesPerLine;
            }
        }

        /* Blit end */
        if (width > 1) {
            UGL_OUT_BYTE (0x3cf, endMask);

            src = pBmp->pPlaneArray[0] + srcOffset + width - 1;
            dest = destStart + width - 1;

            for (y = height; y != 0; --y) {
                tmp = *dest;
                *dest = *src;

                /* Advance to next line */
                src  += srcBytesPerLine;
                dest += destBytesPerLine;
            }
        }
    }

    /* If draw background */
    if (gc->backgroundColor != UGL_COLOR_TRANSPARENT) {

        /* Setup source and destination */
        src = pBmp->pPlaneArray[1] + srcOffset;
        dest = destStart;

        /* Set foreground color */
        UGL_OUT_WORD (0x3ce, (u_int16_t) (gc->backgroundColor << 8));

        /* Set bitmask register */
        UGL_OUT_BYTE (0x3ce, 0x08);
        UGL_OUT_BYTE (0x3cf, startMask);

        /* Blit start */
        for (y = height; y != 0; --y) {
            tmp = *dest;
            *dest = *src;

            /* Advance to next line */
            src  += srcBytesPerLine;
            dest += destBytesPerLine;
        }

        /* Blit middle */
        if (width > 2) {

            /* Set bitmask register */
            UGL_OUT_BYTE (0x3cf, 0xff);

            src = pBmp->pPlaneArray[1] + srcOffset + 1;
            dest = destStart + 1;

            for (y = height; y != 0; --y) {
                for (x = 0; x < width - 2; x++) {
                    tmp = dest[x];
                    dest[x] = src[x];
                }

                /* Advance to next line */
                src  += srcBytesPerLine;
                dest += destBytesPerLine;
            }
        }

        /* Blit end */
        if (width > 1) {
            UGL_OUT_BYTE (0x3cf, endMask);

            src = pBmp->pPlaneArray[1] + srcOffset + width - 1;
            dest = destStart + width - 1;

            for (y = height; y != 0; --y) {
                tmp = *dest;
                *dest = *src;

                /* Advance to next line */
                src  += srcBytesPerLine;
                dest += destBytesPerLine;
            }
        }
    }

    /* Restore registers */
    UGL_OUT_BYTE (0x3cf, 0xff);

    /* Restore color */
    UGL_OUT_WORD (0x3ce, ((u_int16_t) pVgaDrv->color << 8));
}

/******************************************************************************
 *
 * uglVgaMonoBitmapBltToColor - Blit from mono-bitmap to memory bitmap
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglVgaBltMonoToColor (
    UGL_DEVICE_ID  devId,
    UGL_VGA_MDDB * pSrcBmp,
    UGL_RECT *     pSrcRect,
    UGL_VGA_DDB *  pDestBmp,
    UGL_RECT *     pDestRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_SIZE             srcStride;
    UGL_SIZE             destStride;
    UGL_COLOR            fg;
    UGL_COLOR            bg;
    UGL_COLOR            colorMask;
    UGL_UINT8 *          src;
    UGL_INT32            plane;
    UGL_INT32            numPlanes;
    UGL_RASTER_OP        rasterOp;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get gc */
    gc = pDrv->gc;

    /* Cache gc attributes */
    fg = gc->foregroundColor;
    bg = gc->backgroundColor;

    /* Calcucalte variables */
    srcStride  = (pSrcBmp->header.width + 7) / 8 + 1;
    destStride = (pDestBmp->header.width + 7) / 8 + 1;
    numPlanes  = pDestBmp->colorDepth;
    colorMask  = 0x01;

    uglVgaBltAlign (devId, (UGL_BMAP_HEADER *) pSrcBmp, pSrcRect,
                    pDestBmp, pDestRect);

    /* Select raster op */
    switch (gc->rasterOp) {
        case UGL_RASTER_OP_COPY:
            if (fg != UGL_COLOR_TRANSPARENT) {
                if (bg != UGL_COLOR_TRANSPARENT) {
                    for (plane = 0; plane < numPlanes; plane++) {
                        if ((fg & colorMask) != 0x00) {
                            if ((bg & colorMask) != 0x00) {
                                src = (UGL_UINT8 *) -1;
                            }
                            else {
                                src = pSrcBmp->pPlaneArray[0];
                            }
                        }
                        else if ((bg & colorMask) != 0x00) {
                            src = pSrcBmp->pPlaneArray[1];
                        }
                        else {
                            src = (UGL_UINT8 *) 0;
                        }

                        /* Draw plane */
                        uglVgaBltPlane (devId, src, pSrcRect,
                                        srcStride, pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride,
                                        UGL_RASTER_OP_COPY);

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
                else {
                    for (plane = 0; plane < numPlanes; plane++) {
                        if ((fg & colorMask) != 0x00) {
                            src = pSrcBmp->pPlaneArray[0];
                            rasterOp = UGL_RASTER_OP_OR;
                        }
                        else {
                            src = pSrcBmp->pPlaneArray[1];
                            rasterOp = UGL_RASTER_OP_AND;
                        }

                        /* Draw plane */
                        uglVgaBltPlane (devId, src, pSrcRect,
                                        srcStride, pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride, rasterOp);

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
            }
            else if (bg != UGL_COLOR_TRANSPARENT) {
                for (plane = 0; plane < numPlanes; plane++) {
                    if ((bg & colorMask) != 0x00) {
                        src = pSrcBmp->pPlaneArray[1];
                        rasterOp = UGL_RASTER_OP_OR;
                    }
                    else {
                        src = pSrcBmp->pPlaneArray[0];
                        rasterOp = UGL_RASTER_OP_AND;
                    }

                    /* Draw plane */
                    uglVgaBltPlane (devId, src, pSrcRect,
                                    srcStride, pDestBmp->pPlaneArray[plane],
                                    pDestRect, destStride, rasterOp);

                    /* Advance */
                    colorMask <<= 1;
                }
            }
            break;

        case UGL_RASTER_OP_AND:
            if (fg != UGL_COLOR_TRANSPARENT) {
                if (bg != UGL_COLOR_TRANSPARENT) {
                    for (plane = 0; plane < numPlanes; plane++) {
                        rasterOp = UGL_RASTER_OP_AND;
                        if ((fg & colorMask) != 0x00) {
                            if ((bg & colorMask) != 0x00) {
                                colorMask <<= 1;
                                continue;
                            }
                            else {
                                src = pSrcBmp->pPlaneArray[0];
                            }
                        }
                        else if ((bg & colorMask) == 0x00) {
                            src = pSrcBmp->pPlaneArray[1];
                        }
                        else {
                            src = (UGL_UINT8 *) 0;
                            rasterOp = UGL_RASTER_OP_COPY;
                        }

                        /* Draw plane */
                        uglVgaBltPlane (devId, src, pSrcRect,
                                        srcStride, pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride, rasterOp);

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
                else {
                    for (plane = 0; plane < numPlanes; plane++) {
                        if ((fg & colorMask) == 0x00) {

                            /* Draw plane */
                            uglVgaBltPlane (devId, pSrcBmp->pPlaneArray[1],
                                            pSrcRect, srcStride,
                                            pDestBmp->pPlaneArray[plane],
                                            pDestRect, destStride,
                                            UGL_RASTER_OP_AND);
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
            }
            else if (bg != UGL_COLOR_TRANSPARENT) {
                for (plane = 0; plane < numPlanes; plane++) {
                    if ((bg & colorMask) == 0x00) {

                        /* Draw plane */
                        uglVgaBltPlane (devId, pSrcBmp->pPlaneArray[0],
                                        pSrcRect, srcStride,
                                        pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride,
                                        UGL_RASTER_OP_AND);
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
            }
            break;

        case UGL_RASTER_OP_OR:
            if (fg != UGL_COLOR_TRANSPARENT) {
                if (bg != UGL_COLOR_TRANSPARENT) {
                    for (plane = 0; plane < numPlanes; plane++) {
                        rasterOp = UGL_RASTER_OP_OR;
                        if ((fg & colorMask) != 0x00) {
                            if ((bg & colorMask) != 0x00) {
                                src = (UGL_UINT8 *) -1;
                                rasterOp = UGL_RASTER_OP_COPY;
                            }
                            else {
                                src = pSrcBmp->pPlaneArray[0];
                            }
                        }
                        else if ((bg & colorMask) != 0x00) {
                            src = pSrcBmp->pPlaneArray[1];
                        }
                        else {
                            colorMask <<= 1;
                            continue;
                        }

                        /* Draw plane */
                        uglVgaBltPlane (devId, src, pSrcRect,
                                        srcStride, pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride, rasterOp);

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
                else {
                    for (plane = 0; plane < numPlanes; plane++) {
                        if ((fg & colorMask) != 0x00) {

                            /* Draw plane */
                            uglVgaBltPlane (devId, pSrcBmp->pPlaneArray[0],
                                            pSrcRect, srcStride,
                                            pDestBmp->pPlaneArray[plane],
                                            pDestRect, destStride,
                                            UGL_RASTER_OP_OR);
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
            }
            else if (bg != UGL_COLOR_TRANSPARENT) {
                for (plane = 0; plane < numPlanes; plane++) {
                    if ((bg & colorMask) != 0x00) {

                        /* Draw plane */
                        uglVgaBltPlane (devId, pSrcBmp->pPlaneArray[1],
                                        pSrcRect, srcStride,
                                        pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride,
                                        UGL_RASTER_OP_OR);
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
            }
            break;

        case UGL_RASTER_OP_XOR:
            if (fg != UGL_COLOR_TRANSPARENT) {
                if (bg != UGL_COLOR_TRANSPARENT) {
                    for (plane = 0; plane < numPlanes; plane++) {
                        if ((fg & colorMask) != 0x00) {
                            if ((bg & colorMask) != 0x00) {
                                src = (UGL_UINT8 *) -1;
                            }
                            else {
                                src = pSrcBmp->pPlaneArray[0];
                            }
                        }
                        else if ((bg & colorMask) != 0x00) {
                            src = pSrcBmp->pPlaneArray[1];
                        }
                        else {
                            src = (UGL_UINT8 *) 0;
                        }

                        /* Draw plane */
                        uglVgaBltPlane (devId, src, pSrcRect,
                                        srcStride, pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride,
                                        UGL_RASTER_OP_XOR);

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
                else {
                    for (plane = 0; plane < numPlanes; plane++) {
                        if ((fg & colorMask) != 0x00) {

                            /* Draw plane */
                            uglVgaBltPlane (devId, pSrcBmp->pPlaneArray[0],
                                            pSrcRect, srcStride,
                                            pDestBmp->pPlaneArray[plane],
                                            pDestRect, destStride,
                                            UGL_RASTER_OP_XOR);
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                }
            }
            else if (bg != UGL_COLOR_TRANSPARENT) {
                for (plane = 0; plane < numPlanes; plane++) {
                    if ((bg & colorMask) != 0x00) {

                        /* Draw plane */
                        uglVgaBltPlane (devId, pSrcBmp->pPlaneArray[1],
                                        pSrcRect, srcStride,
                                        pDestBmp->pPlaneArray[plane],
                                        pDestRect, destStride,
                                        UGL_RASTER_OP_XOR);
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
            }
            break;

        default:
            break;
    }
}

/******************************************************************************
 *
 * uglVgaMonoBitmapBlt - Blit from monochrome bitmap memory area to another
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaMonoBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_MDDB_ID    srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_VGA_MDDB *       pSrcBmp;
    UGL_VGA_DDB *        pDestBmp;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_RECT             destRect;
    UGL_RECT             clipRect;
    const UGL_RECT *     pRect;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get gc */
    gc = pDrv->gc;

    /* Start from the top of the clip region list */
    pRect = UGL_NULL;

    if (destBmpId == UGL_DEFAULT_ID) {

        /* If drawing to default bitmap, enable clip region */
        if (uglClipListGet (gc, &clipRect, &pRect) != UGL_STATUS_OK) {
            return (UGL_STATUS_OK);
        }

        UGL_POINT_MOVE (*pDestPoint, gc->viewPort.left, gc->viewPort.top);
    }

    do {
        /* Store source and dest */
        pSrcBmp  = (UGL_VGA_MDDB *) srcBmpId;
        pDestBmp = (UGL_VGA_DDB *) destBmpId;

        /* Store source rectangle */
        UGL_RECT_COPY (&srcRect, pSrcRect);

        /* Store starting point */
        UGL_POINT_COPY (&destPoint, pDestPoint);

        /* Clip */
        if (uglGenericClipDdbToDdb (devId, &clipRect,
                                    (UGL_BMAP_ID *) &pSrcBmp,
                                    &srcRect,
                                    (UGL_BMAP_ID *) &pDestBmp,
                                    &destPoint) == UGL_TRUE) {

            /* Calculate destination */
            destRect.left = 0;
            destRect.top  = 0;
            UGL_RECT_MOVE_TO_POINT (destRect, destPoint);
            UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH(srcRect),
                              UGL_RECT_HEIGHT(srcRect));

            if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
                uglVgaBltMonoToFrameBuffer (devId, pSrcBmp, &srcRect,
                                            &destRect);
            }
            else {
                 uglVgaBltMonoToColor(devId, pSrcBmp, &srcRect,
                                      pDestBmp, &destRect);
            }
        }

        /* Clip region only enabled when drawing to default bitmap */
        if (destBmpId != UGL_DEFAULT_ID) {
            break;
        }

    } while (uglClipListGet (gc, &clipRect, &pRect) == UGL_STATUS_OK);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaMonoBitmapWrite - Write monochrome dib to monochrome ddb
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaMonoBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_MDIB *     pMdib,
    UGL_RECT *     pSrcRect,
    UGL_MDDB_ID    mDdbId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_VGA_MDDB *       pVgaMonoBmp;
    UGL_UINT8 *          srcStart;
    UGL_RECT             srcRect;
    UGL_POINT            destPoint;
    UGL_RECT             destRect;
    UGL_UINT32           srcIndex;
    UGL_INT32            leftEdge;
    UGL_SIZE             destStride;
    UGL_UINT8 *          destFgStart;
    UGL_UINT8 *          destBgStart;
    UGL_UINT8            dest0Mask;
    UGL_INT32            i;
    UGL_INT32            x;
    UGL_INT32            y;
    UGL_INT32            width;
    UGL_INT32            height;
    UGL_UINT8 *          src;
    UGL_UINT8 *          dest;
    UGL_UINT8            destMask;
    UGL_UINT8            srcMask;
    UGL_UINT8 *          destFg;
    UGL_UINT8 *          destBg;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get device dependent bitmap */
    pVgaMonoBmp = (UGL_VGA_MDDB *) mDdbId;

    /* Setup source data */
    srcStart = (UGL_UINT8 *) pMdib->pData;

    /* Get geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    UGL_POINT_COPY (&destPoint, pDestPoint);

    if (uglGenericClipDibToDdb (devId, (UGL_DIB *) pMdib, &srcRect,
                                (UGL_BMAP_ID *) &pVgaMonoBmp,
                                &destPoint) == UGL_TRUE) {

        /* Calculate destination geometry */
        destRect.left = 0;
        destRect.top  = 0;
        UGL_RECT_MOVE_TO_POINT (destRect, destPoint);
        UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH(srcRect),
                          UGL_RECT_HEIGHT(srcRect));

        /* Calculcate variables */
        width       = UGL_RECT_WIDTH (destRect);
        height      = UGL_RECT_HEIGHT (destRect);
        srcIndex    = (srcRect.top * pMdib->stride) + srcRect.left;
        leftEdge    = destRect.left + pVgaMonoBmp->shiftValue;
        destStride  = (pVgaMonoBmp->header.width + 7) / 8 + 1;
        destFgStart = pVgaMonoBmp->pPlaneArray[0] + destRect.top * destStride +
                      (leftEdge >> 3);
        destBgStart = pVgaMonoBmp->pPlaneArray[1] + destRect.top * destStride +
                      (leftEdge >> 3);
        dest0Mask   = 0x80 >> (leftEdge & 0x07);

        /* Over height */
        for (y = height; y != 0; --y) {
            src      = srcStart + (srcIndex >> 3);
            srcMask  = 0x80 >> (srcIndex & 0x07);
            destFg   = destFgStart;
            destBg   = destBgStart;
            destMask = dest0Mask;

            for (x = width; x != 0; --x) {
                if (srcMask == 0x80 && destMask == 0x80 && x >= 8) {

                    /* Copy foreground */
                    memcpy (destFg, src, x >> 3);
                    destFg += (x >> 3);

                    /* Copy to background as inverted */
                    for (i = x >> 3; i != 0; --i) {
                        *(destBg++) = ~(*src++);
                    }

                    x = (x & 0x07) + 1;
                }
                else
                {
                    if (*src & srcMask) {
                        *destFg |= destMask;
                        *destBg &= ~destMask;
                    }
                    else {
                        *destFg &= ~destMask;
                        *destBg |= destMask;
                    }

                    /* Advance source */
                    if ((srcMask >>= 1) == 0x00) {
                        srcMask = 0x80;
                        src++;
                    }

                    /* Advance destination */
                    if ((destMask >>= 1) == 0x00) {
                        destMask = 0x80;
                        destFg++;
                        destBg++;
                    }
                }
            }

            /* Advance line */
            srcIndex    += pMdib->stride;
            destFgStart += destStride;
            destBgStart += destStride;
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaMonoBitmapRead - Read monochrome ddb to monochrome dib
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaMonoBitmapRead (
    UGL_DEVICE_ID  devId,
    UGL_MDDB_ID    mDdbId,
    UGL_RECT *     pSrcRect,
    UGL_MDIB *     pMdib,
    UGL_POINT *    pDestPoint
    ) {
    UGL_VGA_MDDB * pVgaMonoBmp;
    UGL_RECT       srcRect;
    UGL_POINT      destPoint;
    UGL_INT32      x;
    UGL_INT32      y;
    UGL_INT32      width;
    UGL_INT32      height;
    UGL_INT32      srcIndex;
    UGL_INT32      destIndex;
    UGL_UINT8 *    src;
    UGL_UINT8 *    dest;
    UGL_UINT8      srcMask;
    UGL_UINT8      destMask;
    UGL_SIZE       srcStride;
    UGL_UINT8 *    destStart;

    /* Get device dependent bitmap */
    pVgaMonoBmp = (UGL_VGA_MDDB *) mDdbId;

    /* Setup destination start */
    destStart = (UGL_UINT8 *) pMdib->pData;

    /* Get geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    UGL_POINT_COPY (&destPoint, pDestPoint);

    /* Clip */
    if (uglGenericClipDdbToDib (devId, (UGL_BMAP_ID *) &pVgaMonoBmp,
                                &srcRect, (UGL_DIB *) pMdib,
                                &destPoint) == UGL_TRUE) {
        /* Setup variables */
        width     = UGL_RECT_WIDTH (srcRect);
        height    = UGL_RECT_HEIGHT (srcRect);
        srcStride = ((pVgaMonoBmp->header.width + 7) / 8 + 1) * 8;
        destIndex = destPoint.x;
        srcIndex  = (srcRect.top * pVgaMonoBmp->stride) + srcRect.left +
                    pVgaMonoBmp->shiftValue;

        /* Over height */
        for (y = 0; y < height; y++) {
            src      = pVgaMonoBmp->pPlaneArray[0] + (srcIndex >> 3);
            srcMask  = 0x80 >> (srcIndex & 0x07);
            dest     = destStart + (destIndex >> 3);
            destMask = 0x80 >> (destIndex & 0x07);

            /* Over width */
            for (x = 0; x < width; x++) {
                if (srcMask == 0x00) {
                    srcMask = 0x80;
                    src++;
                }

                if (destMask == 0x00) {
                    destMask = 0x80;
                    dest++;
                }

                if (*src & srcMask) {
                    *dest |= destMask;
                }
                else {
                    *dest &= ~destMask;
                }

                /* Advance column */
                srcMask  >>= 1;
                destMask >>= 1;
            }

            /* Advance row */
            srcIndex  += pVgaMonoBmp->stride;
            destIndex += pMdib->stride;
        }
    }

    return (UGL_STATUS_OK);
}

