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

    if (devId == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Create driver specific gc */
    gc = (*devId->gcCreate) (devId);
    if (gc == UGL_NULL) {
        uglOSUnlock (devId->lockId);
        return (UGL_NULL);
    }

    uglOSUnlock (devId->lockId);

    /* Clear gc structure */
    memset(gc, 0, sizeof (UGL_GC));

    /* Create lock semaphore */
    gc->lockId = uglOSLockCreate ();
    if (gc->lockId == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Initialize driver reference */
    gc->pDriver = devId;

    /* Set display as default rendering area */
    gc->pDefaultBitmap = UGL_DISPLAY_ID;

    /* Initialize bounding rectagle */
    gc->boundRect.left   = 0;
    gc->boundRect.top    = 0;
    gc->boundRect.right  = devId->pMode->width - 1;
    gc->boundRect.bottom = devId->pMode->height - 1;

    /* Initialize viewport */
    UGL_RECT_COPY (&gc->viewPort, &gc->boundRect);

    /* Initialize clipping rectagle */
    UGL_RECT_COPY (&gc->clipRect, &gc->boundRect);

    /* Initialize colors */
    gc->foregroundColor = 1;
    gc->backgroundColor = 0;

    /* Initialize raster operation */
    gc->rasterOp = UGL_RASTER_OP_COPY;

    /* Set line attributes */
    gc->lineWidth = 1;
    gc->lineStyle = UGL_LINE_STYLE_SOLID;

    /* Mark all attributes as changed */
    gc->changed = 0xffffffff;

    /* Set context magic number id */
    gc->magicNumber = uglMagicNumber++;

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
    UGL_RECT       vpRect;

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

        /* Hide cursors */
        if (devId->cursorHide != UGL_NULL &&
            gc->pDefaultBitmap == UGL_DISPLAY_ID &&
            devId->magicNumber == gc->magicNumber &&
            devId->batchCount > 0) {

            UGL_RECT_COPY (&vpRect, &gc->clipRect);
            vpRect.left   += gc->viewPort.left;
            vpRect.top    += gc->viewPort.top;
            vpRect.right  += gc->viewPort.left;
            vpRect.bottom += gc->viewPort.top;

            if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
                return (UGL_STATUS_ERROR);
            }
            (*devId->cursorHide) (devId, &vpRect);
            uglOSUnlock (devId->lockId);
        }
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

        gc->changed |= UGL_GC_VIEW_PORT_CHANGED;
        UGL_GC_CHANGED_SET (gc);
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

    /* Intersect width region and viewport */
    UGL_RECT_MOVE (clipRect, gc->viewPort.left, gc->viewPort.top);
    UGL_RECT_INTERSECT (clipRect, gc->boundRect, clipRect);
    UGL_RECT_INTERSECT (clipRect, gc->viewPort, clipRect);

    /* Hide cursor */
    if (devId->cursorHide != UGL_NULL &&
        gc->pDefaultBitmap == UGL_DISPLAY_ID &&
        devId->magicNumber == gc->magicNumber &&
        devId->batchCount > 0) {

        /* Lock device */
        if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }
        (*devId->cursorHide) (devId, &clipRect);
        uglOSUnlock (devId->lockId);
    }

    /* Restore clip rectangle and store */
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
 * uglClipRegionSet - Set graphics context clipping region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClipRegionSet (
    UGL_GC_ID      gc,
    UGL_REGION_ID  clipRegionId
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Set clipping region */
    gc->clipRegionId = clipRegionId;
    gc->changed |= UGL_GC_CLIP_REGION_CHANGED;
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

/******************************************************************************
 *
 * uglLineWidthSet - Set graphics context line width
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglLineWidthSet (
    UGL_GC_ID  gc,
    UGL_SIZE   lineWidth
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Change line style if needed */
    if (gc->lineWidth != lineWidth) {
        gc->lineWidth = lineWidth;
        gc->changed |= UGL_GC_LINE_WIDTH_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglLineStyleSet - Set graphics context line style
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglLineStyleSet (
    UGL_GC_ID       gc,
    UGL_LINE_STYLE  lineStyle
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Change line style if needed */
    if (gc->lineStyle != lineStyle) {
        gc->lineStyle = lineStyle;
        gc->changed |= UGL_GC_LINE_STYLE_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglFillPatternSet - Set graphics context fill pattern
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglFillPatternSet (
    UGL_GC_ID      gc,
    UGL_MDDB_ID    patternBitmap
    ) {

    if (gc == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock GC */
    if (uglOSLock (gc->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Set fill pattern new */
    if (gc->pPatternBitmap != patternBitmap) {
        gc->pPatternBitmap = patternBitmap;
        gc->changed |= UGL_GC_PATTERN_BITMAP_CHANGED;
        UGL_GC_CHANGED_SET (gc);
    }

    /* Unlock */
    uglOSUnlock (gc->lockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglClipListSortedGet - Get sorted clip rectangles from graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClipListSortedGet (
    UGL_GC_ID         gc,
    UGL_RECT *        pRect,
    const UGL_RECT ** ppRect,
    UGL_BLT_DIR       rectOrder
    ) {
    UGL_STATUS  status = UGL_STATUS_OK;

    /* If reset traversal */
    if (pRect == UGL_NULL || ppRect == UGL_NULL) {
        if (gc->clipRegionId != UGL_NULL) {
            status = uglRegionRectSortedGet (gc->clipRegionId, UGL_NULL,
                                             rectOrder);
            return (status);
        }
    }

    /* If no clipping region set */
    if (gc->clipRegionId == UGL_NULL) {

        /* If get ordinary clipping rectangle */
        if (*ppRect == UGL_NULL) {
            *pRect = gc->clipRect;
            *ppRect = &gc->clipRect;
        }
        else {
            *ppRect = UGL_NULL;
            return (UGL_STATUS_FINISHED);
        }
    }
    else {
        do {
            status = uglRegionRectSortedGet (gc->clipRegionId, ppRect,
                                             rectOrder);
            if (status != UGL_STATUS_OK || ppRect == UGL_NULL ||
                *ppRect == UGL_NULL) {
                return (status);
            }

            /* Intersect with ordinary clipping rectangle */
            UGL_RECT_INTERSECT (**ppRect, gc->clipRect, *pRect);
        } while (UGL_RECT_WIDTH (*pRect) <= 0 || UGL_RECT_HEIGHT (*pRect) <= 0);
    }

    /* Move according to viewport */
    UGL_RECT_MOVE (*pRect, gc->viewPort.left, gc->viewPort.top);

    return (status);
}

/******************************************************************************
 *
 * uglClipListGet - Get clip rectangles for graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClipListGet (
    UGL_GC_ID         gc,
    UGL_RECT *        pRect,
    const UGL_RECT ** ppRect
    ) {

    return uglClipListSortedGet (gc, pRect, ppRect, UGL_TR2BL);
}

