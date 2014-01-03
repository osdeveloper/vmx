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

/* udvga.c - Universal graphics library display driver for vga */

#include <stdlib.h>
#include <string.h>
#include <arch/sysArchLib.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/graphics/generic/udgen8.h>
#include <ugl/driver/graphics/vga/udvgamode.h>
#include <ugl/driver/graphics/vga/udvga.h>

/* Locals */
UGL_LOCAL UGL_MODE modes[] = {
    {
        "320x200x8 @ 60Hz",
        320, 200, 8, 60,
        UGL_MODE_CRT,
        UGL_MODE_INDEXED_COLOR,
        0,
        640, 664, 760, 800, 0,
        480, 491, 493, 525
    },

    {
        "640x480x4 @ 60Hz",          /* h: 31.5 kHz v: 60Hz */
        640, 480, 4, 60,
        UGL_MODE_CRT,
        UGL_MODE_INDEXED_COLOR,
        0,
        640, 664, 760, 800, 0,
        480, 491, 493, 525
    }
};

UGL_LOCAL UGL_MODE *pModes = modes;

UGL_LOCAL UGL_STATUS uglVgaModeAvailGet (
    UGL_DEVICE_ID     devId,
    UGL_UINT32 *      pNumModes,
    const UGL_MODE ** pModeArray
    );

UGL_LOCAL UGL_STATUS uglVgaModeSet (
    UGL_DEVICE_ID  devId,
    UGL_MODE *     pMode
    );

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
    ) {
    UGL_VGA_DRIVER * pDrv;
    UGL_DEVICE_ID    devId = UGL_NULL;

    /* Allocate memory for driver data structure */
    pDrv = (UGL_VGA_DRIVER *) UGL_CALLOC (1, sizeof (UGL_VGA_DRIVER));
    if (pDrv == NULL) {
        return (UGL_NULL);
    }

    /* Store at begining of device struct, since it must be the first field */
    devId = (UGL_DEVICE_ID) pDrv;

    /* Initialize device driver */
    uglUgiDevInit (devId);

    /* Set frame buffer address */
    pDrv->generic.fbAddress = (UGL_UINT8 *) 0xa0000L;

    /* Setup device support methods */
    devId->destroy          = uglVgaDevDestroy;

    /* Setup info method */
    devId->info             = uglVgaInfo;

    /* Setup mode support methods */
    devId->modeAvailGet     = uglVgaModeAvailGet;
    devId->modeSet          = uglVgaModeSet;

    /* Setup graphics context support methods */
    devId->gcCreate         = uglGenericGcCreate;
    devId->gcCopy           = uglGenericGcCopy;
    devId->gcDestroy        = uglGenericGcDestroy;
    devId->gcSet            = uglVgaGcSet;

    /* Set color support methods */
    devId->colorAlloc       = uglGenericColorAllocIndexed;
    devId->colorFree        = uglGenericColorFreeIndexed;
    devId->clutGet          = uglVgaClutGet;
    devId->clutSet          = uglVgaClutSet;
    devId->colorConvert     = uglVga4BitColorConvert;

    return (devId);
}

