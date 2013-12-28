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

/* uglbatch.c - Universal graphics library batch job support */

#include <ugl/ugl.h>

/******************************************************************************
 *
 * uglBatchStart - Start batch job
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBatchStart (
    UGL_GC_ID  gc
    ) {
    UGL_DEVICE_ID    devId;
    UGL_RECT         clipRect;
    const UGL_RECT * pRect;

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = gc->pDriver;

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* If not nested */
    if (++devId->batchCount == 1) {

        /* Protect cursors */
        if (devId->cursorHide != UGL_NULL &&
            gc->pDefaultBitmap == UGL_DISPLAY_ID) {

            while (uglClipListGet (gc, &clipRect, &pRect) == UGL_STATUS_OK) {
                (*devId->cursorHide) (devId, &clipRect);
            }
        }
    }

    /* Set graphics context as current */
    UGL_GC_SET (devId, gc);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglBatchEnd - End batch job
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBatchEnd (
    UGL_GC_ID  gc
    ) {
    UGL_DEVICE_ID  devId;

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = gc->pDriver;

    /* If end of nesting */
    if (--devId->batchCount == 0) {
        if (devId->cursorShow != UGL_NULL) {
            (*devId->cursorShow) (devId);
        }
    }

    /* Unlock device */
    uglOSLock (devId->lockId);

    /* Unlock GC */
    uglOSLock (gc->lockId);

    return (UGL_STATUS_OK);
}

