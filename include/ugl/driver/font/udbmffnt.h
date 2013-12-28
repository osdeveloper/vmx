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

/* udbmffnt.h - Bitmap font driver for Universal Graphics Library */

#ifndef _udbmffnt_h
#define _udbmffnt_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <ugl/ugltypes.h>
#include <ugl/uglfont.h>
#include <ugl/uglugi.h>

/* Types */

typedef struct ugl_bmf_font_driver {
    UGL_FONT_DRIVER  header;
    UGL_ORD          textOrigin;
    UGL_LOCK_ID      lockId;
} UGL_BMF_FONT_DRIVER;

/* Functions */

/******************************************************************************
 *
 * uglBMFFontDriverCreate - Create bitmap font driver
 *
 * RETURNS: UGL_FONT_DRIVER_ID or UGL_NULL
 */

UGL_FONT_DRIVER_ID uglBMFFontDriverCreate (
    UGL_DEVICE_ID  devId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udbmffnt_h */

