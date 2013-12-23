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

/* udvgalin.c - Universal graphics library vga line operations */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/vga/udvga.h>

/******************************************************************************
 *
 * uglVgaHLine - Draw horiznotal line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaHLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              y,
    UGL_POS              x1,
    UGL_POS              x2,
    UGL_COLOR            c
    ) {
    UGL_VGA_DRIVER *    pVga;
    UGL_GC_ID           gc;
    UGL_UINT8 *         dest;
    UGL_UINT8           startMask;
    UGL_UINT8           endMask;
    UGL_INT32           byteLen;
    volatile UGL_UINT8  tmp;
    UGL_VGA_DDB *       pVgaBmp;
    UGL_INT32           startIndex;
    UGL_INT32           plane;
    UGL_INT32           numPlanes;
    UGL_UINT8           colorMask;
    UGL_INT32           i;
    UGL_INT32           len;

    /* Get driver which is first in the device structure */
    pVga = (UGL_VGA_DRIVER *) pDrv;

    /* Get graphics context */
    gc = pDrv->gc;

    /* Check if render to display or bitmap */
    if (gc->pDefaultBitmap == UGL_DISPLAY_ID) {

        /* Calculate variables */
        dest = (UGL_UINT8 *) pDrv->fbAddress +
               (y * pVga->bytesPerLine) + (x1 >> 3);
        startMask = 0xff >> (x1 & 0x07);
        endMask = 0xff << (7 - (x2 & 0x07));
        byteLen = (x2 >> 3) - (x1 >> 3) + 1;

        /* Set color register if needed */
        if (c != pVga->color) {
            UGL_OUT_WORD (0x3ce, (UGL_UINT16) (c << 8));
            pVga->color = c;
        }

        /* If line only one byte */
        if (byteLen == 1) {
            tmp = *dest;
            *dest = startMask & endMask;
        }
        else {

            /* Line start */
            tmp = *dest;
            *dest = startMask;
            dest++;

            /* Line middle */
            if (byteLen > 2) {
                if (gc->rasterOp == UGL_RASTER_OP_COPY) {
                    memset (dest, 0xff, byteLen - 2);
                    dest += byteLen - 2;
                }
                else {
                    byteLen--;
                    while (--byteLen) {
                        tmp = *dest;
                        *(dest++) = 0xff;
                    }
                }
            }

            /* Line end */
            tmp = *dest;
            *dest = endMask;
        }
    }
    else {

        /* Destinaion is memory bitmap */
        pVgaBmp = (UGL_VGA_DDB *) gc->pDefaultBitmap;

        /* Adjust for shift */
        x1 += pVgaBmp->shiftValue;
        x2 += pVgaBmp->shiftValue;

        /* Caluclate variables */
        startIndex = y * ((pVgaBmp->header.width + 7) / 8 + 1)+
                     (x1 >> 3);
        startMask  = 0xff >> (x1 & 0x07);
        endMask    = 0xff << (7 - (x2 & 0x07));
        byteLen    = (x2 >> 3) - (x1 >> 3) + 1;
        numPlanes  = pVgaBmp->colorDepth;
        colorMask  = 0x01;

        /* Select raster op */
        switch (gc->rasterOp) {
            case UGL_RASTER_OP_COPY:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;

                    if ((c & colorMask) != 0x00) {
                        if (byteLen == 1) {
                            dest[i] |= (startMask & endMask);
                        }
                        else {

                            /* Line start */
                            dest[i++] |= startMask;

                            /* Line middle */
                            if (byteLen > 2) {
                                memset (&dest[i], 0xff, byteLen - 2);
                                i += byteLen - 2;
                            }

                            /* Line end */
                            dest[i] |= endMask;
                        }
                    }
                    else {
                        if (byteLen == 1) {
                            dest[i] &= ~(startMask & endMask);
                        }
                        else {

                            /* Line start */
                            dest[i++] &= ~startMask;

                            /* Line middle */
                            if (byteLen > 2) {
                                memset (&dest[i], 0x00, byteLen - 2);
                                i += byteLen - 2;
                            }

                            /* Line end */
                            dest[i] &= ~endMask;
                        }
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            case UGL_RASTER_OP_AND:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;

                    if ((c & colorMask) == 0x00) {
                        if (byteLen == 1) {
                            dest[i] &= ~(startMask & endMask);
                        }
                        else {

                            /* Line start */
                            dest[i++] &= ~startMask;

                            /* Line middle */
                            if (byteLen > 2) {
                                memset (&dest[i], 0x00, byteLen - 2);
                                i += byteLen - 2;
                            }

                            /* Line end */
                            dest[i] &= ~endMask;
                        }
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            case UGL_RASTER_OP_OR:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;

                    if ((c & colorMask) != 0x00) {
                        if (byteLen == 1) {
                            dest[i] |= (startMask | endMask);
                        }
                        else {

                            /* Line start */
                            dest[i++] |= startMask;

                            /* Line middle */
                            if (byteLen > 2) {
                                memset (&dest[i], 0xff, byteLen - 2);
                                i += byteLen - 2;
                            }

                            /* Line end */
                            dest[i] |= endMask;
                        }
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            case UGL_RASTER_OP_XOR:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;
                    len = byteLen;

                    if (len == 1) {
                        dest[i] ^= (startMask & endMask);
                    }
                    else {

                        /* Line start */
                        dest[i++] ^= startMask;

                        /* Line middle */
                        if (len > 2) {
                            len--;
                            while (--len) {
                                dest[i++] ^= 0xff;
                            }
                        }

                        /* Line end */
                        dest[i] ^= endMask;
                    }
                }
                break;

            default:
                return (UGL_STATUS_ERROR);
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaVLine - Draw vertical line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaVLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              x,
    UGL_POS              y1,
    UGL_POS              y2,
    UGL_COLOR            c
    ) {
    UGL_VGA_DRIVER *    pVga;
    UGL_GC_ID           gc;
    UGL_SIZE            bytesPerLine;
    UGL_UINT8 *         dest;
    UGL_UINT8           mask;
    UGL_INT32           height;
    volatile UGL_UINT8  tmp;
    UGL_VGA_DDB         *pVgaBmp;
    UGL_UINT8           colorMask;
    UGL_INT32           plane;
    UGL_INT32           numPlanes;
    UGL_INT32           startIndex;
    UGL_INT32           i;
    UGL_INT32           len;

    /* Get driver which is first in the device structure */
    pVga = (UGL_VGA_DRIVER *) pDrv;

    /* Get graphics context */
    gc = pDrv->gc;

    if (gc->pDefaultBitmap == UGL_DISPLAY_ID) {

        /* Caclulate variables */
        bytesPerLine = pVga->bytesPerLine;
        dest         = (UGL_UINT8 *) pDrv->fbAddress +
                       (y1 * bytesPerLine) + (x >> 3);
        mask         = 0x80 >> (x & 0x07);
        height       = y2 - y1 + 1;

        /* Set color register if needed */
        if (c != pVga->color) {
            UGL_OUT_WORD (0x3ce, (UGL_UINT16) (c << 8));
            pVga->color = c;
        }

        /* Draw line */
        while (height--) {
            tmp = *dest;
            *dest = mask;
            dest += bytesPerLine;
        }
    }
    else {
        pVgaBmp = (UGL_VGA_DDB *) gc->pDefaultBitmap;

        /* Calculate variables */
        bytesPerLine = (pVgaBmp->header.width + 7) / 8 + 1;
        numPlanes    = pVgaBmp->colorDepth;

        /* Shift bitmap */
        x += pVgaBmp->shiftValue;

        /* Caluclate variables */
        startIndex = y1 * bytesPerLine + (x >> 3);
        mask       = 0x80 >> (x & 0x07);
        height     = y2 - y1 + 1;
        colorMask  = 0x01;

        /* Select raster op */
        switch (gc->rasterOp) {
            case UGL_RASTER_OP_COPY:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;
                    len = height + 1;

                    if ((c & colorMask) != 0x00) {
                        while (--len) {
                            dest[i] |= mask;
                            i += bytesPerLine;
                        }
                    }
                    else {
                        while (--len) {
                            dest[i] &= ~mask;
                            i += bytesPerLine;
                        }
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            case UGL_RASTER_OP_AND:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;
                    len = height + 1;

                    if ((c & colorMask) == 0x00) {
                        while (--len) {
                            dest[i] &= ~mask;
                            i += bytesPerLine;
                        }
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            case UGL_RASTER_OP_OR:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;
                    len = height + 1;

                    if ((c & colorMask) != 0x00) {
                        while (--len) {
                            dest[i] |= mask;
                            i += bytesPerLine;
                        }
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            case UGL_RASTER_OP_XOR:
                for (plane = 0; plane < numPlanes; plane++) {
                    dest = pVgaBmp->pPlaneArray[plane];
                    i = startIndex;
                    len = height + 1;

                    while (--len) {
                        dest[i] ^= mask;
                        i += bytesPerLine;
                    }

                    /* Advance */
                    colorMask <<= 1;
                }
                break;

            default:
                return (UGL_STATUS_ERROR);
        }
    }
}

/******************************************************************************
 *
 * uglVgaBresenhamLine - Draw bresenham line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaBresenhamLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POINT *          pStartPoint,
    UGL_SIZE             numPoints,
    UGL_BOOL             xMajor,
    UGL_ORD              majorInc,
    UGL_ORD              minorInc,
    UGL_ORD              errorValue,
    UGL_ORD              majorErrorInc,
    UGL_ORD              minorErrorInc
    ) {
    UGL_VGA_DRIVER *    pVga;
    UGL_GC_ID           gc;
    UGL_COLOR           c;
    UGL_SIZE            stride;
    UGL_INT32           xDir;
    UGL_INT32           plane;
    UGL_INT32           numPlanes;
    UGL_UINT8 *         dest;
    UGL_UINT8           colorMask;
    UGL_UINT8           mask;
    UGL_VGA_DDB *       pVgaBmp;
    volatile UGL_UINT8  tmp;
    UGL_INT32           i;

    /* Get driver which is first in the device structure */
    pVga = (UGL_VGA_DRIVER *) pDrv;

    /* Get graphics context */
    gc = pDrv->gc;

    /* Get foreground color */
    c = gc->foregroundColor;

    if (gc->pDefaultBitmap == UGL_DISPLAY_ID) {

        /* Calculate variables */
        stride = pVga->bytesPerLine;
        dest   = (UGL_UINT8 *) pDrv->fbAddress +
                 (pStartPoint->y * stride) + (pStartPoint->x >> 3);
        mask   = 0x80 >> (pStartPoint->x & 0x07);

        if (xMajor == UGL_TRUE) {
            stride *= minorInc;
            xDir = majorInc;
        }
        else {
            stride *= majorInc;
            xDir = minorInc;
        }

        /* Set color register if needed */
        if (c != pVga->color) {
            UGL_OUT_WORD (0x3ce, (UGL_UINT16) (c << 8));
            pVga->color = c;
        }

        /* Draw line */
        while (numPoints-- > 0) {
            tmp = *dest;
            *dest = mask;

            if (errorValue >= 0 || xMajor == UGL_FALSE) {
                dest += stride;
            }

            if (errorValue >= 0 || xMajor == UGL_TRUE) {
                if (xDir == 1) {
                    if ((mask >>= 1) == 0x00) {
                        dest++;
                        mask = 0x80;
                    }
                }
                else {
                    if ((mask <<= 1) == 0x00) {
                        dest--;
                        mask = 0x01;
                    }
                }
            }

            if (errorValue >= 0) {
                errorValue += minorErrorInc;
            }

            errorValue += majorErrorInc;
        }
    }
    else {

        /* Destinaion is memory bitmap */
        pVgaBmp = (UGL_VGA_DDB *) gc->pDefaultBitmap;

        /* Calculate variables */
        stride    = (pVgaBmp->header.width + 7) / 8 + 1;
        numPlanes = pVgaBmp->colorDepth;

        /* Shift coordinate */
        pStartPoint->x += pVgaBmp->shiftValue;

        /* Calculate variables */
        i    = pStartPoint->y * stride + (pStartPoint->x >> 3);
        mask = 0x80 >> (pStartPoint->x & 0x07);

        if (xMajor == UGL_TRUE) {
            stride *= minorInc;
            xDir = majorInc;
        }
        else {
            stride *= majorInc;
            xDir = minorInc;
        }

        /* Draw line */
        while (numPoints-- > 0) {
            colorMask = 0x01;

            /* Select raster op */
            switch (gc->rasterOp) {
                case UGL_RASTER_OP_COPY:
                    for (plane = 0; plane < numPlanes; plane++) {
                        dest = pVgaBmp->pPlaneArray[plane];

                        if ((c & colorMask) != 0) {
                            dest[i] |= mask;
                        }
                        else {
                            dest[i] &= ~mask;
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                    break;

                case UGL_RASTER_OP_AND:
                    for (plane = 0; plane < numPlanes; plane++) {
                        dest = pVgaBmp->pPlaneArray[plane];

                        if ((c & colorMask) == 0) {
                            dest[i] &= ~mask;
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                    break;

                case UGL_RASTER_OP_OR:
                    for (plane = 0; plane < numPlanes; plane++) {
                        dest = pVgaBmp->pPlaneArray[plane];

                        if ((c & colorMask) != 0) {
                            dest[i] |= mask;
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                    break;

                case UGL_RASTER_OP_XOR:
                    for (plane = 0; plane < numPlanes; plane++) {
                        dest = pVgaBmp->pPlaneArray[plane];

                        if ((c & colorMask) != 0) {
                            dest[i] ^= mask;
                        }

                        /* Advance */
                        colorMask <<= 1;
                    }
                    break;

                default:
                    return (UGL_STATUS_ERROR);
            }

            if (errorValue >= 0 || xMajor == UGL_FALSE) {
                i += stride;
            }

            if (errorValue >= 0 || xMajor == UGL_TRUE) {
                if (xDir == 1) {
                    if ((mask >>= 1) == 0x00) {
                        i++;
                        mask = 0x80;
                    }
                }
                else {
                    if ((mask <<= 1) == 0x00) {
                        i--;
                        mask = 0x01;
                    }
                }
            }

            if (errorValue >= 0) {
                errorValue += minorErrorInc;
            }

            errorValue += majorErrorInc;
        }
    }

    return (UGL_STATUS_OK);
}

