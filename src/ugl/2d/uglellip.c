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

/* uglellip.c - Ellipse drawing support for Universal Graphics Library */

#include "ugl.h"

/******************************************************************************
 *
 * uglEllipse - Draw ellipse
 *
 * RETURNS: N/A
 */

UGL_STATUS uglEllipse (
    UGL_GC_ID  gc,
    UGL_POS    left,
    UGL_POS    top,
    UGL_POS    right,
    UGL_POS    bottom,
    UGL_POS    startX,
    UGL_POS    startY,
    UGL_POS    endX,
    UGL_POS    endY
    ) {
    UGL_DEVICE_ID  devId;
    UGL_RECT       boundRect;
    UGL_POINT      startArc;
    UGL_POINT      endArc;
    UGL_STATUS     status;

    /* Check if trivial */
    if (right < left || bottom < top) {
        return (UGL_STATUS_OK);
    }

    /* Start batch job */
    if ((uglBatchStart (gc)) == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId =  gc->pDriver;

    /* Setup geometry */
    boundRect.left   = left;
    boundRect.top    = top;
    boundRect.right  = right;
    boundRect.bottom = bottom;
    startArc.x = startX;
    startArc.y = startY;
    endArc.x = endX;
    endArc.y = endY;

    /* Call driver specific method */
    status = (*devId->ellipse) (devId, &boundRect, &startArc, &endArc);

    /* End batch job */
    uglBatchEnd (gc);

    return (status);
}

