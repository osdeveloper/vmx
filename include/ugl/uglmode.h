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

/* uglmode.h - Universal graphics library display mode */

#ifndef _uglmode_h
#define _uglmode_h

#ifndef _ASMLANGUAGE

/* Color modes */
#define UGL_MODE_DIRECT_COLOR           0x00000001
#define UGL_MODE_TRUE_COLOR             0x00000001
#define UGL_MODE_INDEXED_COLOR          0x00000002
#define UGL_MODE_PSEUDO_COLOR           0x00000002
#define UGL_MODE_MONO                   0x00000004
#define UGL_MODE_GRAY_SCALE             0x00000008

/* Monitor types */
#define UGL_MODE_CRT                    1
#define UGL_MODE_FLAT_PANEL             2
#define UGL_MODE_DUAL_OUTPUT            3

/* Video modes */
#define UGL_NTSC                        0
#define UGL_PAL                         1
#define UGL_SECAM                       2

/* Display types */
#define UGL_COLOR_CRT                   0
#define UGL_MONO_CRT                    1
#define UGL_S_VIDEO                     2
#define UGL_COMPOSITE                   3
#define UGL_S_VIDEO_COMPOSITE           4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ugl_mode {
    UGL_CHAR *  name;                   /* Mode name */
    UGL_UINT16  width;                  /* Display width */
    UGL_UINT16  height;                 /* Display height */
    UGL_UINT8   colorDepth;             /* BitsPerPixel */
    UGL_ORD     refreshRate;            /* Monitor refresh rate */
    UGL_UINT16  monitorType;            /* Monitor type */
    UGL_UINT32  flags;                  /* Misc Flags */
    UGL_UINT8   clock;                  /* PixelClock used */
    UGL_UINT32  hDisplay;
    UGL_UINT32  hSyncStart;
    UGL_UINT32  hSyncEnd;
    UGL_UINT32  hTotal;
    UGL_UINT32  hSkew;
    UGL_UINT32  vDisplay;
    UGL_UINT32  vSyncStart;
    UGL_UINT32  vSyncEnd;
    UGL_UINT32  vTotal;
} UGL_MODE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglmode_h */

