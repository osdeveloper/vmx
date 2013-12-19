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
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/******************************************************************************
 *
 * uglGenericRectFill - Generic rectangular fill area
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericRectFill (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_RECT *           pRect
    ) {
    UGL_POS **  pBuf;
    UGL_POS *   pData;
    UGL_INT32   i;
    UGL_POS     height;
    UGL_STATUS  status;

    /* Calculate data height */
    height = pRect->bottom - pRect->top + 1;

    pBuf = (UGL_POS **) uglScratchBufferAlloc ((UGL_DEVICE_ID) pDrv,
                                               height * sizeof (UGL_POS *) +
                                               3 * sizeof (UGL_UINT32));
    if (pBuf == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Setup pointer to data */
    pData = (UGL_POS *) &pBuf[height + 1];
    pData[0] = 1;
    pData[1] = pRect->left;
    pData[2] = pRect->right;

    for (i = 0; i < height; i++) {
        pBuf[i]  = pData;
    }

    /* Call driver specific fill method */
    status = (*pDrv->fill) (pDrv, pRect->top, pRect->bottom, pBuf);

    uglScratchBufferFree ((UGL_DEVICE_ID) pDrv, pBuf);

    return (status);
}

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
    UGL_POS **           fillData
    ) {
    UGL_GC_ID  gc;
    UGL_COLOR  bg;
    UGL_MDDB * pPatBitmap;
    UGL_SIZE   patWidth;
    UGL_SIZE   patHeight;
    UGL_RECT   clipRect;
    UGL_POS    i;
    UGL_POS    y;
    UGL_POS    xStart;
    UGL_POS    xEnd;
    UGL_POS    yStart;
    UGL_POS    yEnd;
    UGL_POS *  pData;
    UGL_ORD    nSegs;

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

        pData = fillData[y - y1];
        i = 0;
        nSegs = pData[i++];

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
            }
        }
    }

    UGL_CLIP_LOOP_END

    return (UGL_STATUS_OK);
}

