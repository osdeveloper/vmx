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

/* udvga.h - Universal graphics library vga driver */

#ifndef _udvga_h
#define _udvga_h

#define UGL_GRAPHICS_CREATE             uglVgaDevCreate

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

typedef struct ugl_vga_driver {
    UGL_GENERIC_DRIVER  generic;        /* Generic driver (required) */
    UGL_SIZE            colorPlanes;    /* Number of planes */
    UGL_SIZE            bytesPerLine;   /* Number of bytes per line */
    UGL_RASTER_OP       rasterOp;       /* Raster operation */
    UGL_COLOR           color;          /* Current color */
} UGL_VGA_DRIVER;

typedef struct ugl_vga_ddb {
    UGL_BMAP_HEADER  header;            /* Bitmap header */
    UGL_UINT16       colorDepth;        /* Color depth */
    UGL_UINT16       stride;            /* Scanline offset */
    UGL_UINT16       shiftValue;        /* Alignment shift */
    UGL_UINT8 **     pPlaneArray;       /* Bit planes */
} UGL_VGA_DDB;

typedef struct ugl_vga_mddb {
  UGL_BMAP_HEADER  header;              /* Bitmap header */
  UGL_UINT16       stride;              /* Scanline offset */
  UGL_UINT16       shiftValue;          /* Alignment shift */
  UGL_UINT8 **     pPlaneArray;         /* Bit planes */
} UGL_VGA_MDDB;

/* Device initialization support functions */

/******************************************************************************
 *
 * uglVgaDevCreate - Create vga graphics driver
 *
 * RETURNS: Pointer to driver structure (UGL_UGI_DRIVER *) or UGL_NULL
 */

UGL_UGI_DRIVER * uglVgaDevCreate (
    UGL_UINT32  arg0,
    UGL_UINT32  arg1,
    UGL_UINT32  arg2
    );

extern UGL_STATUS uglVgaDevDestroy(UGL_DEVICE_ID devId);
extern UGL_STATUS uglVgaInfo(UGL_DEVICE_ID devId,
                             UGL_INFO_REQ infoReq,
                             void *info);
extern UGL_STATUS uglVgaGcSet(UGL_DEVICE_ID devId, UGL_GC_ID gc);
extern UGL_STATUS uglVgaPixelSet(UGL_DEVICE_ID devId,
                                 UGL_POINT *p,
                                 UGL_COLOR c);
extern UGL_STATUS uglVgaClutSet(UGL_DEVICE_ID devId,
                                UGL_ORD offset,
                                UGL_ARGB *pColors,
                                UGL_SIZE numColors);
extern UGL_STATUS uglVgaClutGet(UGL_DEVICE_ID devId,
                                UGL_ORD offset,
                                UGL_ARGB *pColors,
                                UGL_SIZE numColors);
extern UGL_STATUS uglVga4BitColorConvert(UGL_DEVICE_ID devId,
                                         void *srcArray,
                                         UGL_COLOR_FORMAT srcFormat,
                                         void *destArray,
                                         UGL_COLOR_FORMAT destFormat,
                                         UGL_SIZE arraySize);
extern UGL_DDB_ID uglVgaBitmapCreate(UGL_DEVICE_ID devId,
                                     UGL_DIB *pDib,
                                     UGL_DIB_CREATE_MODE createMode,
                                     UGL_UINT32 initValue,
                                     UGL_MEM_POOL_ID poolId);
extern UGL_STATUS uglVgaBitmapDestroy(UGL_DEVICE_ID devId,
                                      UGL_DDB_ID ddbId,
                                      UGL_MEM_POOL_ID poolId);
extern UGL_STATUS uglVgaBitmapBlt(UGL_DEVICE_ID devId,
                                  UGL_DDB_ID srcBmpId,
                                  UGL_RECT *pSrcRect,
                                  UGL_DDB_ID destBmpId,
                                  UGL_POINT *pDestPoint);
extern UGL_STATUS uglVgaBitmapWrite(UGL_DEVICE_ID devId,
                                    UGL_DIB *pDib,
                                    UGL_RECT *pSrcRect,
                                    UGL_DDB_ID ddbId,
                                    UGL_POINT *pDestPoint);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udvga_h */

