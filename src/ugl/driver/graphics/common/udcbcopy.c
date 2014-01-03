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

/* udcbcopy.c - Common byte copy with raster operation */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcomm.h>

/******************************************************************************
 *
 * uglCommonByteCopy - Copy data bytewise using raster operation
 *
 * RETURNS: N/A
 */

UGL_VOID uglCommonByteCopy (
    const void *   pSrc,
    void *         pDest,
    int            nBytes,
    UGL_RASTER_OP  rasterOp
    ) {
    int               sz;
    const UGL_UINT8 * src;
    UGL_UINT8 *       dest;
    int               count;

    if (rasterOp == UGL_RASTER_OP_COPY) {
        memcpy (pDest, pSrc, nBytes);
    }
    else {
        sz = (unsigned int) pDest - (unsigned int) pSrc;
        if (sz <= 0 || sz >= nBytes) {

            /* Perform forward copy */
            src   = (UGL_UINT8 *) pSrc;
            dest  = (UGL_UINT8 *) pDest;
            count = nBytes;

            switch (rasterOp) {
                case UGL_RASTER_OP_AND:
                    while (--count >= 0) {
                        *dest &= *src;
                        dest++;
                        src++;
                    }
                    break;

                case UGL_RASTER_OP_OR:
                    while (--count >= 0) {
                        *dest |= *src;
                        dest++;
                        src++;
                    }
                    break;

                case UGL_RASTER_OP_XOR:
                    while (--count >= 0) {
                        *dest ^= *src;
                        dest++;
                        src++;
                    }
                    break;
            }
        }
        else {

            /* Backward copy */
            src   = (char *) pSrc + nBytes;
            dest  = (char *) pDest + nBytes;
            count = nBytes;

            switch (rasterOp) {
                case UGL_RASTER_OP_AND:
                    while (--count >= 0) {
                        dest--;
                        src--;
                        *dest &= *src;
                    }
                    break;

                case UGL_RASTER_OP_OR:
                    while (--count >= 0) {
                        dest--;
                        src--;
                        *dest |= *src;
                    }
                    break;

                case UGL_RASTER_OP_XOR:
                    while (--count >= 0) {
                        dest--;
                        src--;
                        *dest ^= *src;
                    }
                    break;
            }
        }
    }
}

