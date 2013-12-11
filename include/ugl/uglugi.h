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

/* uglugi.h - Universal graphics library output driver */

#ifndef _uglugi_h
#define _uglugi_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Graphics context change flags */

#define UGL_GC_DEFAULT_BITMAP_CHANGED           0x00000001
#define UGL_GC_CLIP_RECT_CHANGED                0x00000002
#define UGL_GC_FOREGROUND_COLOR_CHANGED         0x00000004
#define UGL_GC_BACKGROUND_COLOR_CHANGED         0x00000008
#define UGL_GC_RASTER_OP_CHANGED                0x00000010

typedef struct ugl_gc {
    struct ugl_ugi_driver * pDriver;            /* Current driver */
    UGL_DDB               * pDefaultBitmap;     /* Rendering output */
    UGL_RECT                boundRect;          /* Bounding rectagle */
    UGL_RECT                viewPort;           /* View port */
    UGL_RECT                clipRect;           /* Clipping rectangle */
    UGL_COLOR               foregroundColor;    /* Foreground color */
    UGL_COLOR               backgroundColor;    /* Background color */
    UGL_RASTER_OP           rasterOp;           /* Raster operation */
    UGL_UINT32              changed;            /* Changed flags */
    UGL_UINT32              magicNumber;        /* GC id and changed status */
    UGL_LOCK_ID             lockId;             /* Mutex */
} UGL_GC;

typedef struct ugl_gc * UGL_GC_ID;

typedef struct ugl_ugi_driver {
    UGL_MODE *   pMode;                         /* Current graphics mode */
    UGL_ORD      batchCount;                    /* Number of nested batches */
    UGL_LOCK_ID  lockId;                        /* Mutex */
    UGL_UINT32   magicNumber;                   /* Idetifies GC flags set */
    UGL_GC_ID    defaultGc;                     /* Default graphics context*/

    /* Device support methods */

    UGL_STATUS   (*destroy) (
        struct ugl_ugi_driver * pDrv
        );

    /* Info support method */

    UGL_STATUS   (*info) (
        struct ugl_ugi_driver * pDrv,
        UGL_INFO_REQ            infoReq,
        void *                  info
        );

    /* Mode support methods */

    UGL_STATUS   (*modeAvailGet) (
        struct ugl_ugi_driver * pDrv,
        UGL_UINT32 *            pNumModes,
        const UGL_MODE **       pModeArray
        );

    UGL_STATUS   (*modeSet) (
        struct ugl_ugi_driver * pDrv,
        UGL_MODE *              pMode
        );

    /* Graphics context support */

    UGL_GC *     (*gcCreate) (
        struct ugl_ugi_driver * pDrv
        );

    UGL_STATUS   (*gcCopy) (
        struct ugl_ugi_driver * pDrv,
        UGL_GC *                pSrcGc,
        UGL_GC *                pDestGc
        );

    UGL_STATUS   (*gcDestroy) (
        struct ugl_ugi_driver * pDrv,
        UGL_GC *                pGc
        );

    UGL_STATUS   (*gcSet) (
        struct ugl_ugi_driver * pDrv,
        UGL_GC *                pGc
        );

    /* Graphics primitive support */

    UGL_STATUS   (*pixelSet) (
        struct ugl_ugi_driver * pDrv,
        UGL_POINT *             p,
        UGL_COLOR               c
        );

    /* Color support */
    UGL_STATUS   (*colorAlloc) (
        struct ugl_ugi_driver * pDrv,
        UGL_ARGB *              pReqColors,
        UGL_ORD *               pIndex,
        UGL_ARGB *              pActualColors,
        UGL_COLOR *             pUglColors,
        UGL_SIZE                numColors
        );

    UGL_STATUS   (*colorFree) (
        struct ugl_ugi_driver * pDrv,
        UGL_COLOR *             pColors,
        UGL_SIZE                numColors
        );

    UGL_STATUS   (*clutSet) (
        struct ugl_ugi_driver * pDrv,
        UGL_ORD                 offset,
        UGL_ARGB *              pColors,
        UGL_SIZE                numColors
        );

    UGL_STATUS   (*clutGet) (
        struct ugl_ugi_driver * pDrv,
        UGL_ORD                 offset,
        UGL_ARGB *              pColors,
        UGL_SIZE                numColors
        );

    UGL_STATUS   (*colorConvert) (
        struct ugl_ugi_driver * pDrv,
        void *                  srcArray,
        UGL_COLOR_FORMAT        srcFormat,
        void *                  destArray,
        UGL_COLOR_FORMAT        destFormat,
        UGL_SIZE                arraySize
        );

    /* Bitmap support */

    UGL_DDB *    (*bitmapCreate) (
        struct ugl_ugi_driver * pDrv,
        UGL_DIB *               pDib,
        UGL_DIB_CREATE_MODE     createMode,
        UGL_COLOR               intiValue,
        UGL_MEM_POOL_ID         poolId
        );

    UGL_STATUS   (*bitmapDestroy) (
        struct ugl_ugi_driver * pDrv,
        UGL_DDB *               pDdb,
        UGL_MEM_POOL_ID         poolId
        );

    UGL_STATUS   (*bitmapBlt) (
        struct ugl_ugi_driver * pDrv,
        UGL_DDB *               pSrcBmp,
        UGL_RECT *              pSrcRect,
        UGL_DDB *               pDestBmp,
        UGL_POINT *             pDestPoint
        );

    UGL_STATUS   (*bitmapWrite) (
        struct ugl_ugi_driver * pDrv,
        UGL_DIB *               pDib,
        UGL_RECT *              pSrcRect,
        UGL_DDB *               pDdb,
        UGL_POINT *             pDestPoint
        );

    UGL_DDB *    (*monoBitmapCreate) (
        struct ugl_ugi_driver * pDrv,
        UGL_MDIB *              pMdib,
        UGL_DIB_CREATE_MODE     createMode,
        UGL_UINT8               intiValue,
        UGL_MEM_POOL_ID         poolId
        );

    UGL_STATUS   (*monoBitmapDestroy) (
        struct ugl_ugi_driver * pDrv,
        UGL_MDDB *              pMddb,
        UGL_MEM_POOL_ID         poolId
        );

    UGL_STATUS   (*monoBitmapBlt) (
        struct ugl_ugi_driver * pDrv,
        UGL_MDDB *              pSrcBmp,
        UGL_RECT *              pSrcRect,
        UGL_DDB *               pDestBmp,
        UGL_POINT *             pDestPoint
        );

    UGL_STATUS   (*monoBitmapWrite) (
        struct ugl_ugi_driver * pDrv,
        UGL_MDIB *              pMdib,
        UGL_RECT *              pSrcRect,
        UGL_MDDB *              pMddb,
        UGL_POINT *             pDestPoint
        );

} UGL_UGI_DRIVER;

