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

#define LARGE_BUF_LIMIT       10

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
    int          sz;
    const void * src;
    void *       dest;
    int          count;

    if (rasterOp == UGL_RASTER_OP_COPY) {
        memcpy (pDest, pSrc, nBytes);
    }
    else {
        sz = (unsigned int) pDest - (unsigned int) pSrc;
        if (sz <= 0 || sz >= nBytes) {

            /* Perform forward copy */
            src   = pSrc;
            dest  = pDest;
            count = nBytes;

            if (rasterOp == UGL_RASTER_OP_AND) {

                /* If larget data set to copy */
                if (count >= LARGE_BUF_LIMIT) {
                    while (((int) dest & 0x03) != 0x00) {
                        *(UGL_UINT8 *) dest &= *(UGL_UINT8 *) src;
                        (UGL_UINT8 *) dest++;
                        (UGL_UINT8 *) src++;
                        count--;
                    }

                    if (((int) src & 0x03) == 0x00) {

                        /* Copy 32-bit chunks */
                        while (count >= 4) {
                            *(UGL_UINT32 *) dest &= *(UGL_UINT32 *) src;
                            (UGL_UINT32 *) dest++;
                            (UGL_UINT32 *) src++;
                            count -= 4;
                        }
                    }
                    else if (((int) src & 0x01) == 0x00) {

                        /* Copy 16-bit chunks */
                        while (count >= 2) {
                            *(UGL_UINT16 *) dest &= *(UGL_UINT16 *) src;
                            (UGL_UINT16 *) dest++;
                            (UGL_UINT16 *) src++;
                            count -= 2;
                        }
                    }
                }

                /* Small copy bytewise */
                while (--count >= 0) {
                    *(UGL_UINT8 *) dest &= *(UGL_UINT8 *) src;
                    (UGL_UINT8 *) dest++;
                    (UGL_UINT8 *) src++;
                }
            }
            else if (rasterOp == UGL_RASTER_OP_OR) {

                /* If larget data set to copy */
                if (count >= LARGE_BUF_LIMIT) {
                    while (((int) dest & 0x03) != 0x00) {
                        *(UGL_UINT8 *) dest |= *(UGL_UINT8 *) src;
                        (UGL_UINT8 *) dest++;
                        (UGL_UINT8 *) src++;
                        count--;
                    }

                    if (((int) src & 0x03) == 0x00) {

                        /* Copy 32-bit chunks */
                        while (count >= 4) {
                            *(UGL_UINT32 *) dest |= *(UGL_UINT32 *) src;
                            (UGL_UINT32 *) dest++;
                            (UGL_UINT32 *) src++;
                            count -= 4;
                        }
                    }
                    else if (((int) src & 0x01) == 0x00) {

                        /* Copy 16-bit chunks */
                        while (count >= 2) {
                            *(UGL_UINT16 *) dest |= *(UGL_UINT16 *) src;
                            (UGL_UINT16 *) dest++;
                            (UGL_UINT16 *) src++;
                            count -= 2;
                        }
                    }
                }

                /* Small copy bytewise */
                while (--count >= 0) {
                    *(UGL_UINT8 *) dest |= *(UGL_UINT8 *) src;
                    (UGL_UINT8 *) dest++;
                    (UGL_UINT8 *) src++;
                }
            }
            else if (rasterOp == UGL_RASTER_OP_XOR) {

                /* If larget data set to copy */
                if (count >= LARGE_BUF_LIMIT) {
                    while (((int) dest & 0x03) != 0x00) {
                        *(UGL_UINT8 *) dest ^= *(UGL_UINT8 *) src;
                        (UGL_UINT8 *) dest++;
                        (UGL_UINT8 *) src++;
                        count--;
                    }

                    if (((int) src & 0x03) == 0x00) {

                        /* Copy 32-bit chunks */
                        while (count >= 4) {
                            *(UGL_UINT32 *) dest ^= *(UGL_UINT32 *) src;
                            (UGL_UINT32 *) dest++;
                            (UGL_UINT32 *) src++;
                            count -= 4;
                        }
                    }
                    else if (((int) src & 0x01) == 0x00) {

                        /* Copy 16-bit chunks */
                        while (count >= 2) {
                            *(UGL_UINT16 *) dest ^= *(UGL_UINT16 *) src;
                            (UGL_UINT16 *) dest++;
                            (UGL_UINT16 *) src++;
                            count -= 2;
                        }
                    }
                }

                /* Small copy bytewise */
                while (--count >= 0) {
                    *(UGL_UINT8 *) dest ^= *(UGL_UINT8 *) src;
                    (UGL_UINT8 *) dest++;
                    (UGL_UINT8 *) src++;
                }
            }
        }
        else {

            /* Backward copy */
            src   = (void *) ((char *) pSrc + nBytes);
            dest  = (void *) ((char *) pDest + nBytes);
            count = nBytes;

            if (rasterOp == UGL_RASTER_OP_AND) {

                /* If larget data set to copy */
                if (count >= LARGE_BUF_LIMIT) {
                    while (((int) dest & 0x03) != 0x00) {
                        (UGL_UINT8 *) dest--;
                        (UGL_UINT8 *) src--;
                        *(UGL_UINT8 *) dest &= *(UGL_UINT8 *) src;
                        count--;
                    }

                    if (((int) src & 0x03) == 0x00) {

                        /* Copy 32-bit chunks */
                        while (count >= 4) {
                            (UGL_UINT32 *) dest--;
                            (UGL_UINT32 *) src--;
                            *(UGL_UINT32 *) dest &= *(UGL_UINT32 *) src;
                            count -= 4;
                        }
                    }
                    else if (((int) src & 0x01) == 0x00) {

                        /* Copy 16-bit chunks */
                        while (count >= 2) {
                            (UGL_UINT16 *) dest--;
                            (UGL_UINT16 *) src--;
                            *(UGL_UINT16 *) dest &= *(UGL_UINT16 *) src;
                            count -= 2;
                        }
                    }
                }

                /* Small copy bytewise */
                while (--count >= 0) {
                    (UGL_UINT8 *) dest--;
                    (UGL_UINT8 *) src--;
                    *(UGL_UINT8 *) dest &= *(UGL_UINT8 *) src;
                }
            }
            else if (rasterOp == UGL_RASTER_OP_OR) {

                /* If larget data set to copy */
                if (count >= LARGE_BUF_LIMIT) {
                    while (((int) dest & 0x03) != 0x00) {
                        (UGL_UINT8 *) dest--;
                        (UGL_UINT8 *) src--;
                        *(UGL_UINT8 *) dest |= *(UGL_UINT8 *) src;
                        count--;
                    }

                    if (((int) src & 0x03) == 0x00) {

                        /* Copy 32-bit chunks */
                        while (count >= 4) {
                            (UGL_UINT32 *) dest--;
                            (UGL_UINT32 *) src--;
                            *(UGL_UINT32 *) dest |= *(UGL_UINT32 *) src;
                            count -= 4;
                        }
                    }
                    else if (((int) src & 0x01) == 0x00) {

                        /* Copy 16-bit chunks */
                        while (count >= 2) {
                            (UGL_UINT16 *) dest--;
                            (UGL_UINT16 *) src--;
                            *(UGL_UINT16 *) dest |= *(UGL_UINT16 *) src;
                            count -= 2;
                        }
                    }
                }

                /* Small copy bytewise */
                while (--count >= 0) {
                    (UGL_UINT8 *) dest--;
                    (UGL_UINT8 *) src--;
                    *(UGL_UINT8 *) dest |= *(UGL_UINT8 *) src;
                }
            }
            else if (rasterOp == UGL_RASTER_OP_XOR) {

                /* If larget data set to copy */
                if (count >= LARGE_BUF_LIMIT) {
                    while (((int) dest & 0x03) != 0x00) {
                        (UGL_UINT8 *) dest--;
                        (UGL_UINT8 *) src--;
                        *(UGL_UINT8 *) dest ^= *(UGL_UINT8 *) src;
                        count--;
                    }

                    if (((int) src & 0x03) == 0x00) {

                        /* Copy 32-bit chunks */
                        while (count >= 4) {
                            (UGL_UINT32 *) dest--;
                            (UGL_UINT32 *) src--;
                            *(UGL_UINT32 *) dest ^= *(UGL_UINT32 *) src;
                            count -= 4;
                        }
                    }
                    else if (((int) src & 0x01) == 0x00) {

                        /* Copy 16-bit chunks */
                        while (count >= 2) {
                            (UGL_UINT16 *) dest--;
                            (UGL_UINT16 *) src--;
                            *(UGL_UINT16 *) dest ^= *(UGL_UINT16 *) src;
                            count -= 2;
                        }
                    }
                }

                /* Small copy bytewise */
                while (--count >= 0) {
                    (UGL_UINT8 *) dest--;
                    (UGL_UINT8 *) src--;
                    *(UGL_UINT8 *) dest ^= *(UGL_UINT8 *) src;
                }
            }
        }
    }
}

