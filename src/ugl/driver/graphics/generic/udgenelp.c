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

/* udgenelp.c - Generic ellipse support for Universal Graphics Library */

/* Includes */

#include <stdlib.h>
#include <limits.h>

#include "ugl.h"
#include "driver/graphics/generic/udgen.h"

/* Defines */

#define MAX_TOGGLE_POINTS     7

/* Types */

typedef struct ugl_ellipse_data {
    UGL_GENERIC_DRIVER  * pDrv;
    UGL_SIZE              lineWidth;

    UGL_POS **            ppFillData;
    UGL_POS *             pFillBuf;
    UGL_UINT32            fillDataSize;
    UGL_UINT32            fillBufSize;

    UGL_POINT *           pLineData;
    UGL_UINT32            lineDataSize;

    UGL_POS **            ppExtData;
    UGL_POS *             pExtBuf;
    UGL_UINT32            extDataSize;
    UGL_UINT32            extBufSize;

    UGL_POS               left;
    UGL_POS               top;
    UGL_POS               right;
    UGL_POS               bottom;

    UGL_POS               x;
    UGL_POS               y;
    UGL_POS               startX;
    UGL_POS               startY;
    UGL_POS               endX;
    UGL_POS               endY;

    UGL_INT32             innerStartX;
    UGL_INT32             innerEndX;
    UGL_INT32             innerStartY;
    UGL_INT32             innerEndY;

    UGL_INT32             outerStartX;
    UGL_INT32             outerEndX;
    UGL_INT32             outerStartY;
    UGL_INT32             outerEndY;
} UGL_ELLIPSE_DATA;

/* Macros */

#define SIGN(A)               (((A) < 0) ? -1 : 1)
#define MATH_TO_SCRX(E, X)    ((X) + (E)->x)
#define MATH_TO_SCRY(E, Y)    ((E)->y - (Y))
#define SCR_TO_MATHX(E, X)    ((X) - (E)->x)
#define SCR_TO_MATHY(E, Y)    ((E)->y - (Y))
#define MATH_TO_ARRAYY(E, Y)  ((((E)->bottom - (E)->top) >> 1) - (Y))
#define SRC_TO_ARRAYY(E, Y)   ((Y) - ((E)->lineWidth >> 1))

/* Locals */

UGL_LOCAL UGL_STATUS uglEllipseAlloc (
    UGL_ELLIPSE_DATA *  pEd
    );

UGL_LOCAL UGL_VOID uglEllipseGet (
    UGL_ELLIPSE_DATA *  pEd,
    UGL_BOOL            inner
    );

UGL_LOCAL UGL_INT32 uglEllipseIntersectStart (
    UGL_ELLIPSE_DATA *  pEd,
    UGL_INT32           x,
    UGL_INT32           y,
    UGL_INT32           minError,
    UGL_BOOL            inner
    );

UGL_LOCAL UGL_INT32 uglEllipseIntersectEnd (
    UGL_ELLIPSE_DATA *  pEd,
    UGL_INT32           x,
    UGL_INT32           y,
    UGL_INT32           minError,
    UGL_BOOL            inner
    );

UGL_LOCAL UGL_VOID uglEllpiseFlatFix (
    UGL_ELLIPSE_DATA * pEd
    );

