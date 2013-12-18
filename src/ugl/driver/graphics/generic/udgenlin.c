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

/* udgenlin.c - Universal graphics library generic line drawing */

#include <stdlib.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/* Locals */

UGL_LOCAL UGL_STATUS uglLinePointsGet (
    UGL_POINT *  p1,
    UGL_POINT *  p2,
    UGL_POINT ** ppData,
    UGL_INT32    bufSize,
    UGL_ORD *    pNumPoints
    );

UGL_LOCAL UGL_STATUS uglLinePointsGetBoundary (
    UGL_POINT *  p1,
    UGL_POINT *  p2,
    UGL_POINT ** ppData,
    UGL_INT32    bufSize,
    UGL_ORD *    pNumPoints
    );

/******************************************************************************
 *
 * uglGenericLine - Draw a generic line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericLine (
    UGL_DEVICE_ID  devId,
    UGL_POINT *    p1,
    UGL_POINT *    p2
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_LINE_STYLE       lineStyle;
    UGL_SIZE             lineWidth;
    UGL_COLOR            fg;
    UGL_COLOR            bg;
    UGL_POS              x1;
    UGL_POS              x2;
    UGL_POS              y1;
    UGL_POS              y2;
    UGL_POS              coord;
    UGL_POINT            pt1;
    UGL_POINT            pt2;
    UGL_RECT             clipRect;
    UGL_STATUS           status;

    /* Get driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get graphics context */
    gc = pDrv->gc;

    /* Cache variables from gc */
    lineStyle = gc->lineStyle;
    lineWidth = gc->lineWidth;
    fg        = gc->foregroundColor;
    bg        = gc->backgroundColor;

    /* Cache coordinates */
    x1 = p1->x;
    y1 = p1->y;
    x2 = p2->x;
    y2 = p2->y;

    /* Check if anything to do */
    if (lineWidth <= 0 || fg == UGL_COLOR_TRANSPARENT || lineStyle <= 0) {
        return (UGL_STATUS_OK);
    }

    /* Copy geometry */
    UGL_POINT_COPY (&pt1, p1);
    UGL_POINT_COPY (&pt2, p2);

    /* Adjust to viewport */
    UGL_POINT_MOVE (pt1, gc->viewPort.left, gc->viewPort.top);
    UGL_POINT_MOVE (pt2, gc->viewPort.left, gc->viewPort.top);

    /* Check for horizontal and vertical lines */
    if (x1 == x2 || y1 == y2) {
        UGL_CLIP_LOOP_START (gc, clipRect);

        /* Cache coordinates */
        x1 = pt1.x;
        y1 = pt1.y;
        x2 = pt2.x;
        y2 = pt2.y;

        /* If horizontal line */
        if (y1 == y2 && y1 >= clipRect.top && y1 <= clipRect.bottom) {
            if (x1 > x2) {
                coord = x1;
                x1 = x2;
                x2 = coord;
            }

            /* Clip */
            if (x1 < clipRect.left) {
                x1 = clipRect.left;
            }

            if (x2 > clipRect.right) {
                x2 = clipRect.right;
            }

            /* Draw horizontal line */
            if (x2 >= x1) {
                status =  (*pDrv->hLine) (pDrv, y1, x1, x2, fg);
            }
        }
        else if (x1 == x2 && x1 >= clipRect.left && x1 <= clipRect.right) {
            if (y1 > y2) {
                coord = y1;
                y1 = y2;
                y2 = coord;
            }

            /* Clip */
            if (y1 < clipRect.top) {
                y1 = clipRect.top;
            }

            if (y2 > clipRect.bottom) {
                y2 = clipRect.bottom;
            }

            /* Draw vertical line */
            if (y2 >= y1) {
                status = (*pDrv->vLine) (pDrv, x1, y1, y2, fg);
            }
        }

        UGL_CLIP_LOOP_END;
    }
}

