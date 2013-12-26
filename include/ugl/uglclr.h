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

/* uglclr.h - Universal graphics library color info */

#ifndef _uglclr_h
#define _uglclr_h

/* UGL_COLOR_MODE */
#define UGL_DIRECT              0               /* Direct color */
#define UGL_INDEXED_1           1               /* 1-bit */
#define UGL_INDEXED_2           2               /* 2-bit */
#define UGL_INDEXED_4           4               /* 4-bit */
#define UGL_INDEXED_8           8               /* 8-bit */
#define UGL_TRANS_COLOR_KEY     0x80000000
#define UGL_INDEX_MASK          0xff

/* UGL_COLOR_FORMAT */
#define UGL_CM_DEVICE_32        0x80000000
#define UGL_CM_DEVICE           0x90000000
#define UGL_CM_MONOCHROME       0xa0000000
#define UGL_CM_ARGB             0x01000000
#define UGL_CM_ABGR             0x02000000
#define UGL_CM_RGBA             0x03000000
#define UGL_CM_BGRA             0x04000000
#define UGL_CM_YUV              0x05000000
#define UGL_CM_CMYK             0x06000000
#define UGL_CM_YIQ              0x07000000

#define UGL_ARGB8888            (UGL_CM_ARGB | 0x00048888)
#define UGL_RGB565              (UGL_CM_ARGB | 0x00020565)

/* Color model */
#define UGL_CMODEL_INDEXED              1                 /* Indexed color */
#define UGL_CMODEL_DIRECT               2                 /* Direct color */
#define UGL_CMODEL_TRUE                 UGL_CMODEL_DIRECT

/* Short form color format */
#define UGL_DEVICE_COLOR_32             UGL_CM_DEVICE_32
#define UGL_DEVICE_COLOR                UGL_CM_DEVICE

/* Color space */
#define UGL_CSPACE_RGB                  1
#define UGL_CSPACE_YUV                  2
#define UGL_CSPACE_YIQ                  3
#define UGL_CSPACE_CMYK                 4
#define UGL_CSPACE_MONO                 5
#define UGL_CSPACE_GRAY                 6

/* Clut */
#define UGL_CLUT_WRITE                  0x0001

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Color types */
typedef UGL_INT32   UGL_COLOR_MODEL;
typedef UGL_UINT32  UGL_COLOR_FORMAT;
typedef UGL_UINT32  UGL_ARGB;
typedef UGL_ARGB    UGL_RGB;

/* Draw color */
#define UGL_COLOR_TRANSPARENT           ((UGL_COLOR) 0x00010101L)

/* Macros */

/******************************************************************************
 *
 * UGL_ARGB_ALPHA - Get alpha (transparency) component from color info
 *
 * RETURNS: Alpha value
 */

#define UGL_ARGB_ALPHA(argb)    (UGL_ARGB)(((argb) >> 24) & 0xff)

/******************************************************************************
 *
 * UGL_ARGB_RED - Get red component from color info
 *
 * RETURNS: Red value
 */

#define UGL_ARGB_RED(argb)      (UGL_ARGB)(((argb) >> 16) & 0xff)

/******************************************************************************
 *
 * UGL_ARGB_GREEN - Get green component from color info
 *
 * RETURNS: Green value
 */

#define UGL_ARGB_GREEN(argb)    (UGL_ARGB)(((argb) >> 8) & 0xff)

/******************************************************************************
 *
 * UGL_ARGB_BLUE - Get blue component from color info
 *
 * RETURNS: Green value
 */

#define UGL_ARGB_BLUE(argb)     (UGL_ARGB)((argb) & 0xff)

/******************************************************************************
 *
 * UGL_MAKE_ARGB - Genereate color info from argb values
 *
 * RETURNS: Argb balue
 */

#define UGL_MAKE_ARGB(a, r, g, b)                                             \
        ( ((UGL_ARGB)(a) << 24) |                                             \
          ((UGL_ARGB)(r) << 16) |                                             \
          ((UGL_ARGB)(g) << 8 ) |                                             \
           (UGL_ARGB)(b)      )

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglclr_h */

