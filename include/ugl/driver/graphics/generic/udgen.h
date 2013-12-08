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

/* udgen.h - Universal graphics library generic driver */

#ifndef _udgen_h
#define _udgen_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcclr.h>

typedef struct ugl_generic_driver {
  UGL_UGI_DRIVER  ugi;                  /* UGI driver (required) */
  UGL_GC_ID       gc;                   /* Graphics context */
  UGL_CLUT *      pClut;                /* Palette */
  void *          fbAddress;            /* Frame buffer address */
} UGL_GENERIC_DRIVER;

typedef struct ugl_gen_bitmap {
  UGL_BMAP_HEADER  header;
} UGL_GENERIC_BMAP;

typedef UGL_GENERIC_BMAP * UGL_BMAP_ID;

typedef struct ugl_gen_ddb {
  UGL_BMAP_HEADER  header;              /* Header */
  UGL_UINT16       colorDepth;          /* Bits-per-pixel */
  UGL_UINT16       stride;              /* Pixels per line */
  UGL_UINT32       transColorKey;       /* Transparent color key */
  void *           pData;               /* Image data */
} UGL_GEN_DDB;

/* Generic mode support functions */

/******************************************************************************
 *
 * uglGenericModeFind- Find graphics mode
 *
 * RETURNS: Graphics mode index or UGL_STATUS_ERROR
 */

UGL_INT32 uglGenericModeFind(
    UGL_MODE *  pList,
    UGL_MODE *  pReqMode,
    UGL_UINT32  numModes
    );

/* Generic graphics context support functions */

/******************************************************************************
 *
 * uglGenericGcCreate - Create graphics context
 *
 * RETURNS: Graphics context id or UGL_NULL
 */

UGL_GC_ID uglGenericGcCreate (
    UGL_DEVICE_ID  devId
    );

/******************************************************************************
 *
 * uglGenericGcCopy - Copy graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericGcCopy (
    UGL_DEVICE_ID  devId,
    UGL_GC_ID      srcGcId,
    UGL_GC_ID      destGcId
    );

/******************************************************************************
 *
 * uglGenericGcDestroy - Free graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericGcDestroy (
    UGL_DEVICE_ID  devId,
    UGL_GC_ID      gc
    );

/******************************************************************************
 *
 * uglGenericGcSet - Set current graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericGcSet (
    UGL_DEVICE_ID  devId,
    UGL_GC_ID      gc
    );

/* Generic palette support functions */

/******************************************************************************
 *
 * uglGenericClutCreate - Create palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutCreate (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_SIZE             numColors
    );

/******************************************************************************
 *
 * uglGenericClutSet - Set generic indexed palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutSet (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_ORD              offset,
    UGL_ARGB *           pColors,
    UGL_SIZE             numColors
    );

/******************************************************************************
 *
 * uglGenericClutGet - Get palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutGet (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_ORD              offset,
    UGL_ARGB *           pColors,
    UGL_SIZE             numColors
    );

/******************************************************************************
 *
 * uglGenericClutMapNearest - Map to nearest match
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutMapNearest (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_ARGB *           pMapColors,
    UGL_ARGB  *          pActualColors,
    UGL_COLOR *          pUglColors,
    UGL_SIZE             numColors
    );

/******************************************************************************
 *
 * uglGenericClutAllocIndexed - Allocate color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericColorAllocIndexed (
    UGL_DEVICE_ID  devId,
    UGL_ARGB *     pReqColors,
    UGL_ORD *      pIndex,
    UGL_ARGB *     pActualColors,
    UGL_COLOR *    pUglColors,
    UGL_SIZE       numColors
    );

/******************************************************************************
 *
 * uglGenericClutFreeIndexed - Free color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericColorFreeIndexed (
    UGL_DEVICE_ID  devId,
    UGL_COLOR *    pColors,
    UGL_SIZE       numColors
    );

/******************************************************************************
 *
 * uglGenericClutDestroy - Free palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutDestroy (
    UGL_GENERIC_DRIVER * pDrv
    );

/* Generic clipt support functions */

/******************************************************************************
 *
 * uglGenericClipDdbToDdb - Generic clip of device dependent bitmap
 *
 * RETURNS: UGL_TRUE or UGL_FALSE
 */

UGL_STATUS uglGenericClipDdbToDdb (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pClipRect,
    UGL_BMAP_ID *  pSrcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_BMAP_ID *  pDestBmpId,
    UGL_POINT *    pDestPoint
    );

/******************************************************************************
 *
 * uglGenericClipDibToDdb - Generic clip of device independent to dependent
 *
 * RETURNS: UGL_TRUE or UGL_FALSE
 */

UGL_BOOL uglGenericClipDibToDdb (
    UGL_DEVICE_ID  devId,
    UGL_DIB *      pDib,
    UGL_RECT *     pSrcRect,
    UGL_BMAP_ID *  pBmpId,
    UGL_POINT *    pDestPoint
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udgen_h */