/******************************************************************************
 *
 * uglLinePointsGet - Calculate data points for line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglLinePointsGet (
    UGL_POINT *  p1,
    UGL_POINT *  p2,
    UGL_POINT ** ppData,
    UGL_INT32    bufSize,
    UGL_ORD *    pNumPoints
    ) {
    UGL_INT32   minBufSize;
    UGL_POINT * pBuf;
    UGL_INT32   dx;
    UGL_INT32   dy;
    UGL_INT32   ax;
    UGL_INT32   ay;
    UGL_INT32   sx;
    UGL_INT32   sy;
    UGL_INT32   x;
    UGL_INT32   y;
    UGL_INT32   d;

    /* If allocate new buffer */
    minBufSize = max (abs (p2->x - p1->x) + 1, abs (p2->y - p1->y) + 1);

    if (bufSize == -1) {
        pBuf = (UGL_POINT *) UGL_MALLOC (bufSize * sizeof (UGL_POINT));
        if (pBuf == UGL_NULL) {
            return (UGL_STATUS_ERROR);
        }
        *ppData = pBuf;
    }
    else if (bufSize >= minBufSize) {
        pBuf = *ppData;
    }
    else {
        return (UGL_STATUS_ERROR);
    }

    /* Calculate variables */
    dx = p2->x - p1->x;
    dy = p2->y - p1->y;
    ax = abs (dx) << 1;
    ay = abs (dy) << 1;
    sx = (dx > 0) ? 1 : -1;
    dy = (dy > 0) ? 1 : -1;

    x = p1->x;
    y = p1->y;

    if (ax > ay) {
        d = ay - (ax >> 1);
        while (1) {
            pBuf->x = x;
            pBuf->y = y;
            pBuf++;

            if (x == p2->x) {
                break;
            }

            if (d >= 0) {
                y += sy;
                d -= ax;
            }

            x += sx;
            d += ay;
        }
    }
    else {
        d = ax - (ay >> 1);
        while (1) {
            pBuf->x = x;
            pBuf->y = y;
            pBuf++;

            if (y == p2->y) {
                break;
            }

            if (d >= 0) {
                x += sx;
                d -= ay;
            }

            y += sy;
            d += ax;
        }
    }

    if (pNumPoints != UGL_NULL) {
        *pNumPoints = (UGL_ORD) (pBuf - *ppData);
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglLinePointsGetBoundary - Calculate data points bounding line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglLinePointsGetBoundary (
    UGL_POINT *  p1,
    UGL_POINT *  p2,
    UGL_POINT ** ppData,
    UGL_INT32    bufSize,
    UGL_ORD *    pNumPoints
    ) {
    UGL_INT32   minBufSize;
    UGL_POINT * pBuf;
    UGL_INT32   dx;
    UGL_INT32   dy;
    UGL_INT32   ex;
    UGL_INT32   ey;
    UGL_INT32   sx;
    UGL_INT32   sy;
    UGL_INT32   x;
    UGL_INT32   y;
    UGL_INT32   kx;
    UGL_INT32   errorValue;
    UGL_INT32   majorError;
    UGL_INT32   minorError;

    /* Horizontal line */
    if (p1->y == p2->y) {
        if (pNumPoints != UGL_NULL) {
            *pNumPoints = 0;
        }

        return (UGL_STATUS_OK);
    }

    minBufSize = abs (p2->y - p1->y) + 1;

    if (bufSize == -1) {
        pBuf = (UGL_POINT *) UGL_MALLOC (minBufSize * sizeof (UGL_POINT));
        if (pBuf == UGL_NULL) {
            return (UGL_STATUS_ERROR);
        }
        *ppData = pBuf;
    }
    else if (bufSize >= minBufSize) {
        pBuf = *ppData;
    }
    else {
        return (UGL_STATUS_ERROR);
    }

    x = p1->x;
    y = p1->y;

    dx = p2->x - p1->x;
    ex = 2 * dx;
    sx = (dx > 0) ? 1 : -1;

    dy = p2->y - p1->y;
    ey = 2 * dy;
    sy = (dy > 0) ? 1 : -1;

    kx = dx / dy * sy;

    errorValue = -dy * sy;
    majorError = (ex - kx * ey * sy) * sx;
    minorError = ey * sy;

    pBuf->x = x;
    pBuf->y = y;
    pBuf++;

    while (y != p2->y) {
        x += kx;
        y += sy;

        errorValue += majorError;
        if (errorValue >= 0) {
            x += sx;
            errorValue -= minorError;
        }

        pBuf->x = x;
        pBuf->y = y;
        pBuf++;
    }

    if (pNumPoints != UGL_NULL) {
        *pNumPoints = (UGL_ORD) (pBuf - *ppData);
    }

    return (UGL_STATUS_OK);
}

