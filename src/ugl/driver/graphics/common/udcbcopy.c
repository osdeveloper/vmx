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

    if (rasterOp == UGL_RASTER_OP_COPY) {
        memcpy (pDest, pSrc, nBytes);
    }
    else {
        int sz = (unsigned int) pDest - (unsigned int) pSrc;
        if (sz <= 0 || sz >= nBytes) {

            /* Perform forward copy */
            const UGL_UINT8 * src   = (UGL_UINT8 *) pSrc;
            UGL_UINT8 *       dest  = (UGL_UINT8 *) pDest;
            int               count = nBytes;

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
            const UGL_UINT8 * src   = (char *) pSrc + nBytes;
            UGL_UINT8 *       dest  = (char *) pDest + nBytes;
            int               count = nBytes;

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

/******************************************************************************
 *
 * uglCommonBitCopy - Copy data bitwise using raster operation
 *
 * RETURNS: N/A
 */

UGL_VOID uglCommonBitCopy (
    const void *   pSrc,
    int            srcBitOffset,
    void *         pDest,
    int            destBitOffset,
    int            nBits,
    UGL_RASTER_OP  rasterOp
    ) {
    const char * pSrcStart;
    char *       pDestStart;

    /* Check if trivial */
    if (nBits <= 0) {
        return;
    }

    /* Setup source */
    pSrcStart     = (const char *) pSrc + (srcBitOffset >> 3);
    srcBitOffset &= 0x07;

    /* Setup destination */
    pDestStart     = (char *) pDest + (destBitOffset >> 3);
    destBitOffset &= 0x07;

    if (pSrc > pDest || (pSrc == pDest && srcBitOffset > destBitOffset) ||
        ((((unsigned int) pDest - (unsigned int) pSrc) << 3) > nBits + 31)) {

        /* Forward copy */
        if (srcBitOffset == destBitOffset) {

            /* Bit aligned offsets */
            UGL_UINT8 * src   = (UGL_UINT8 *) pSrcStart;
            UGL_UINT8 * dest  = (UGL_UINT8 *) pDestStart;
            int         count = nBits;
            UGL_UINT8   mask;

            /* Handle start bit offset */
            if (destBitOffset > 0) {
                if (count + destBitOffset < 8) {
                    mask  = (UGL_UINT8) ((0xff >> destBitOffset) &
                                         (0xff << (8 - destBitOffset - count)));
                    count = 0;
                }
                else {
                    mask  = (UGL_UINT8) (0xff >> destBitOffset);
                    count = count - (8 - destBitOffset);
                }

                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        *dest = (UGL_UINT8) ((*dest & ~mask) | (*src & mask));
                        break;

                    case UGL_RASTER_OP_AND:
                        *dest &= *src | ~mask;
                        break;

                    case UGL_RASTER_OP_OR:
                        *dest |= *src & mask;
                        break;

                    case UGL_RASTER_OP_XOR:
                        *dest ^= *src & mask;
                        break;
                }

                /* Advance */
                src++;
                dest++;
            }

            /* Handle byte copy */
            if (count >= 8) {
                int nBytes = count >> 3;
                uglCommonByteCopy (src, dest, nBytes, rasterOp);

                /* Advance chunk */
                src   += nBytes;
                dest  += nBytes;
                count &= 0x07;
            }

            /* Handle left over bits */
            if (count > 0) {
                mask = (UGL_UINT8) (0xff << (8 - count));
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        *dest = (UGL_UINT8) ((*dest & ~mask) | (*src & mask));
                        break;

                    case UGL_RASTER_OP_AND:
                        *dest &= *src | ~mask;
                        break;

                    case UGL_RASTER_OP_OR:
                        *dest |= *src & mask;
                        break;

                    case UGL_RASTER_OP_XOR:
                        *dest ^= *src & mask;
                        break;
                }
            }
        }
        else {

            /* Non-bit aligned offsets */
            /* TODO */
        }
    }
    else {

        /* Backward copy */
        /* TODO */
    }
}

