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
    UGL_INT32   i;
    UGL_COLOR * pSrc;
    UGL_UINT8 * pDest;

    /* TODO: Function not complete */

    if (destFormat == UGL_DEVICE_COLOR) {
        switch (srcFormat) {
            case UGL_DEVICE_COLOR_32:
                pSrc = (UGL_COLOR *) srcArray;
                pDest = (UGL_UINT8 *) destArray;

                for (i = 0; i < arraySize; i++) {
                    pDest[i] = (UGL_UINT8) (pSrc[i] & 0x000000ff);
                }
                break;

            default:
                /* TODO */
                return (UGL_STATUS_ERROR);
        }
    }
    else {
        /* TODO */
        return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

