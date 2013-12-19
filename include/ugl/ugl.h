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

/* ugl.h - Universal graphics library header */

#ifndef _ugl_h
#define _ugl_h

#include <ugl/ugltypes.h>
#include <ugl/uglclr.h>
#include <ugl/uglinfo.h>
#include <ugl/uglmode.h>
#include <ugl/ugldib.h>
#include <ugl/uglos.h>
#include <ugl/uglmem.h>
#include <ugl/uglregn.h>
#include <ugl/uglugi.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Mode support functions */

/******************************************************************************
 *
 * uglModeAvailGet - Get avilable graphics modes
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglModeAvailGet (
    UGL_DEVICE_ID     devId,
    UGL_UINT32 *      pNumModes,
    const UGL_MODE ** pModeArray
    );

/******************************************************************************
 *
 * uglModeSet - Set graphics modes
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglModeSet (
    UGL_DEVICE_ID  devId,
    UGL_MODE *     pMode
    );

/* Graphics context support functions */

/******************************************************************************
 *
 * uglGcCreate - Create graphics context
 *
 * RETURNS: Graphics context id or UGL_NULL
 */

UGL_GC_ID uglGcCreate (
    UGL_DEVICE_ID  devId
    );

/******************************************************************************
 *
 * uglGcCopy - Copy graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGcCopy (
    UGL_GC_ID  src,
    UGL_GC_ID  dest
    );

/******************************************************************************
 *
 * uglGcDestroy - Free graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGcDestroy (
    UGL_GC_ID  gc
    );

/******************************************************************************
 *
 * uglDefaultBitmapSet - Set graphics context default bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglDefaultBitmapSet (
    UGL_GC_ID   gc,
    UGL_DDB_ID  bmpId
    );

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
    );

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
    );

/******************************************************************************
 *
 * uglClipRegionSet - Set graphics context clipping region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClipRegionSet (
    UGL_GC_ID      gc,
    UGL_REGION_ID  clipRegionId
    );

/******************************************************************************
 *
 * uglForegroundColorSet - Set graphics context foreground color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglForegroundColorSet (
    UGL_GC_ID  gc,
    UGL_COLOR  color
    );

/******************************************************************************
 *
 * uglBackgroundColorSet - Set graphics context background color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBackgroundColorSet (
    UGL_GC_ID  gc,
    UGL_COLOR  color
    );

/******************************************************************************
 *
 * uglRasterModeSet - Set graphics context raster mode
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRasterModeSet (
    UGL_GC_ID      gc,
    UGL_RASTER_OP  rasterOp
    );

/******************************************************************************
 *
 * uglFillPatternSet - Set graphics context fill pattern
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglFillPatternSet (
    UGL_GC_ID      gc,
    UGL_MDDB_ID    patternBitmap
    );

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
    );

/******************************************************************************
 *
 * uglClipListGet - Get clip rectangles from graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClipListGet (
    UGL_GC_ID         gc,
    UGL_RECT *        pRect,
    const UGL_RECT ** ppRect
    );

/* Pixel support functions */

/******************************************************************************
 *
 * uglPixelSet - Set pixel
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglPixelSet (
    UGL_GC_ID  gc,
    UGL_POS    x,
    UGL_POS    y,
    UGL_COLOR  color
    );

/* Color support functions */

/******************************************************************************
 *
 * uglClutSet
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClutSet (
    UGL_DEVICE_ID  devId,
    UGL_ORD        offset,
    UGL_ARGB *     pColors,
    UGL_SIZE       numColors
    );

/******************************************************************************
 *
 * uglClutGet
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglClutGet (
    UGL_DEVICE_ID  devId,
    UGL_ORD        offset,
    UGL_ARGB *     pColors,
    UGL_SIZE       numColors
    );

/******************************************************************************
 *
 * uglColorAllocExt - Allocate color extended
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorAllocExt (
    UGL_DEVICE_ID  devId,
    UGL_ARGB *     pReqColors,
    UGL_ORD *      pIndex,
    UGL_ARGB *     pActualColors,
    UGL_COLOR *    pUglColors,
    UGL_SIZE       numColors
    );

/******************************************************************************
 *
 * uglColorAlloc - Allocate color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorAlloc (
    UGL_DEVICE_ID  devId,
    UGL_ARGB *     pReqColors,
    UGL_ORD *      pIndex,
    UGL_COLOR *    pUglColors,
    UGL_SIZE       numColors
    );

/******************************************************************************
 *
 * uglColorFree - Free color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorFree (
    UGL_DEVICE_ID  devId,
    UGL_COLOR *    pColors,
    UGL_SIZE       numColors
    );

/* Line drawing support functions */

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
    );

/* Rectangle drawing support functions */

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
    );

/* Bitmap support functions */

/******************************************************************************
 *
 * uglBitmapCreate - Create bitmap
 *
 * RETURNS: Pointer to device dependent bitmap
 */

UGL_DDB_ID uglBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_COLOR            initValue,
    UGL_MEM_POOL_ID      poolId
    );

/******************************************************************************
 *
 * uglBitmapDestroy - Free bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_DDB *        pDdb
    );

/******************************************************************************
 *
 * uglBitmapBlt - Block transfer for device independet bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBitmapBlt (
    UGL_GC_ID      gc,
    UGL_BITMAP_ID  srcBmpId,
    UGL_POS        srcLeft,
    UGL_POS        srcTop,
    UGL_POS        srcRight,
    UGL_POS        srcBottom,
    UGL_DDB_ID     destBmpId,
    UGL_POS        destX,
    UGL_POS        destY
    );

/* Monocrhome bitmap support functions */

/******************************************************************************
 *
 * uglMonoBitmapCreate - Create monochrome bitmap
 *
 * RETURNS: Pointer to device dependent monochrome bitmap
 */

UGL_DDB_ID uglMonoBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_MDIB *           pMdib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_UINT8            initValue,
    UGL_MEM_POOL_ID      poolId
    );

/******************************************************************************
 *
 * uglMonoBitmapDestroy - Free monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglMonoBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_MDDB *       pMddb
    );

/******************************************************************************
 *
 * uglMonoBitmapRead - Read monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglMonoBitmapRead (
    UGL_GC_ID    gc,
    UGL_MDDB_ID  mDdbId,
    UGL_POS      srcLeft,
    UGL_POS      srcTop,
    UGL_POS      srcRight,
    UGL_POS      srcBottom,
    UGL_MDIB *   pMdib,
    UGL_POS      destX,
    UGL_POS      destY
    );

/******************************************************************************
 *
 * uglTransBitmapCreate - Create transparent bitmap
 *
 * RETURNS: Pointer to device dependent transparent bitmap
 */

UGL_DDB_ID uglTransBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_MDIB *           pMdib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_COLOR            initValue,
    UGL_MEM_POOL_ID      poolId
    );

/******************************************************************************
 *
 * uglTransBitmapDestroy - Free transparent bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglTransBitmapDestroy (
    UGL_DEVICE_ID    devId,
    UGL_TDDB *       pTddb
    );

/* Batch job support functions */

/******************************************************************************
 *
 * uglBatchStart - Start batch job
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBatchStart (
    UGL_GC_ID  gc
    );

/******************************************************************************
 *
 * uglBatchEnd - End batch job
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglBatchEnd (
    UGL_GC_ID  gc
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ugl_h */

