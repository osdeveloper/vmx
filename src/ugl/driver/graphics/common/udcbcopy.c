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

            /* Copy start bits */
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

                /* Advance to next byte */
                src++;
                dest++;
            }

            /* Copy byte wise */
            if (count >= 8) {
                int nBytes = count >> 3;
                uglCommonByteCopy (src, dest, nBytes, rasterOp);

                /* Advance chunk */
                src   += nBytes;
                dest  += nBytes;
                count &= 0x07;
            }

            /* Copy left over bits */
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
#ifdef UGL_BIG_ENDIAN
            /* TODO */
#else
            UGL_UINT8 * src   = (UGL_UINT8 *) pSrcStart;
            UGL_UINT8 * dest  = (UGL_UINT8 *) pDestStart;
            int         count = nBits;
            UGL_UINT8   value1;
            UGL_UINT8   value2;
            UGL_UINT8   mask;
            int         leftShift;
            int         rightShift;

            if (count + destBitOffset < 8) {
                mask  = (UGL_UINT8) ((0xff >> destBitOffset) &
                                     (0xff << (8 - destBitOffset - count)));
                count = 0;
            }
            else {
                mask  = (UGL_UINT8) (0xff >> destBitOffset);
                count = count - (8 - destBitOffset);
            }

            if (destBitOffset > srcBitOffset) {

                /* Right shift source */
                rightShift = destBitOffset - srcBitOffset;
                leftShift  = 8 - rightShift;
                value1     = (UGL_UINT8) (*src >> rightShift);
            }
            else {

                /* Left shift source */
                leftShift  = srcBitOffset - destBitOffset;
                rightShift = 8 - leftShift;
                value1     = (UGL_UINT8) (*src << leftShift);
                src++;
                value1 |= *src >> rightShift;
            }

            /* Copy remaining start bits */
            switch (rasterOp) {
                case UGL_RASTER_OP_COPY:
                    *dest = (UGL_UINT8) ((*dest & ~mask) | (value1 & mask));
                    break;

                case UGL_RASTER_OP_AND:
                    *dest &= value1 | ~mask;
                    break;

                case UGL_RASTER_OP_OR:
                    *dest |= value1 & mask;
                    break;

                case UGL_RASTER_OP_XOR:
                    *dest ^= value1 & mask;
                    break;
            }

            /* Advance to next byte */
            value1 = *src;
            src++;
            dest++;

            /* Copy data byte wise */
            if (count >= 8) {
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 << leftShift);
                            value1 = *src;
                            *dest = (UGL_UINT8) ((value1 >> rightShift) |
                                                  value2);

                            /* Advance to next byte */
                            src++;
                            dest++;
                            count -= 8;
                        }
                        break;

                    case UGL_RASTER_OP_AND:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 << leftShift);
                            value1 = *src;
                            *dest &= (value1 >> rightShift) | value2;

                            /* Advance to next byte */
                            src++;
                            dest++;
                            count -= 8;
                        }
                        break;

                    case UGL_RASTER_OP_OR:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 << leftShift);
                            value1 = *src;
                            *dest |= (value1 >> rightShift) | value2;

                            /* Advance to next byte */
                            src++;
                            dest++;
                            count -= 8;
                        }
                        break;

                    case UGL_RASTER_OP_XOR:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 << leftShift);
                            value1 = *src;
                            *dest ^= (value1 >> rightShift) | value2;

                            /* Advance to next byte */
                            src++;
                            dest++;
                            count -= 8;
                        }
                        break;
                }
            }

            /* Copy remaining end bits */
            if (count > 0) {
                value1 = value1 << leftShift | *src >> rightShift;
                mask   = (UGL_UINT8) (0xff << (8 - count));
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        *dest = (UGL_UINT8) ((*dest & ~mask) | (value1 & mask));
                        break;

                    case UGL_RASTER_OP_AND:
                        *dest &= value1 | ~mask;
                        break;

                    case UGL_RASTER_OP_OR:
                        *dest |= value1 & mask;
                        break;

                    case UGL_RASTER_OP_XOR:
                        *dest ^= value1 & mask;
                        break;
                }
            }