/******************************************************************************
 *
 * uglEllipseAlloc - Allocate memory for ellipse data
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglEllipseAlloc (
    UGL_ELLIPSE_DATA *  pEd
    ) {
    UGL_INT32   a;
    UGL_INT32   b;
    UGL_INT32   totalSize;
    UGL_UINT8 * pData;

    a = (pEd->right - pEd->left) / 2;
    b = (pEd->bottom - pEd->top) / 2;

    pEd->fillDataSize = (pEd->bottom - pEd->top + 1 + pEd->lineWidth + 2) *
                        sizeof (UGL_POS *);
    pEd->extDataSize  = (pEd->bottom - pEd->top + 1 + pEd->lineWidth + 2) *
                        sizeof (UGL_POS *);
    pEd->fillBufSize  = (pEd->bottom - pEd->top + 1 + pEd->lineWidth + 2) *
                        (MAX_TOGGLE_POINTS + 1) * sizeof (UGL_POS);
    pEd->extBufSize   = (pEd->bottom - pEd->top + 1 + pEd->lineWidth + 2) *
                        (MAX_TOGGLE_POINTS + 1) * sizeof (UGL_POS);
    pEd->lineDataSize = max (a + (pEd->lineWidth >> 1) + 1,
                             b + (pEd->lineWidth >> 1) + 1) *
                        sizeof (UGL_POINT);

    totalSize = pEd->fillDataSize + pEd->fillBufSize +
                pEd->extDataSize + pEd->extBufSize + pEd->lineDataSize;
    pData = (UGL_UINT8 *) uglScratchBufferAlloc ((UGL_DEVICE_ID) pEd->pDrv,
                                                 totalSize);
    if (pData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    memset (pData, 0, totalSize);

    pEd->ppFillData = (UGL_POS **) pData;
    pData += pEd->fillDataSize;
    pEd->pFillBuf = (UGL_POS *) pData;
    pData += pEd->fillBufSize;
    pEd->ppExtData = (UGL_POS **) pData;
    pData += pEd->extDataSize;
    pEd->pExtBuf = (UGL_POS *) pData;
    pData += pEd->extBufSize;
    pEd->pLineData = (UGL_POINT *) pData;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglEllipseGet - Get ellipse data using Bresenham Algorithm
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglEllipseGet (
    UGL_ELLIPSE_DATA *  pEd,
    UGL_BOOL            inner
    ) {
    UGL_INT32    x;
    UGL_INT32    y;
    UGL_INT32    d;
    UGL_INT32    xRefl;
    UGL_INT32    yRefl;
    UGL_INT32    a;
    UGL_INT32    b;
    UGL_INT32 *  pData;
    UGL_INT32 ** ppData;
    UGL_INT32    xMax;
    UGL_INT32    minStartError;
    UGL_INT32    minEndError;

    x = 0;
    xMax = INT_MIN;

    xRefl = (pEd->right - pEd->left) & 1;
    yRefl = (pEd->bottom - pEd->top) & 1;

    if (inner == UGL_TRUE) {
        a = ((pEd->right - pEd->left) >> 1) -
             (pEd->lineWidth >> 1) - (pEd->lineWidth & 1);
        b = ((pEd->bottom - pEd->top) >> 1) -
             (pEd->lineWidth >> 1) - (pEd->lineWidth & 1);
        y = b;
        ppData = pEd->ppFillData;

        pEd->innerStartX = MATH_TO_SCRX (pEd, 0);
        pEd->innerStartY = MATH_TO_SCRY (pEd, 0);
        pEd->innerEndX   = MATH_TO_SCRX (pEd, 0);
        pEd->innerEndY   = MATH_TO_SCRY (pEd, 0);
    }
    else {
        a = ((pEd->right - pEd->left) >> 1) - (pEd->lineWidth >> 1);
        b = ((pEd->bottom - pEd->top) >> 1) - (pEd->lineWidth >> 1);
        y = b;
        ppData = pEd->ppExtData;

        pEd->outerStartX = MATH_TO_SCRX (pEd, 0);
        pEd->outerStartY = MATH_TO_SCRY (pEd, 0);
        pEd->outerEndX   = MATH_TO_SCRX (pEd, 0);
        pEd->outerEndY   = MATH_TO_SCRY (pEd, 0);
    }

    if ((a >= 0) && (b >= 0)) {
        if ((pEd->startX == pEd->endX) && (pEd->startY == pEd->endY)) {
            minStartError = 0;
            minEndError   = 0;
        }
        else {
            minStartError = INT_MAX;
            minEndError   = INT_MAX;
        }

        d = a * a - 4 * b * (a * a - b);
        while (a * a * (2 * y - 1) > 2 * b * b * (x + 1)) {
            if (minStartError > 0) {
                minStartError = uglEllipseIntersectStart (pEd, x, y,
                                                          minStartError, inner);
            }

            if (minEndError > 0) {
                minEndError = uglEllipseIntersectStart (pEd, x, y,
                                                        minEndError, inner);
            }

            if (x > xMax) {
                xMax = x;
            }

            if (d < 0) {
                d += b * b * (8 * x + 12);
            }
            else {
                pData = ppData[MATH_TO_ARRAYY (pEd, y) + (pEd->lineWidth >> 1)];
                pData[0] = 2;
                pData[1] = MATH_TO_SCRX (pEd, -xMax);
                pData[2] = MATH_TO_SCRX (pEd, xMax + xRefl);

                pData = ppData[MATH_TO_ARRAYY (pEd, -(y + yRefl)) +
                               (pEd->lineWidth >> 1)];
                pData[0] = 2;
                pData[1] = MATH_TO_SCRX (pEd, -xMax);
                pData[2] = MATH_TO_SCRX (pEd, xMax + xRefl);

                xMax = INT_MIN;
                d += (b * b * (8 * x + 12) + a * a * (-8 * y + 8));
                y--;
            }

            x++;
        }

        xMax = INT_MIN;
        d = b * b * (2 * x + 1) * (2 * x + 1) + 4 * a * a * (y - 1) * (y - 1) -
            4 * a * a* b * b;
        while (y >= 0) {
            if (minStartError > 0) {
                minStartError = uglEllipseIntersectStart (pEd, x, y,
                                                          minStartError, inner);
            }

            if (minEndError > 0) {
                minEndError = uglEllipseIntersectStart (pEd, x, y,
                                                        minEndError, inner);
            }

            pData = ppData[MATH_TO_ARRAYY (pEd, y) + (pEd->lineWidth >> 1)];
            pData[0] = 2;
            pData[1] = MATH_TO_SCRX (pEd, -x);
            pData[2] = MATH_TO_SCRX (pEd, x + xRefl);

            pData = ppData[MATH_TO_ARRAYY (pEd, -(y + xRefl)) +
                           (pEd->lineWidth >> 1)];
            pData[0] = 2;
            pData[1] = MATH_TO_SCRX (pEd, -x);
            pData[2] = MATH_TO_SCRX (pEd, x + xRefl);

            if (d < 0) {
                d += 8 * b * b * (x + 1) + a * a * (-8 * y + 12);
                x++;
            }
            else {
                d += a * a * (-8 * y + 12);
            }

            y--;
        }
    }
}

/******************************************************************************
 *
 * uglEllipseIntersectStart - Calculate intersection points of ellipse
 *
 * RETURNS: Minimum error
 */

