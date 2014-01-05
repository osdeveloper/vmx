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

/* udcmset.c - Memory set functions for graphics */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcomm.h>

/******************************************************************************
 *
 * uglCommonMemSet - Fill memory with color data
 *
 * RETURNS: N/A
 *
 */

UGL_VOID uglCommonMemSet (
    void *         pBuf,
    int            offset,
    int            nItems,
    int            bpp,
    UGL_COLOR *    pColor,
    UGL_RASTER_OP  rasterOp
    ) {

    if (rasterOp == UGL_RASTER_OP_COPY && bpp == 8) {
        memset (pBuf + offset, *pColor, nItems);
    }
    else {
        /* TODO */
    }
}

