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

/* uglline.c - Universal graphics library line drawing support */

#include <ugl/ugl.h>

/******************************************************************************
 *
 * uglLine - Draw line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglLine (
    UGL_GC_ID  gc,
    UGL_POS    x1,
    UGL_POS    y1,
    UGL_POS    x2,
    UGL_POS    y2
    ) {
    UGL_STATUS     status;
    UGL_DEVICE_ID  devId;
    UGL_POINT      points[2];

    /* Start batch job */
    if ((uglBatchStart (gc)) == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }


    /* Get device */
    devId =  gc->pDriver;

    /* Store points */
    points[0].x = x1;
    points[0].y = y1;
    points[1].x = x2;
    points[1].y = y2;

    /* Call driver specific method */
    status = (*devId->line) (devId, &points[0], &points[1]);

    /* End batch job */
    uglBatchEnd (gc);

    return (status);
}