typedef struct ugl_ugi_driver * UGL_DEVICE_ID;

/* Macros */

/******************************************************************************
 *
 * UGL_GC_CHANGED_SET - Mark graphics context as changed
 *
 * RETURNS: N/A
 */

#define UGL_GC_CHANGED_SET(gc)          (gc)->magicNumber |= 0x80000000L

/******************************************************************************
 *
 * UGL_GC_CHANGED_CLEAR - Clear graphics context change flags
 *
 * RETURNS: N/A
 */

#define UGL_GC_CHANGED_CLEAR(gc)        (gc)->magicNumber &= 0x7fffffffL

/******************************************************************************
 *
 * UGL_GC_SET - Set graphics context
 *
 * RETURNS: N/A
 */

#define UGL_GC_SET(devId, gcId)                                               \
    if ((devId)->magicNumber != (gcId)->magicNumber) {                        \
        (*(devId)->gcSet) (devId, gcId);                                      \
        (gcId)->changed = 0;                                                  \
        UGL_GC_CHANGED_CLEAR (gcId);                                          \
        (devId)->magicNumber = (gcId)->magicNumber;                           \
    }                                                                         \

/******************************************************************************
 *
 * UGL_CLIP_LOOP_START/UGL_CLIP_LOOP_END - Start/end clipping loop
 *
 * RETURNS: N/A
 */

#define UGL_CLIP_LOOP_START(gc, pClipRect)                                    \
{                                                                             \
    UGL_STATUS _status;                                                       \
    do {                                                                      \
        (pClipRect) = &(gc)->clipRect;                                        \
        UGL_RECT_MOVE(*pClipRect, (gc)->viewPort.left, (gc)->viewPort.top);   \
        _status = UGL_STATUS_FINISHED;
#define UGL_CLIP_LOOP_END                                                     \
    } while (_status == UGL_STATUS_OK);                                       \
}

/******************************************************************************
 *
 * uglUgiDevInit - Initialize graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglUgiDevInit (
    UGL_DEVICE_ID  devId
    );

/******************************************************************************
 *
 * uglUgiDevDeinit - Deinitialize graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglUgiDevDeinit (
    UGL_DEVICE_ID  devId
    );

/******************************************************************************
 *
 * uglGraphicsDevDestroy - Free graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglGraphicsDevDestroy (
    UGL_DEVICE_ID  devId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglugi_h */

