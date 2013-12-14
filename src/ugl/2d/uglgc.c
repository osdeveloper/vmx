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

/* Locals */

LOCAL UGL_UINT32 uglMagicNumber = 1;

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
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Create driver specific gc and clear */
    gc = (*devId->gcCreate) (devId);
    if (gc == UGL_NULL) {
        uglOSUnlock (devId->lockId);
        return (UGL_NULL);
    }

    memset(gc, 0, sizeof (UGL_GC));

    /* Create lock semaphore */
    gc->lockId = uglOSLockCreate ();
    if (gc->lockId == UGL_NULL) {
        uglOSUnlock (devId->lockId);
        return (UGL_NULL);
    }

    /* Get dimensions */
    width  = (UGL_POS) devId->pMode->width;
    height = (UGL_POS) devId->pMode->height;

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

    /* Initialize colors */
    gc->foregroundColor   =       1;
    gc->backgroundColor   =       0;

    /* Initialize raster operation */
    gc->rasterOp          =       UGL_RASTER_OP_COPY;

    /* Mark all attributes as changed */
    gc->changed           = 0xffffffff;

    /* Set context magic number id */
    gc->magicNumber       = uglMagicNumber;

    /* Unlock */
    uglOSUnlock (devId->lockId);

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
    if (uglOSLock (src->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = src->pDriver;

    /* Call driver specific method */
    if ((*devId->gcCopy) (devId, src, dest) != UGL_STATUS_OK) {
        uglOSUnlock (src->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Unlock */
    uglOSUnlock (src->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGcDestroy - Free graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGcDestroy (
    UGL_GC_ID  gc
    ) {
    UGL_DEVICE_ID  devId;

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = gc->pDriver;

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Switch to default GC if destroyed GC is used */
    if ((gc->magicNumber & 0x7fffffff) == (devId->magicNumber & 0x7fffffff)) {
        UGL_GC_SET (devId, devId->defaultGc);
    }

    /* Destroy locking semaphore */
    if (uglOSLockDestroy (gc->lockId) != UGL_STATUS_OK) {
        uglOSUnlock (devId->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Call specific destroy method */
    if ((*devId->gcDestroy) (devId, gc) != UGL_STATUS_OK) {
        uglOSUnlock (devId->lockId);
        return (UGL_STATUS_ERROR);
    }

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglDefaultBitmapSet - Set graphics context default bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglDefaultBitmapSet (
    UGL_GC_ID   gc,
    UGL_DDB_ID  bmpId
    ) {
    UGL_DEVICE_ID  devId;

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    if (bmpId == NULL) {
        bmpId = UGL_DISPLAY_ID;
    }
    else if (bmpId != UGL_DISPLAY_ID && bmpId->type != UGL_DDB_TYPE) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = gc->pDriver;

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Set default bitmap */
    if (gc->pDefaultBitmap != bmpId) {
        gc->pDefaultBitmap = bmpId;

        if (bmpId == UGL_DISPLAY_ID) {
            gc->boundRect.left   = 0;
            gc->boundRect.top    = 0;
            gc->boundRect.right  = devId->pMode->width - 1;
            gc->boundRect.bottom = devId->pMode->height - 1;
        }
        else {
            gc->boundRect.left   = 0;
            gc->boundRect.top    = 0;
            gc->boundRect.right  = bmpId->width - 1;
            gc->boundRect.bottom = bmpId->height - 1;
        }

        /* Update viewport */
        uglViewPortSet (gc, gc->boundRect.left, gc->boundRect.top,
                        gc->boundRect.right, gc->boundRect.bottom);

        gc->changed |= UGL_GC_DEFAULT_BITMAP_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglViewPortSet - Set graphics context viewport
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglViewPortSet (
    UGL_GC_ID  gc,
    UGL_POS    left,
    UGL_POS    top,
    UGL_POS    right,
    UGL_POS    bottom
    ) {

    if (gc != UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Check if trivial and set viewport */
    if (gc->viewPort.left != left || gc->viewPort.top != top ||
        gc->viewPort.right != right || gc->viewPort.bottom != bottom) {

        gc->viewPort.left   = left;
        gc->viewPort.top    = top;
        gc->viewPort.right  = right;
        gc->viewPort.bottom = bottom;
    }

    /* Set clipping rectangle to bounds */
    uglClipRectSet (gc, 0, 0, gc->viewPort.right - gc->viewPort.left,
                    gc->viewPort.bottom - gc->viewPort.top);

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglClipRectSet - Set graphics context clipping rectangle
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClipRectSet (
    UGL_GC_ID  gc,
    UGL_POS    left,
    UGL_POS    top,
    UGL_POS    right,
    UGL_POS    bottom
    ) {
    UGL_DEVICE_ID  devId;
    UGL_RECT       clipRect;

    if (gc != UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get device */
    devId = gc->pDriver;

    /* Setup rectagle */
    clipRect.left   = left;
    clipRect.top    = top;
    clipRect.right  = right;
    clipRect.bottom = bottom;

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Align clip rectangle and store */
    UGL_RECT_MOVE (clipRect, -gc->viewPort.left, -gc->viewPort.top);
    UGL_RECT_COPY (&gc->clipRect, &clipRect);

    /* Mark context field as changed */
    gc->changed |= UGL_GC_CLIP_RECT_CHANGED;
    UGL_GC_CHANGED_SET (gc);

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglForegroundColorSet - Set graphics context foreground color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglForegroundColorSet (
    UGL_GC_ID  gc,
    UGL_COLOR  color
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Set if new color */
    if (gc->foregroundColor != color) {
        gc->foregroundColor  = color;
        gc->changed         |= UGL_GC_FOREGROUND_COLOR_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglBackgroundColorSet - Set graphics context background color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBackgroundColorSet (
    UGL_GC_ID  gc,
    UGL_COLOR  color
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Set if new color */
    if (gc->backgroundColor != color) {
        gc->backgroundColor  = color;
        gc->changed         |= UGL_GC_BACKGROUND_COLOR_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRasterModeSet - Set graphics context raster mode
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRasterModeSet (
    UGL_GC_ID      gc,
    UGL_RASTER_OP  rasterOp
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Set raster op if new */
    if (gc->rasterOp != rasterOp) {
        gc->rasterOp  = rasterOp;
        gc->changed  |= UGL_GC_RASTER_OP_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

