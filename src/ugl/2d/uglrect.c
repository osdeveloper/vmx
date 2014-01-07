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

/* uglrect.c - Universal graphics library rectangle drawing support */

#include "ugl.h"

/******************************************************************************
 *
 * uglRectangle - Draw rectangle
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRectangle (
    UGL_GC_ID  gc,
    UGL_POS    left,
    UGL_POS    top,
    UGL_POS    right,
    UGL_POS    bottom
    ) {
    UGL_STATUS     status;
    UGL_DEVICE_ID  devId;
    UGL_RECT       rect;

    /* Check size */
    if (right < left || bottom < top) {
        return (UGL_STATUS_OK);
    }

    /* Start batch job */
    if ((uglBatchStart (gc)) == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId =  gc->pDriver;

    /* Store points */
    rect.left   = left;
    rect.top    = top;
    rect.right  = right;
    rect.bottom = bottom;

    /* Call driver specific method */
    status = (*devId->rectangle) (devId, &rect);

    /* End batch job */
    uglBatchEnd (gc);

    return (status);
}