UGL_LOCAL UGL_INT32 uglEllipseIntersectStart (
    UGL_ELLIPSE_DATA *  pEd,
    UGL_INT32           x,
    UGL_INT32           y,
    UGL_INT32           minError,
    UGL_BOOL            inner
    ) {
    UGL_INT32  errorValue;
    UGL_INT32  xRefl;
    UGL_INT32  yRefl;

    xRefl = (pEd->right - pEd->left) & 1;
    yRefl = (pEd->bottom - pEd->top) & 1;

    x = (SIGN (SCR_TO_MATHX (pEd, pEd->startX)) == -1) ? -x : x + xRefl;
    y = (SIGN (SCR_TO_MATHY (pEd, pEd->startY)) == -1) ? -y - yRefl : y;

    errorValue = abs (SCR_TO_MATHX (pEd, pEd->startX) * y -
                      SCR_TO_MATHY (pEd, pEd->startY) * x);
    if (errorValue < minError) {
        if (inner == UGL_TRUE) {
            pEd->innerStartX = MATH_TO_SCRX (pEd, x);
            pEd->innerStartY = MATH_TO_SCRY (pEd, y);
        }
        else {
            pEd->outerStartX = MATH_TO_SCRX (pEd, x);
            pEd->outerStartY = MATH_TO_SCRY (pEd, y);
        }

        minError = errorValue;
    }

    return (minError);
}

/******************************************************************************
 *
 * uglEllipseIntersectEnd - Calculate intersection points of ellipse
 *
 * RETURNS: Minimum error
 */

UGL_LOCAL UGL_INT32 uglEllipseIntersectEnd (
    UGL_ELLIPSE_DATA *  pEd,
    UGL_INT32           x,
    UGL_INT32           y,
    UGL_INT32           minError,
    UGL_BOOL            inner
    ) {
    UGL_INT32  errorValue;
    UGL_INT32  xRefl;
    UGL_INT32  yRefl;

    xRefl = (pEd->right - pEd->left) & 1;
    yRefl = (pEd->bottom - pEd->top) & 1;

    x = (SIGN (SCR_TO_MATHX (pEd, pEd->endX)) == -1) ? -x : x + xRefl;
    y = (SIGN (SCR_TO_MATHY (pEd, pEd->endY)) == -1) ? -y - yRefl: y;

    errorValue = abs (SCR_TO_MATHX (pEd, pEd->endX) * y -
                      SCR_TO_MATHY (pEd, pEd->endY) * x);
    if (errorValue < minError) {
        if (inner == UGL_TRUE) {
            pEd->innerEndX = MATH_TO_SCRX (pEd, x);
            pEd->innerEndY = MATH_TO_SCRY (pEd, y);
        }
        else {
            pEd->outerEndX = MATH_TO_SCRX (pEd, x);
            pEd->outerEndY = MATH_TO_SCRY (pEd, y);
        }

        minError = errorValue;
    }

    return (minError);
}

/******************************************************************************
 *
 * uglEllipseFlatFix - Fix flat ellipse
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglEllpiseFlatFix (
    UGL_ELLIPSE_DATA * pEd
    ) {
    UGL_INT32  i;

    for (i = 1; i <= pEd->bottom - pEd->top + (pEd->lineWidth & ~1) - 1; i++) {
        if (pEd->ppFillData[i][0] > 0) {
            if (i < SRC_TO_ARRAYY (pEd, pEd->y)) {
                if (pEd->ppExtData[i - 1][0] > 0) {
                    if (pEd->ppFillData[i][1] < pEd->ppExtData[i - 1][1]) {
                        pEd->ppFillData[i][1] = pEd->ppExtData[i - 1][1];
                    }

                    if (pEd->ppFillData[i][2] < pEd->ppExtData[i - 1][2]) {
                        pEd->ppFillData[i][2] = pEd->ppExtData[i - 1][2];
                    }
                }
            }
            else if (pEd->ppExtData[i + 1][0] > 0) {
                if (pEd->ppFillData[i][1] < pEd->ppExtData[i + 1][1]) {
                    pEd->ppFillData[i][1] = pEd->ppExtData[i + 1][1];
                }

                if (pEd->ppFillData[i][2] < pEd->ppExtData[i + 1][2]) {
                    pEd->ppFillData[i][2] = pEd->ppExtData[i + 1][2];
                }
            }
        }
    }
}