/******************************************************************************
 *
 * uglVgaModeAvailGet - Get available graphics modes
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_LOCAL UGL_STATUS uglVgaModeAvailGet (
    UGL_DEVICE_ID     devId,
    UGL_UINT32 *      pNumModes,
    const UGL_MODE ** pModeArray
    ) {

    *pModeArray = pModes;

    /* Calculate number of elements */
    *pNumModes = sizeof (modes) / sizeof (modes[0]);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaModeSet - Set graphics mode
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglVgaModeSet (
    UGL_DEVICE_ID  devId,
    UGL_MODE *     pMode
    ) {
    UGL_VGA_DRIVER * pDrv;
    UGL_INT32        modeIndex;
    UGL_UINT8        byteValue;
    UGL_GEN_DDB *    pDdb;
    struct vgaHWRec  regs;

    /* Get driver from device struct, since it it the first entry */
    pDrv = (UGL_VGA_DRIVER *) devId;

    /* Get requested mode from list */
    modeIndex = uglGenericModeFind (modes, pMode,
                                    sizeof (modes) / sizeof (modes[0]));
    if (modeIndex == UGL_STATUS_ERROR) {
        return (UGL_STATUS_ERROR);
    }

    /* Store current mode */
    devId->pMode = &modes[modeIndex];

    /* Set graphics mode */
    vgaInitMode (&modes[modeIndex], &regs);
    vgaLoadPalette (&regs, NULL);
    vgaRestore (&regs, 0);

    /* Select graphics mode */
    switch(modeIndex) {
        /* Vga mode 13 */
        case 0:
            /* Change sync polarity */
            UGL_OUT_BYTE (0x3c0, 0x63);

            /* Modify sequencer registers */
            UGL_OUT_WORD (0x3c4, 0x0e04);       /* Enable Chain-4 */

            /* Modify graphics controller registers */
            UGL_OUT_WORD (0x3ce, 0x4005);       /* Set mode register */

            /* Modify CRT controller registers */
            UGL_OUT_WORD (0x3d4, 0x2c11);       /* Disable write protection */
            UGL_OUT_WORD (0x3d4, 0xbf06);       /* Vertical total */
            UGL_OUT_WORD (0x3d4, 0x1f07);       /* Overflow register */
            UGL_OUT_WORD (0x3d4, 0x4109);       /* Set duplicate scanlines */
            UGL_OUT_WORD (0x3d4, 0x9c10);       /* Vertical retrace start */
            UGL_OUT_WORD (0x3d4, 0x8e11);       /* Vertical retrace end */
            UGL_OUT_WORD (0x3d4, 0x8f12);       /* Vertical display end */
            UGL_OUT_WORD (0x3d4, 0x2813);       /* Set logical width */
            UGL_OUT_WORD (0x3d4, 0x4014);       /* Enable double word mode */
            UGL_OUT_WORD (0x3d4, 0x9615);       /* Vertical blanking start */
            UGL_OUT_WORD (0x3d4, 0xb916);       /* Vertical blanking end */
            UGL_OUT_WORD (0x3d4, 0xa317);       /* Enable word mode */

            /* Set attribute registers */
            byteValue = UGL_IN_BYTE(0x3da);     /* Start attribute set */
            UGL_OUT_BYTE (0x3c0, 0x10);         /* Set mode control register */
            UGL_OUT_BYTE (0x3c0, 0x41);         /* for lores 256 color */
            UGL_OUT_BYTE (0x3c0, 0x20);         /* End attribute set */

            /* Set releated info */
            pDrv->bytesPerLine = devId->pMode->width;
            pDrv->colorPlanes  = devId->pMode->colorDepth;

            /* Setup first drawing page */
            devId->pPageZero = (UGL_PAGE *) UGL_CALLOC (1, sizeof (UGL_PAGE) + 
                                                        sizeof (UGL_GEN_DDB));
            if (devId->pPageZero == UGL_NULL) {
                return (UGL_STATUS_ERROR);
            }

            pDdb = (UGL_GEN_DDB *) &devId->pPageZero[1];
            pDdb->header.width  = devId->pMode->width;
            pDdb->header.height = devId->pMode->height;
            pDdb->header.type   = UGL_DDB_TYPE;
            pDdb->colorDepth    = devId->pMode->colorDepth;
            pDdb->stride        = devId->pMode->width;
            pDdb->pData         = pDrv->generic.fbAddress;

            devId->pPageZero->pDdb = (UGL_DDB *) pDdb;

            pDrv->generic.pDrawPage    = devId->pPageZero;
            pDrv->generic.pVisiblePage = devId->pPageZero;

            /* Create palette for 256 colors */
            if (uglGenericClutCreate ((UGL_GENERIC_DRIVER *) devId, 256)
                != UGL_STATUS_OK) {
                UGL_FREE (devId->pPageZero);
                return (UGL_STATUS_ERROR);
            }

            /* Set graphics primitives support methods */
            devId->pixelSet = uglGeneric8BitPixelSet;

            /* Setup bitmap support methods */
            devId->bitmapCreate  = uglGeneric8BitBitmapCreate;
            devId->bitmapDestroy = uglGeneric8BitBitmapDestroy;
            devId->bitmapBlt     = uglGeneric8BitBitmapBlt;
            devId->bitmapWrite   = uglGeneric8BitBitmapWrite;

            /* Clear screen */
            memset (pDrv->generic.fbAddress, 0,
                    devId->pMode->height * pDrv->bytesPerLine);
            break;

        /* Vga mode 12 */
        case 1:
            /* Set releated info */
            pDrv->bytesPerLine = devId->pMode->width / 8;
            pDrv->colorPlanes  = devId->pMode->colorDepth;

            /* Set generic driver methods */
            pDrv->generic.hLine =         uglVgaHLine;
            pDrv->generic.vLine =         uglVgaVLine;
            pDrv->generic.bresenhamLine = uglVgaBresenhamLine;
            pDrv->generic.fill          = uglGenericFill;

            /* Setup first drawing page */
            devId->pPageZero = (UGL_PAGE *) UGL_CALLOC (1, sizeof (UGL_PAGE) + 
                                                        sizeof (UGL_GEN_DDB));
            if (devId->pPageZero == UGL_NULL) {
                return (UGL_STATUS_ERROR);
            }

            pDdb = (UGL_GEN_DDB *) &devId->pPageZero[1];
            pDdb->header.width  = devId->pMode->width;
            pDdb->header.height = devId->pMode->height;
            pDdb->header.type   = UGL_DDB_TYPE;
            pDdb->colorDepth    = devId->pMode->colorDepth;
            pDdb->stride        = devId->pMode->width;
            pDdb->pData         = pDrv->generic.fbAddress;

            devId->pPageZero->pDdb = (UGL_DDB *) pDdb;

            pDrv->generic.pDrawPage    = devId->pPageZero;
            pDrv->generic.pVisiblePage = devId->pPageZero;

            /* Create palette for 16 colors */
            if (uglGenericClutCreate ((UGL_GENERIC_DRIVER *) devId, 16)
                != UGL_STATUS_OK) {
                UGL_FREE (devId->pPageZero);
                return (UGL_STATUS_ERROR);
            }

            /* Set graphics primitives support methods */
            devId->pixelSet  = uglVgaPixelSet;
            devId->line      = uglGenericLine;
            devId->rectangle = uglGenericRectangle;
            devId->polygon   = uglGenericPolygon;

            /* Setup bitmap support methods */
            devId->bitmapCreate             = uglVgaBitmapCreate;
            devId->bitmapDestroy            = uglVgaBitmapDestroy;
            devId->bitmapBlt                = uglVgaBitmapBlt;
            devId->bitmapWrite              = uglVgaBitmapWrite;
            devId->bitmapRead               = uglVgaBitmapRead;
            devId->monoBitmapCreate         = uglVgaMonoBitmapCreate;
            devId->monoBitmapDestroy        = uglVgaMonoBitmapDestroy;
            devId->monoBitmapBlt            = uglVgaMonoBitmapBlt;
            devId->monoBitmapWrite          = uglVgaMonoBitmapWrite;
            devId->monoBitmapRead           = uglVgaMonoBitmapRead;
            devId->transBitmapCreate        = uglGenericTransBitmapCreate;
            devId->transBitmapCreateFromDdb =
                uglGenericTransBitmapCreateFromDdb;
            devId->transBitmapDestroy       = uglGenericTransBitmapDestroy;
            devId->transBitmapBlt           = uglGenericTransBitmapLinearBlt;

            /* Setup cursor support methods */
            devId->cursorInit          = uglGenericCursorInit;
            devId->cursorDeinit        = uglGenericCursorDeinit;
            devId->cursorBitmapCreate  = uglGenericCursorBitmapCreate;
            devId->cursorBitmapDestroy = uglGenericCursorBitmapDestroy;
            devId->cursorImageSet      = uglGenericCursorImageSet;
            devId->cursorImageGet      = uglGenericCursorImageGet;
            devId->cursorOn            = uglGenericCursorOn;
            devId->cursorOff           = uglGenericCursorOff;
            devId->cursorHide          = uglGenericCursorHide;
            devId->cursorShow          = uglGenericCursorShow;
            devId->cursorMove          = uglGenericCursorMove;

            /* Set write mode 3 */
            UGL_OUT_BYTE (0x3ce, 0x05);
            byteValue = UGL_IN_BYTE (0x3cf);
            UGL_OUT_BYTE (0x3cf, byteValue | 0x03);

            /* Set set/reset register */
            UGL_OUT_WORD (0x3ce, 0x0f01);

            /* Clear screen */
            UGL_OUT_WORD (0x3ce, 0);
            memset (pDrv->generic.fbAddress, 0xff,
                    devId->pMode->height * pDrv->bytesPerLine);

            /* Enable video output */
            UGL_OUT_BYTE (0x3c4, 0x01);
            UGL_OUT_BYTE (0x3c5, 0x01);
            break;

        default:
            return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

