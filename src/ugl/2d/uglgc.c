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

/* uglgc.c - Universal graphics library graphics context support */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/******************************************************************************
 *
 * uglGcCreate - Create graphics context
 *
 * RETURNS: Graphics context id or UGL_NULL
 */

UGL_GC_ID uglGcCreate (
    UGL_DEVICE_ID  devId
    ) {
    UGL_GC_ID  gc;
    UGL_POS    width;
    UGL_POS    height;

    if (devId == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Lock device */
    if (uglOsLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Create driver specific gc and clear */
    gc = (*devId->gcCreate) (devId);
    if (gc == UGL_NULL) {
        uglOsUnLock (devId->lockId);
        return (UGL_NULL);
    }

    memset(gc, 0, sizeof (UGL_GC));

    /* Create lock semaphore */
    gc->lockId = uglOsLockCreate ();
    if (gc->lockId == UGL_NULL) {
        uglOsUnLock (devId->lockId);
        return (UGL_NULL);
    }

    /* Get dimensions */
    width  = (UGL_POS) devId->pMode->Width;
    height = (UGL_POS) devId->pMode->Height;

    /* Initialize driver reference */
    gc->pDriver           =       devId;

    /* Set display as default rendering area */
    gc->pDefaultBitmap    =       UGL_DISPLAY_ID;

    /* Initialize bounding rectagle */
    gc->boundRect.left    =       0;
    gc->boundRect.top     =       0;
    gc->boundRect.right   =       width - 1;
    gc->boundRect.bottom  =       height - 1;

    /* Initialize viewport */
    gc->viewPort.left     =       0;
    gc->viewPort.top      =       0;
    gc->viewPort.right    =       width - 1;
    gc->viewPort.bottom   =       height - 1;

    /* Initialize clipping rectagle */
    gc->clipRect.left     =       0;
    gc->clipRect.top      =       0;
    gc->clipRect.right    =       width - 1;
    gc->clipRect.bottom   =       height - 1;

    /* Initialize raster operation */
    gc->rasterOp          =       UGL_RASTER_OP_COPY;

    /* Initialize colors */
    gc->backgroundColor   =       0;
    gc->foregroundColor   =       1;

    /* Unlock */
    uglOsUnLock (devId->lockId);

    return (gc);
}

/******************************************************************************
 *
 * uglGcCopy - Copy graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGcCopy (
    UGL_GC_ID  src,
    UGL_GC_ID  dest
    ) {
    UGL_DEVICE_ID  devId;

    if (src == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock source gc */
    if (uglOsLock (src->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = src->pDriver;

    /* Call driver specific method */
    if ((*devId->gcCopy) (devId, src, dest) != UGL_STATUS_OK) {
        uglOsUnLock (src->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Unlock */
    uglOsUnLock (src->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGcDestroy - Free graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGcDestroy(
    UGL_GC_ID  gc
    ) {
    UGL_DEVICE_ID  devId;

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOsLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = gc->pDriver;

    /* Destroy locking semaphore */
    if (uglOsLockDestroy (gc->lockId) != UGL_STATUS_OK) {
        uglOsUnLock (devId->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Call specific destroy method */
    if ((*devId->gcDestroy) (devId, gc) != UGL_STATUS_OK) {
        uglOsUnLock (devId->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Unlock */
    uglOsUnLock (devId->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGcSet - Set current graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGcSet (
    UGL_DEVICE_ID  devId,
    UGL_GC_ID      gc
    ) {

    /* Lock device */
    if (uglOsLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock source gc */
    if (uglOsLock (gc->lockId) != UGL_STATUS_OK) {
        uglOsUnLock (devId->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Call specific method */
    if ((*devId->gcSet) (devId, gc) != UGL_STATUS_OK) {
        uglOsUnLock (devId->lockId);
        uglOsUnLock (gc->lockId);
        return (UGL_STATUS_ERROR);
    }

    uglOsUnLock (devId->lockId);
    uglOsUnLock (gc->lockId);

    return (UGL_STATUS_OK);
}

