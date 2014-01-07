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

/* uglpoly.c - Universal graphics library polygon drawing support */

#include "ugl.h"

/******************************************************************************
 *
 * uglPolygon - Draw polygon
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglPolygon (
    UGL_GC_ID  gc,
    UGL_ORD    numPoints,
    UGL_POS *  pData
    ) {
    UGL_STATUS     status;
    UGL_DEVICE_ID  devId;
    UGL_RECT       rect;

    /* Start batch job */
    if ((uglBatchStart (gc)) == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId =  gc->pDriver;

    /* Call driver specific method */
    status = (*devId->polygon) (devId, (const UGL_POINT *) pData, numPoints);

    /* End batch job */
    uglBatchEnd (gc);

    return (status);
}

