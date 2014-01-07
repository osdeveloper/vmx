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

/* udgenfil.c - Universal graphics library fill support */

#include <stdlib.h>

#include "ugl.h"
#include "driver/graphics/generic/udgen.h"

/******************************************************************************
 *
 * uglGenericFill - Generic fill area
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericFill (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              y1,
    UGL_POS              y2,
    UGL_POS **           ppFillData
    ) {
    UGL_GC_ID  gc;
    UGL_COLOR  bg;
    UGL_MDDB * pPatBitmap;
    UGL_SIZE   patWidth;
    UGL_SIZE   patHeight;
    UGL_RECT   clipRect;
    UGL_POS    i;
    UGL_POS    x;
    UGL_POS    y;
    UGL_POS    xStart;
    UGL_POS    xEnd;
    UGL_POS    yStart;
    UGL_POS    yEnd;
    UGL_POS *  pData;
    UGL_ORD    nSegs;
    UGL_POS    srcLeft;
    UGL_POS    srcTop;
    UGL_RECT   srcRect;
    UGL_POINT  destPoint;

    /* Get gc */
    gc = pDrv->gc;

    /* Get attributes */
    bg = gc->backgroundColor;
    pPatBitmap = gc->pPatternBitmap;
    if (pPatBitmap != UGL_NULL) {
        patWidth  = pPatBitmap->width;
        patHeight = pPatBitmap->height;
    }

    UGL_CLIP_LOOP_START (gc, clipRect)

    yStart = y1;
    yEnd   = y2;

    /* Clip */
    if (yStart < clipRect.top) {
        yStart = clipRect.top;
    }

    if (yEnd > clipRect.bottom) {
        yEnd = clipRect.bottom;
    }

    /* Process each scanline */
    for (y = yStart; y <= yEnd; y++) {

        pData = ppFillData[y - y1];
        i = 0;
        nSegs = pData[i++] / 2;

        while (nSegs-- > 0) {
            xStart = pData[i++];
            xEnd   = pData[i++];

            /* Clip */
            if (xStart < clipRect.left) {
                xStart = clipRect.left;
            }

            if (xEnd > clipRect.right) {
                xEnd = clipRect.right;
            }

            /* Fill line */
            if (xEnd >= xStart) {

                if (pPatBitmap == UGL_NULL) {

                    /* Draw solid line */
                    (*pDrv->hLine) (pDrv, y, xStart, xEnd, bg);
                }
                else {

                    /* Draw pattern line */
                    x = xStart;
                    srcLeft = x % patWidth;
                    srcTop  = y % patHeight;

                    if (xEnd / patWidth == xStart / patWidth) {
                        srcRect.left   = srcLeft;
                        srcRect.top    = srcTop;
                        srcRect.right  = xEnd % patWidth;
                        srcRect.bottom = srcTop;
                        destPoint.x = x - gc->viewPort.left;
                        destPoint.y = y - gc->viewPort.top;

                        (*pDrv->ugi.monoBitmapBlt) (&pDrv->ugi,
                                                    pPatBitmap,
                                                    &srcRect,
                                                    UGL_DEFAULT_ID,
                                                    &destPoint);
                    }
                    else {
                        srcRect.left   = srcLeft;
                        srcRect.top    = srcTop;
                        srcRect.right  = patWidth - 1;
                        srcRect.bottom = srcTop;
                        destPoint.x = x - gc->viewPort.left;
                        destPoint.y = y - gc->viewPort.top;

                        (*pDrv->ugi.monoBitmapBlt) (&pDrv->ugi,
                                                    pPatBitmap,
                                                    &srcRect,
                                                    UGL_DEFAULT_ID,
                                                    &destPoint);

                        /* Advance */
                        x += patWidth - srcLeft;
                        srcRect.left = 0;

                        while (xEnd - x + 1 >= patWidth) {
                            destPoint.x = x - gc->viewPort.left;
                            destPoint.y = y - gc->viewPort.top;

                            (*pDrv->ugi.monoBitmapBlt) (&pDrv->ugi,
                                                        pPatBitmap,
                                                        &srcRect,
                                                        UGL_DEFAULT_ID,
                                                        &destPoint);

                            /* Advance */
                            x += patWidth;
                        }

                        srcRect.right = xEnd - x;
                        destPoint.x = x - gc->viewPort.left;
                        destPoint.y = y - gc->viewPort.top;

                        if (x <= xEnd) {
                            (*pDrv->ugi.monoBitmapBlt) (&pDrv->ugi,
                                                        pPatBitmap,
                                                        &srcRect,
                                                        UGL_DEFAULT_ID,
                                                        &destPoint);
                        }
                    }
                }
            }
        }
    }

    UGL_CLIP_LOOP_END

    return (UGL_STATUS_OK);
}

