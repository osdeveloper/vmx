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

/* udvgapix.c - Universal graphics library vga pixel operations */

#include <arch/sysArchLib.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/graphics/vga/udvga.h>

/******************************************************************************
 *
 * uglVgaPixelSet - Set pixel
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaPixelSet (
    UGL_DEVICE_ID  devId,
    UGL_POINT *    p,
    UGL_COLOR      c
    ) {
    UGL_VGA_DRIVER *     pVga;
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GEN_DDB *        pDrawDdb;
    UGL_GC_ID            gc;
    UGL_RECT             clipRect;
    UGL_VGA_DDB *        pBmp;
    UGL_UINT8            mask;
    UGL_UINT8 *          dest;
    UGL_UINT8 **         pPlaneArray;
    volatile UGL_UINT8   tmp;
    UGL_INT32            i;
    UGL_INT32            pixel;
    UGL_INT32            bytesPerLine;
    UGL_INT32            destIndex;
    UGL_UINT8            pixelMask;
    UGL_UINT8            pixelMaskNot;
    UGL_UINT8            colorMask;

    /* Get driver which is first in the device structure */
    pVga = (UGL_VGA_DRIVER *) devId;
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get drawing page */
    pDrawDdb = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;

    /* Get graphics context */
    gc = pDrv->gc;

    /* Move to position within viewport */
    UGL_POINT_MOVE (*p, gc->viewPort.left, gc->viewPort.top);

    /* Start clipping loop */
    UGL_CLIP_LOOP_START (gc, clipRect);

    /* Check if point is within clipping reqion */
    if (UGL_POINT_IN_RECT(*p, clipRect) == UGL_TRUE) {

        /* Check if render to display or bitmap */
        if (gc->pDefaultBitmap == UGL_DISPLAY_ID) {

            /* Calculate destination address */
            dest = ((UGL_UINT8 *) pDrawDdb->pData) +
                   (p->x >> 3) + (p->y * pVga->bytesPerLine);
            mask = 0x80 >> (p->x & 0x07);

            /* Set color register if needed */
            if (c != pVga->color) {
                UGL_OUT_WORD (0x3ce, (UGL_UINT16) (c << 8));
                pVga->color = c;
            }

            /* Set pixel */
            tmp = *dest;
            *dest = mask;
        }
        else {

            /* Setup pointers */
            pBmp        = (UGL_VGA_DDB *) gc->pDefaultBitmap;
            pPlaneArray = (UGL_UINT8 **) pBmp->pPlaneArray;

            /* Setup variables */
            pixel        = p->x + pBmp->shiftValue;
            bytesPerLine = (pBmp->header.width + 7) / 8 + 1;
            destIndex    = p->y * bytesPerLine + (pixel >> 3);
            pixelMask    = 0x80 >> (pixel & 0x07);
            pixelMaskNot = ~pixelMask;
            colorMask    = 0x01;

            /* Select raster op */
            switch(gc->rasterOp) {
                case UGL_RASTER_OP_COPY:

                    /* For each plane */
                    for (i = 0; i < pBmp->colorDepth; ++i) {
                        if ((c & colorMask) != 0) {
                            pPlaneArray[i][destIndex] |= pixelMask;
                        }
                        else {
                            pPlaneArray[i][destIndex] &= pixelMaskNot;
                        }

                        /* Advance plane mask */
                        colorMask <<= 1;
                    }
                    break;

                case UGL_RASTER_OP_AND:

                    /* For each plane */
                    for (i = 0; i < pBmp->colorDepth; ++i) {
                        if ((c & colorMask) == 0) {
                            pPlaneArray[i][destIndex] &= pixelMaskNot;
                        }

                        /* Advance plane mask */
                        colorMask <<= 1;
                    }
                    break;

                case UGL_RASTER_OP_OR:

                    /* For each plane */
                    for (i = 0; i < pBmp->colorDepth; ++i) {
                        if ((c & colorMask) != 0) {
                            pPlaneArray[i][destIndex] |= pixelMask;
                        }

                        /* Advance plane mask */
                        colorMask <<= 1;
                    }
                    break;

                case UGL_RASTER_OP_XOR:

                    /* For each plane */
                    for (i = 0; i < pBmp->colorDepth; ++i) {
                        pPlaneArray[i][destIndex] ^= pixelMask;
                    }
                    break;

                default:
                    return (UGL_STATUS_ERROR);
            }
        }
    }

    /* End clipping loop */
    UGL_CLIP_LOOP_END;

    return (UGL_STATUS_OK);
}

