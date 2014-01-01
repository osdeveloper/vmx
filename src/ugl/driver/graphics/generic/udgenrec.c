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

/* udgenrec.c - Universal graphics library rectangle drawing support */

#include <stdlib.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/* Locals */

UGL_LOCAL UGL_STATUS uglGenericRectFill (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pRect
    );

/******************************************************************************
 *
 * uglGenericRectangle - Generic rectangle drawing
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericRectangle (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_POINT            topLeft;
    UGL_POINT            bottomLeft;
    UGL_POINT            bottomRight;
    UGL_POINT            topRight;
    UGL_POS              posLeft;
    UGL_RECT             fillRect;
    UGL_INT32            adjust;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get graphics context */
    gc = pDrv->gc;

    if (gc->foregroundColor != UGL_COLOR_TRANSPARENT && gc->lineWidth > 0) {

        /* Top left */
        topLeft.x = pRect->left;
        topLeft.y = pRect->top;

        /* Top right */
        topRight.x = pRect->right;
        topRight.y = pRect->top;

        /* Bottom right */
        bottomRight.x = pRect->right;
        bottomRight.y = pRect->bottom;

        /* Bottom left */
        bottomLeft.x = pRect->left;
        bottomLeft.y = pRect->bottom;

        posLeft = topLeft.y;

        (*devId->line) (devId, &topLeft, &topRight);

        if (bottomLeft.y > posLeft) {
            (*devId->line) (devId, &bottomLeft, &bottomRight);
        }

        /* Top left */
        topLeft.x = pRect->left;
        topLeft.y = pRect->top;

        /* Top right */
        topRight.x = pRect->right;
        topRight.y = pRect->top;

        /* Bottom right */
        bottomRight.x = pRect->right;
        bottomRight.y = pRect->bottom;

        /* Bottom left */
        bottomLeft.x = pRect->left;
        bottomLeft.y = pRect->bottom;

        posLeft = bottomLeft.x;

        if (bottomLeft.y > (topLeft.y + 1))  {
            (*devId->line) (devId, &topLeft, &bottomLeft);

            if (posLeft < bottomRight.x) {
                (*devId->line) (devId, &topRight, &bottomRight);
            }
        }
    }

    if (gc->backgroundColor != UGL_COLOR_TRANSPARENT) {
        UGL_RECT_COPY (&fillRect, pRect);

        if (gc->foregroundColor != UGL_COLOR_TRANSPARENT && gc->lineWidth > 0) {
            adjust = (gc->lineWidth + 2) / 2;
            fillRect.left += adjust;
            fillRect.top  += adjust;

            adjust = -(gc->lineWidth + 1) / 2;
            fillRect.right  += adjust;
            fillRect.bottom += adjust;
        }

        uglGenericRectFill (devId, &fillRect);
    }


    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericRectFill - Generic rectangular fill area
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglGenericRectFill (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pRect
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_POS **           ppFillData;
    UGL_POS *            pData;
    UGL_INT32            i;
    UGL_POS              len;
    UGL_STATUS           status;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Calculate data height */
    len = pRect->bottom - pRect->top + 1;

    ppFillData = (UGL_POS **) uglScratchBufferAlloc (devId,
                                                     len * sizeof (UGL_POS *) +
                                                     3 * sizeof (UGL_UINT32));
    if (ppFillData == NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Setup pointer to data */
    pData = (UGL_POS *) &ppFillData[len + 1];
    pData[0] = 2;
    pData[1] = pRect->left;
    pData[2] = pRect->right;

    for (i = 0; i < len; i++) {
        ppFillData[i]  = pData;
    }

    /* Call driver specific fill method */
    status = (*pDrv->fill) (pDrv, pRect->top, pRect->bottom, ppFillData);

    uglScratchBufferFree (devId, ppFillData);

    return (status);
}

