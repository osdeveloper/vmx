/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2014 Surplus Users Ham Society
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

/* udg8lin.c - Generic line drawing support for 8-bit displays */

#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcomm.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/graphics/generic/udgen8.h>

/******************************************************************************
 *
 * uglGeneric8BitHLine - Draw horizontal line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitHLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              y,
    UGL_POS              x1,
    UGL_POS              x2,
    UGL_COLOR            color
    ) {
    UGL_GC_ID      gc;
    UGL_RASTER_OP  rasterOp;
    UGL_GEN_DDB *  pDestBmp;
    UGL_UINT8 *    pDest;
    UGL_INT32      stride;
    UGL_INT32      width;

    /* Get gc and attributes */
    gc = pDrv->gc;
    rasterOp = gc->rasterOp;

    /* Get destination bitmap */
    pDestBmp = (UGL_GEN_DDB *) gc->pDefaultBitmap;
    if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
        pDestBmp = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
    }

    /* Setup for horizontal fill */
    width  = x2 - x1 + 1;
    stride = pDestBmp->stride;
    pDest  = (UGL_UINT8 *) pDestBmp->pData + (y * stride) + x1;

    if (pDrv->gpBusy == UGL_TRUE) {
        if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }
    }

    /* Draw line */
    uglCommonMemSet (pDest, 0, width, 8, &color, rasterOp);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitVLine - Draw vertical line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitVLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              x,
    UGL_POS              y1,
    UGL_POS              y2,
    UGL_COLOR            color
    ) {
    UGL_GC_ID      gc;
    UGL_RASTER_OP  rasterOp;
    UGL_UINT8      col;
    UGL_GEN_DDB *  pDestBmp;
    UGL_UINT8 *    pDest;
    UGL_INT32      stride;
    UGL_INT32      height;

    /* Get gc and attributes */
    gc = pDrv->gc;
    rasterOp = gc->rasterOp;
    col = (UGL_UINT8) color;

    /* Get destination bitmap */
    pDestBmp = (UGL_GEN_DDB *) gc->pDefaultBitmap;
    if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
        pDestBmp = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
    }

    /* Setup for vertical fill */
    height = y2 - y1 + 1;
    stride = pDestBmp->stride;
    pDest  = (UGL_UINT8 *) pDestBmp->pData + (y1 * stride) + x;

    if (pDrv->gpBusy == UGL_TRUE) {
        if ((*pDrv->gpWait) (pDrv) != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }
    }

    /* Draw line */
    switch (rasterOp) {
        case UGL_RASTER_OP_COPY:
            while (--height >= 0) {
                *pDest = col;

                /* Advance line */
                pDest += stride;
            }
            break;

        case UGL_RASTER_OP_AND:
            while (--height >= 0) {
                *pDest &= col;

                /* Advance line */
                pDest += stride;
            }
            break;

        case UGL_RASTER_OP_OR:
            while (--height >= 0) {
                *pDest |= col;

                /* Advance line */
                pDest += stride;
            }
            break;

        case UGL_RASTER_OP_XOR:
            while (--height >= 0) {
                *pDest ^= col;

                /* Advance line */
                pDest += stride;
            }
            break;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGeneric8BitBresenhamLine - Draw bresenham line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitBresenhamLine (
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
    UGL_GC_ID      gc;
    UGL_RASTER_OP  rasterOp;
    UGL_COLOR      col;
    UGL_GEN_DDB *  pDestBmp;
    UGL_UINT8 *    pDest;
    UGL_INT32      stride;

    /* Get gc and attributes */
    gc = pDrv->gc;
    rasterOp = gc->rasterOp;
    col = (UGL_UINT8) gc->foregroundColor;

    /* Get destination bitmap */
    pDestBmp = (UGL_GEN_DDB *) gc->pDefaultBitmap;
    if ((UGL_DDB_ID) pDestBmp == UGL_DISPLAY_ID) {
        pDestBmp = (UGL_GEN_DDB *) pDrv->pDrawPage->pDdb;
    }

    /* Setup for line fill */
    stride = pDestBmp->stride;
    pDest  = (UGL_UINT8 *) pDestBmp->pData +
             (pStartPoint->y * stride) + pStartPoint->x;

    if (xMajor) {
        minorInc *= stride;
    }
    else {
        majorInc *= stride;
    }

    /* Draw line */
    while (numPoints-- > 0) {
        switch (rasterOp) {
            case UGL_RASTER_OP_COPY:
                *pDest = col;
                break;

            case UGL_RASTER_OP_AND:
                *pDest &= col;
                break;

            case UGL_RASTER_OP_OR:
                *pDest |= col;
                break;

            case UGL_RASTER_OP_XOR:
                *pDest ^= col;
                break;
        }

        /* Advance */
        if (errorValue >= 0) {
            pDest      += minorInc;
            errorValue += minorErrorInc;
        }

        pDest      += majorInc;
        errorValue += majorErrorInc;
    }

    return (UGL_STATUS_OK);
}

