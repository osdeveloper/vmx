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

/* udgtbmp.c - Universal graphics library generic polygon drawing support */

#include <stdlib.h>
#include <limits.h>

#include "ugl.h"
#include "driver/graphics/generic/udgen.h"

/* Imports */

UGL_STATUS uglLineDataGetBoundary (
    UGL_POINT *  p1,
    UGL_POINT *  p2,
    UGL_POINT ** ppData,
    UGL_INT32    bufSize,
    UGL_ORD *    pNumPoints
    );

/******************************************************************************
 *
 * uglGenericPolygon - Draw generic polygon
 *
 * RETURNS: UGL_TDDB_ID or UGL_NULL
 */

UGL_STATUS uglGenericPolygon (
    UGL_DEVICE_ID     devId,
    const UGL_POINT * pointArray,
    UGL_ORD           numPoints
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_INT32            i;
    UGL_INT32            j;
    UGL_INT32            k;
    UGL_INT32            bufSize;
    UGL_INT32            fillBufSize;
    UGL_INT32            numLines;
    UGL_ORD              numLinePoints;
    UGL_POS **           ppFillData;
    UGL_POINT *          pLineData;
    UGL_POS *            pScanLine;
    UGL_INT32            xMin;
    UGL_INT32            xMax;
    UGL_INT32            yMin;
    UGL_INT32            yMax;
    UGL_INT32            prevSlope;
    UGL_INT32            currSlope;
    UGL_INT32            nextSlope;
    UGL_INT32            lineIndex;
    UGL_POS              x;
    UGL_POS              y;
    UGL_POS *            pScanData;
    UGL_POS *            pScanLineData;
    UGL_POINT            pt1;
    UGL_POINT            pt2;
    UGL_RECT             clipRect;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get graphics context */
    gc = pDrv->gc;

    if (numPoints < 3) {
        return (UGL_STATUS_ERROR);
    }

    if (gc->backgroundColor != UGL_COLOR_TRANSPARENT) {
        yMin = INT_MAX;
        yMax = INT_MIN;

        /* Clip */
        UGL_RECT_COPY (&clipRect, &gc->clipRect);
        UGL_RECT_MOVE (clipRect, gc->viewPort.left, gc->viewPort.top);

        for (i = 0; i < numPoints; i++) {
            if (pointArray[i].y < yMin) {
                yMin = pointArray[i].y;
            }

            if (pointArray[i].y > yMax) {
                yMax = pointArray[i].y;
            }
        }

        yMin += gc->viewPort.top;
        yMax += gc->viewPort.top;

        if (yMin < clipRect.top) {
            yMin = clipRect.top;
        }

        if (yMax > clipRect.bottom) {
            yMax = clipRect.bottom;
        }

        if (yMax >= yMin) {
            numLines = yMax - yMin + 1;
            fillBufSize = numLines * (sizeof (UGL_POS *) +
                                      sizeof (UGL_POS) * numPoints);
            bufSize = fillBufSize + numLines * sizeof (UGL_POINT);

            /* Allocate scratch buffer */
            ppFillData = (UGL_POS **) uglScratchBufferAlloc (devId, bufSize);
            if (ppFillData == UGL_NULL) {
                return (UGL_STATUS_ERROR);
            }

            pScanLine = (UGL_POS *) &ppFillData [numLines];
            pLineData = (UGL_POINT *) ((char *) ppFillData + fillBufSize);

            /* Initialize fill data */
            for (i = 0; i < numLines; i++) {
                ppFillData[i] = pScanLine;
                ppFillData[i][0] = 0;

                /* Advance */
                pScanLine += numPoints;
            }

            i = numPoints - 1;
            do {
                i--;
                prevSlope = pointArray[i + 1].y - pointArray[i].y;
            } while (prevSlope == 0 && i > 0);

            if (prevSlope > 0) {
                prevSlope = 1;
            }
            else if (prevSlope < 0) {
                prevSlope = -1;
            }

            for (i = 0; i < (numPoints - 1); i++) {
                lineIndex = 0;

                if (pointArray[i].y == pointArray[i + 1].y) {
                    continue;
                }

                UGL_POINT_COPY (&pt1, &pointArray[i]);
                UGL_POINT_MOVE (pt1, gc->viewPort.left, gc->viewPort.top);
                UGL_POINT_COPY (&pt2, &pointArray[i + 1]);
                UGL_POINT_MOVE (pt2, gc->viewPort.left, gc->viewPort.top);

                /* Calculate slope */
                if ((pointArray[i + 1].y - pointArray[i].y) > 0) {
                    currSlope = 1;
                }
                else {
                    currSlope = -1;
                }

                /* Get line segment */
                uglLineDataGetBoundary (&pt1, &pt2, &pLineData,
                                        numLines, &numLinePoints);

                /* Calculate line index */
                if (currSlope == prevSlope) {
                    lineIndex = 1;
                }
                else {
                    lineIndex = 0;
                }

                prevSlope = currSlope;

                while (lineIndex < numLinePoints) {
                    y = pLineData[lineIndex].y;

                    if (y >= yMin && y <= yMax) {
                        pScanData = ppFillData[y - yMin];

                        for (x = 1; x <= pScanData[0]; x++) {
                            if (pLineData[lineIndex].x < pScanData[x]) {
                                pScanData[0]++;
                                memmove (&pScanData[x + 1], &pScanData[x],
                                         (pScanData[0] - x) * sizeof (UGL_POS));
                                pScanData[x] = pLineData[lineIndex].x;
                                break;
                            }
                        }

                        if (x > pScanData[0]) {
                            pScanData[x] = pLineData[lineIndex].x;
                            pScanData[0]++;
                        }
                    }

                    /* Advance */
                    lineIndex++;
                }
            }

            yMin -= gc->viewPort.top;
            yMax -= gc->viewPort.top;

            /* Special case for horizontal lines */
            for (i = 0; i < (numPoints - 1); i++) {
                if (pointArray[i].y == pointArray[i + 1].y &&
                    pointArray[i].y >= yMin && pointArray[i].y < yMax) {

                    /* Calculate previous slope */
                    if (i == 0) {
                        prevSlope = pointArray[numPoints - 1].y -
                                    pointArray[numPoints - 2].y;
                    }
                    else {
                        prevSlope = pointArray[i].y - pointArray[i - 1].y;
                    }

                    if (prevSlope == 0) {
                        continue;
                    }

                    if (prevSlope > 0) {
                        prevSlope = 1;
                    }
                    else if (prevSlope < 0) {
                        prevSlope = -1;
                    }

                    /* Calculate next slope */
                    j = i;
                    nextSlope = 0;
                    while (nextSlope = 0) {
                        if (j == numPoints - 2) {
                            nextSlope = pointArray[1].y - pointArray[0].y;
                        }
                        else {
                            nextSlope = pointArray[j + 2].y -
                                        pointArray[j + 1].y;
                        }

                        if (++j >= numPoints - 1) {
                            j = 0;
                        }

                        if (j == i) {
                            break;
                        }
                    }

                    if (nextSlope > 0) {
                        nextSlope = 1;
                    }
                    else if (nextSlope < 0) {
                        nextSlope = -1;
                    }

                    if (nextSlope == prevSlope) {
                        pScanLineData = ppFillData[pointArray[i].y - yMin];

                        if (pointArray[i].x <= pointArray[j].x) {
                            xMin = pointArray[i].x + gc->viewPort.left;
                            xMax = pointArray[j].x + gc->viewPort.left;
                        }
                        else {
                            xMin = pointArray[j].x + gc->viewPort.left;
                            xMax = pointArray[i].x + gc->viewPort.left;
                        }

                        k = 1;
                        while (k <= pScanLineData[0] &&
                               pScanLineData[k] != xMin &&
                               pScanLineData[k] != xMax) {
                            k++;
                        }

                        if (pScanLineData[k] == xMin) {
                            while (pScanLineData[k + 1] == pScanLineData[k] &&
                                   k < pScanLineData[0]) {
                                k++;
                            }

                            if ((k & 0x01) == 0x00) {
                                pScanLineData[k] = xMax;
                                while (pScanLineData[k + 1] <
                                           pScanLineData[k] &&
                                       k < pScanLineData[0]) {
                                    UGL_INT_SWAP (pScanLineData[k + 1],
                                                  pScanLineData[k]);
                                    k++;
                                }
                            }
                        }
                        else {
                            while (pScanLineData[k - 1] == pScanLineData[k] &&
                                   k > 1) {
                                k--;
                            }

                            if ((k & 0x01) != 0x00) {
                                pScanLineData[k] = xMin;
                                while (k > 1 &&
                                       pScanLineData[k - 1] >
                                       pScanLineData[k]) {
                                    UGL_INT_SWAP (pScanLineData[k - 1],
                                                  pScanLineData[k]);
                                    k--;
                                }
                            }
                        }
                    }
                }
                else {
                    if ((pointArray[i + 1].y - pointArray[i].y) > 0) {
                        prevSlope = 1;
                    }
                    else {
                        prevSlope = -1;
                    }
                }
            }

            /* Fill polygon */
            yMin += gc->viewPort.top;
            yMax += gc->viewPort.top;
            (*pDrv->fill) (pDrv, yMin, yMax, ppFillData);

            /* Free scratch buffer */
            uglScratchBufferFree (devId, ppFillData);
        }
    }

    /* Draw polygon outline */
    if (gc->foregroundColor != UGL_COLOR_TRANSPARENT && gc->lineWidth > 0) {
        for (i = 0; i < (numPoints - 1); i++) {
            UGL_POINT_COPY (&pt1, &pointArray[i]);
            UGL_POINT_COPY (&pt2, &pointArray[i + 1]);
            (*devId->line) (devId, &pt1, &pt2);
        }
    }

    return (UGL_STATUS_OK);
}

