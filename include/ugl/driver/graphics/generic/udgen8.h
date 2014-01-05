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

/* udgen8.h - Generic 8-Bit graphics support */

#ifndef _udgen8_h
#define _udgen8_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* 8-Bit pixel support functions */

/******************************************************************************
 *
 * uglGeneric8BitPixelSet - Set pixel
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitPixelSet (
    UGL_DEVICE_ID  devId,
    UGL_POINT *    p,
    UGL_COLOR      c
    );

/* 8-Bit line support functions */

/******************************************************************************
 *
 * uglGeneric8BitHLine - Draw horizontal line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitHLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              y,
    UGL_POS              x1,
    UGL_POS              x2,
    UGL_COLOR            color
    );

/******************************************************************************
 *
 * uglGeneric8BitVLine - Draw vertical line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitVLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POS              x,
    UGL_POS              y1,
    UGL_POS              y2,
    UGL_COLOR            color
    );

/******************************************************************************
 *
 * uglGeneric8BitBresenhamLine - Draw bresenham line
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitBresenhamLine (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_POINT *          pStartPoint,
    UGL_SIZE             numPoints,
    UGL_BOOL             xMajor,
    UGL_ORD              majorInc,
    UGL_ORD              minorInc,
    UGL_ORD              errorValue,
    UGL_ORD              majorErrorInc,
    UGL_ORD              minorErrorInc
    );

/******************************************************************************
 *
 * uglGeneric8BitBitmapCreate - Create 8-bit bitmap
 *
 * RETURNS: Bitmap id or UGL_NULL
 */

UGL_DDB_ID uglGeneric8BitBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_DIB *            pDib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_UINT32           initValue,
    UGL_MEM_POOL_ID      poolId
    );

/******************************************************************************
 *
 * uglGeneric8BitBitmapDestroy - Free bitmap
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglGeneric8BitBitmapDestroy (
    UGL_DEVICE_ID   devId,
    UGL_DDB_ID      ddbId
    );

/******************************************************************************
 *
 * uglGeneric8BitBitmapBlt - Blit from one bitmap memory area to another
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_DDB_ID     srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    );

/******************************************************************************
 *
 * uglGeneric8BitBitmapWrite - Write a device independent bitmap to vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_DIB *      pDib,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     ddbId,
    UGL_POINT *    pDestPoint
    );

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapCreate - Create 8-bit monochrome bitmap
 *
 * RETURNS: Bitmap id or UGL_NULL
 */

UGL_MDDB_ID uglGeneric8BitMonoBitmapCreate (
    UGL_DEVICE_ID        devId,
    UGL_MDIB *           pMdib,
    UGL_DIB_CREATE_MODE  createMode,
    UGL_UINT8            initValue,
    UGL_MEM_POOL_ID      poolId
    );

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapDestroy - Free monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglGeneric8BitMonoBitmapDestroy (
    UGL_DEVICE_ID   devId,
    UGL_MDDB_ID     mDdbId
    );

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapBlt - Blit monochrome bitmap to color bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitMonoBitmapBlt (
    UGL_DEVICE_ID  devId,
    UGL_MDDB_ID    srcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DDB_ID     destBmpId,
    UGL_POINT *    pDestPoint
    );

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapWrite - Write monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitMonoBitmapWrite (
    UGL_DEVICE_ID  devId,
    UGL_MDIB *     pMdib,
    UGL_RECT *     pSrcRect,
    UGL_MDDB_ID    mDdbId,
    UGL_POINT *    pDestPoint
    );

/******************************************************************************
 *
 * uglGeneric8BitMonoBitmapRead - Read monochrome bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitMonoBitmapRead (
    UGL_DEVICE_ID  devId,
    UGL_MDDB_ID    mDdbId,
    UGL_RECT *     pSrcRect,
    UGL_MDIB *     pMdib,
    UGL_POINT *    pDestPoint
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udgen8_h */

