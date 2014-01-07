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

/* udvgaclr.c - Vga color support */

#include "ugl.h"
#include "driver/graphics/common/udcclr.h"
#include "driver/graphics/vga/udvga.h"

/******************************************************************************
 *
 * uglVgaClutSet - Set palette entry
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaClutSet (
    UGL_DEVICE_ID  devId,
    UGL_ORD        offset,
    UGL_ARGB *     pColors,
    UGL_SIZE       numColors
    ) {
    UGL_INT32  i;

    /* Call generic method */
    if (uglGenericClutSet ((UGL_GENERIC_DRIVER *) devId,
                           offset, pColors, numColors) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Write data to vga hardware */
    UGL_OUT_BYTE (0x3c8, offset);

    for (i = 0; i < numColors; i++) {
        UGL_OUT_BYTE (0x3c9, (UGL_ARGB_RED (pColors[i]) >> 2));
        UGL_OUT_BYTE (0x3c9, (UGL_ARGB_GREEN (pColors[i]) >> 2));
        UGL_OUT_BYTE (0x3c9, (UGL_ARGB_BLUE (pColors[i]) >> 2));
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglVgaClutGet - Get palette entry
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaClutGet (
    UGL_DEVICE_ID  devId,
    UGL_ORD        offset,
    UGL_ARGB *     pColors,
    UGL_SIZE       numColors
    ) {

    return uglGenericClutGet ((UGL_GENERIC_DRIVER *) devId, offset,
                              pColors, numColors);
}

/******************************************************************************
 *
 * uglVga4BitColorConvert - Color format conversion
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVga4BitColorConvert (
    UGL_DEVICE_ID    devId,
    void *           srcArray,
    UGL_COLOR_FORMAT srcFormat,
    void *           destArray,
    UGL_COLOR_FORMAT destFormat,
    UGL_SIZE         arraySize
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_UINT8 **         pDestArray;
    UGL_UINT8 **         pSrcArray;
    UGL_UINT8 *          pDest;
    UGL_UINT8 *          pSrc;
    UGL_UINT8            destMask;
    UGL_UINT8            srcMask;
    UGL_COLOR *          pDestColor;
    UGL_COLOR *          pSrcColor;
    UGL_COLOR *          pColorBuf;
    UGL_UINT32           i;
    UGL_UINT32           j;
    UGL_UINT32           size;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* If destination color format is device dependent */
    if (destFormat == UGL_DEVICE_COLOR) {

        /* Select source color format */
        switch(srcFormat) {
            case UGL_DEVICE_COLOR:

                /* Setup source, destination and size */
                pSrcArray  = (UGL_UINT8 **) srcArray;
                pDestArray = (UGL_UINT8 **) destArray;
                size       = (arraySize + 7) / 8;

                /* For all color planes */
                for (i = 0; i < 4; i++) {
                    memcpy (pDestArray[i], pSrcArray[i], size);
                }
                break;

            case UGL_DEVICE_COLOR_32:

                /* Setup source, destination and size */
                pSrcColor  = (UGL_COLOR *) srcArray;
                pDestArray = (UGL_UINT8 **) destArray;
                size       = (arraySize + 7) / 8;

                /* For all color planes */
                for (i = 0; i < 4; i++) {

                    /* Setup destination and masks */
                    pDest = pDestArray[i];
                    srcMask  = 0x01 << i;
                    destMask = 0x80;

                    /* Zero out destination */
                    memset (pDest, 0, size);

                    /* Over array size */
                    for (j = 0; j < arraySize; i++) {
                        if ((pSrcColor[j] & srcMask) != 0) {
                            *pDest |= destMask;
                        }

                        /* Shift mask */
                        destMask >>= 1;

                        /* Check if mask reached zero */
                        if (destMask == 0) {

                            /* Advance destination and reset mask */
                            pDest++;
                            destMask = 0x80;
                        }
                    }
                }
                break;

            case UGL_ARGB8888:
                pColorBuf = (UGL_COLOR *) uglScratchBufferAlloc (devId,
                                                arraySize * sizeof (UGL_COLOR));
                if (pColorBuf == UGL_NULL) {
                    return (UGL_STATUS_ERROR);
                }

                uglGenericClutMapNearest (pDrv, (UGL_ARGB *) srcArray, UGL_NULL,
                                          pColorBuf, arraySize);

                if (uglVga4BitColorConvert (devId, pColorBuf,
                                            UGL_DEVICE_COLOR_32,
                                            destArray, destFormat,
                                            arraySize) == UGL_STATUS_ERROR) {
                    uglScratchBufferFree (devId, pColorBuf);
                    return (UGL_STATUS_ERROR);
                }

                uglScratchBufferFree (devId, pColorBuf);

                break;

            default:
                return (UGL_STATUS_ERROR);
        }

        return (UGL_STATUS_OK);
    }


    /* If destination color format is device dependent 32-bit */
    if (destFormat == UGL_DEVICE_COLOR_32) {

        /* Select source color format */
        switch(srcFormat) {
            case UGL_DEVICE_COLOR:

                /* Setup source, destination and size */
                pDestColor = (UGL_COLOR *) destArray;
                pSrcArray  = (UGL_UINT8 **) srcArray;
                size       = sizeof (UGL_COLOR) * arraySize;

                /* Zero out destination array */
                memset (pDest, 0, size);

                /* Setup destination mask */
                destMask = 0x01;

                /* For all color planes */
                for (i = 0; i < 4; i++) {

                    /* Setup source and mask */
                    pSrc    = pSrcArray[i];
                    srcMask = 0x80;

                    /* Over array size */
                    for (j = 0; j < arraySize; j++) {

                        if ((pSrc[i] & srcMask) != 0) {
                            pDestColor[j] |= destMask;
                        }

                        /* Shift source mask */
                        srcMask >>= 1;

                        /* Check if source mask reached zero */
                        if (srcMask == 0) {
                            pSrc++;
                            srcMask = 0x80;
                        }

                        /* Shift destination mask */
                        destMask <<= 1;
                    }
                }
                break;

            case UGL_DEVICE_COLOR_32:
                memcpy (destArray, srcArray, arraySize * sizeof (UGL_COLOR));
                break;

            case UGL_ARGB8888:
                uglCommonClutMapNearest (pDrv->pClut,
                                         (UGL_ARGB *) srcArray, UGL_NULL,
                                         (UGL_COLOR *) destArray, arraySize);
                break;

            default:
                return (UGL_STATUS_ERROR);
        }
    }

    return (UGL_STATUS_OK);
}