#endif
        }
    }
    else {

        /* Backward copy */
        if (srcBitOffset == destBitOffset) {

            /* Bit aligned offsets */
            int         count   = nBits;
            UGL_UINT8 * src     = (UGL_UINT8 *) pSrcStart +
                                  ((srcBitOffset + count - 1) >> 3);
            UGL_UINT8 * dest    = (UGL_UINT8 *) pDestStart +
                                  ((destBitOffset + count - 1) >> 3);
            int         endBits = ((srcBitOffset + count) & 0x07);
            UGL_UINT8   mask;

            if (endBits == 0) {
                endBits = 8;
            }

            /* Copy end bits */
            if (endBits > 0) {
                if (endBits > count) {
                    mask = (UGL_UINT8) ((0xff << (8 - endBits)) &
                                        (0xff >> (endBits - count)));
                    count = 0;
                }
                else {
                    mask = (UGL_UINT8) (0xff << (8 - endBits));
                    count = count - endBits;
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

                /* Advance to previous byte */
                src--;
                dest--;
            }

            /* Copy byte wise */
            if (count >= 8) {
                int nBytes = count >> 3;
                uglCommonByteCopy (src - count + 1, dest - count + 1,
                                   nBytes, rasterOp);

                /* Advance chunk */
                src   -= nBytes;
                dest  -= nBytes;
                count &= 0x07;
            }

            /* Copy left over start bits */
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
#ifdef UGL_BIG_ENDIAN
            /* TODO */
#else
            int         count       = nBits;
            UGL_UINT8 * src         = (UGL_UINT8 *) pSrcStart +
                                      ((srcBitOffset + count - 1) >> 3);
            UGL_UINT8 * dest        = (UGL_UINT8 *) pDestStart +
                                      ((destBitOffset + count - 1) >> 3);
            int         srcEndBits  = ((srcBitOffset + count) & 0x07);
            int         destEndBits = ((destBitOffset + count) & 0x07);
            UGL_UINT8   value1;
            UGL_UINT8   value2;
            UGL_UINT8   mask;
            int         leftShift;
            int         rightShift;

            if (srcEndBits == 0) {
                srcEndBits = 8;
            }

            if (destEndBits == 0) {
                destEndBits = 8;
            }

            if (destEndBits > count) {
                mask  = (UGL_UINT8) ((0xff << (8 - destEndBits)) &
                                     (0xff >> (destEndBits - count)));
                count = 0;
            }
            else {
                mask  = (UGL_UINT8) (0xff << (8 - destEndBits));
                count = count - destEndBits;
            }

            if (destEndBits > srcEndBits) {

                /* Right shift source */
                rightShift = destEndBits - srcEndBits;
                leftShift  = 8 - rightShift;
                value1     = (UGL_UINT8) (*src >> rightShift);
                src--;
                value1 |= *src << leftShift;
            }
            else {

                /* Left shift source */
                leftShift  = srcEndBits - destEndBits;
                rightShift = 8 - leftShift;
                value1     = (UGL_UINT8) (*src << leftShift);
            }

            /* Copy remaining end bits */
            switch (rasterOp) {
                case UGL_RASTER_OP_COPY:
                    *dest = (UGL_UINT8) ((*dest & ~mask) | (value1 & mask));
                    break;

                case UGL_RASTER_OP_AND:
                    *dest &= value1 | ~mask;
                    break;

                case UGL_RASTER_OP_OR:
                    *dest |= value1 & mask;
                    break;

                case UGL_RASTER_OP_XOR:
                    *dest ^= value1 & mask;
                    break;
            }

            /* Advance to previous byte */
            value1 = *src;
            src--;
            dest--;

            /* Copy data byte wise */
            if (count >= 8) {
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 >> rightShift);
                            value1 = *src;
                            *dest = (UGL_UINT8) ((value1 << leftShift) |
                                                  value2);

                            /* Advance to previous byte */
                            src--;
                            dest--;
                            count -= 8;
                        }
                        break;

                    case UGL_RASTER_OP_AND:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 >> rightShift);
                            value1 = *src;
                            *dest &= (value1 << leftShift) | value2;

                            /* Advance to previous byte */
                            src--;
                            dest--;
                            count -= 8;
                        }
                        break;

                    case UGL_RASTER_OP_OR:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 >> rightShift);
                            value1 = *src;
                            *dest |= (value1 << leftShift) | value2;

                            /* Advance to previous byte */
                            src--;
                            dest--;
                            count -= 8;
                        }
                        break;

                    case UGL_RASTER_OP_XOR:
                        while (count >= 8) {
                            value2 = (UGL_UINT8) (value1 >> rightShift);
                            value1 = *src;
                            *dest ^= (value1 << leftShift) | value2;

                            /* Advance to previous byte */
                            src--;
                            dest--;
                            count -= 8;
                        }
                        break;
                }
            }

            /* Copy remaining start bits */
            if (count > 0) {
                value1 = value1 >> rightShift | *src << leftShift;
                mask   = (UGL_UINT8) (0xff >> (8 - count));
                switch (rasterOp) {
                    case UGL_RASTER_OP_COPY:
                        *dest = (UGL_UINT8) ((*dest & ~mask) | (value1 & mask));
                        break;

                    case UGL_RASTER_OP_AND:
                        *dest &= value1 | ~mask;
                        break;

                    case UGL_RASTER_OP_OR:
                        *dest |= value1 & mask;
                        break;

                    case UGL_RASTER_OP_XOR:
                        *dest ^= value1 & mask;
                        break;
                }
            }
#endif
        }
    }
}

