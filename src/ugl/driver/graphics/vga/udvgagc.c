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

/* udvgagc.c - Universal graphics library graphics context support */

#include <arch/sysArchLib.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/graphics/vga/udvga.h>

/******************************************************************************
 *
 * uglVgaGcSet - Set graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglVgaGcSet (
    UGL_DEVICE_ID  devId,
    UGL_GC_ID      gc
    ) {
    UGL_VGA_DRIVER * pDrv;
    UGL_RASTER_OP    rasterOp;

    /* Get driver since it is first in the device struct */
    pDrv = (UGL_VGA_DRIVER *) devId;

    /* Get raster operation from new gc */
    rasterOp = gc->rasterOp;

    /* Set raster operation in hardware if different */
    if (rasterOp != pDrv->rasterOp) {
        switch(rasterOp) {
            case UGL_RASTER_OP_COPY:
                UGL_OUT_WORD (0x3ce, 0x0003);
                break;

            case UGL_RASTER_OP_AND:
                UGL_OUT_WORD (0x3ce, 0x0803);
                break;

            case UGL_RASTER_OP_OR:
                UGL_OUT_WORD (0x3ce, 0x1003);
                break;

            case UGL_RASTER_OP_XOR:
                UGL_OUT_WORD (0x3ce, 0x1803);
                break;

            default:
                return (UGL_STATUS_ERROR);
        }

      /* Update raster operation */
      pDrv->rasterOp = rasterOp;
    }

    /* Call generic method */
    return uglGenericGcSet(devId, gc);
}

