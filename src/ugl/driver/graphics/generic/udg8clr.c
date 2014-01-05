/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2014 Surplus Users Ham Society
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

/* udg8clr.c - Generic color convert support for 8 bit color */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/******************************************************************************
 *
 * uglGeneric8BitColorConvert - Color convert for 8-bit color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGeneric8BitColorConvert (
    UGL_DEVICE_ID     devId,
    void *            srcArray,
    UGL_COLOR_FORMAT  srcFormat,
    void *            destArray,
    UGL_COLOR_FORMAT  destFormat,
    UGL_SIZE          arraySize
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_STATUS           status;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* TODO: Function not complete */

    if (destFormat == UGL_DEVICE_COLOR) {
        switch (srcFormat) {
            case UGL_DEVICE_COLOR:
                memcpy (destArray, srcArray, arraySize * sizeof (UGL_UINT8));
                status = UGL_STATUS_OK;
                break;

            case UGL_DEVICE_COLOR_32: {
                    UGL_COLOR * pSrc  = (UGL_COLOR *) srcArray;
                    UGL_UINT8 * pDest = (UGL_UINT8 *) destArray;
                    UGL_INT32   i;

                    for (i = 0; i < arraySize; i++) {
                        pDest[i] = (UGL_UINT8) (pSrc[i] & 0x000000ff);
                    }

                    status = UGL_STATUS_OK;
                }
                break;

            case UGL_ARGB8888: {
                    UGL_ARGB *  pSrc  = (UGL_ARGB *) srcArray;
                    UGL_UINT8 * pDest = (UGL_UINT8 *) destArray;
                    UGL_COLOR   color;
                    UGL_INT32   i;

                    status = UGL_STATUS_OK;
                    for (i = 0; i < arraySize; i++) {
                        if (uglCommonCubeMapNearest (pDrv->pClut, UGL_ARGB8888,
                                                     &pSrc[i], UGL_NULL, &color,
                                                     1) != UGL_STATUS_OK) {
                            status = UGL_STATUS_ERROR;
                            break;
                        }

                        pDest[i] = (UGL_UINT8) color;
                    }
                }
                break;

            default:
                /* TODO */
                status = UGL_STATUS_ERROR;
                break;
        }
    }
    else if (destFormat == UGL_DEVICE_COLOR_32) {
        status = uglGeneric8BitColorConvert (devId, srcArray, srcFormat,
                                             destArray, UGL_DEVICE_COLOR,
                                             arraySize);
        if (status == UGL_STATUS_OK) {
            UGL_UINT8 * pSrc  = (UGL_UINT8 *) srcArray;
            UGL_COLOR * pDest = (UGL_COLOR *) destArray;
            UGL_INT32   i;

            for (i = 0; i < arraySize; i++) {
                pDest[i] = (UGL_COLOR) pSrc[i];
            }
        }
    }
    else {
        /* TODO */
        status = UGL_STATUS_ERROR;
    }

    return (status);
}

