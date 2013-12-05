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

/* uglinfo.h - Universal graphics library video mode info */

#ifndef _uglinfo_h
#define _uglinfo_h

/* UGL_INFO_REQ */
#define UGL_FB_INFO_REQ                 1       /* Request frambuffer info */
#define UGL_COLOR_INFO_REQ              2       /* Request color info */
#define UGL_MODE_INFO_REQ               3       /* Request mode info */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef UGL_UINT32  UGL_INFO_REQ;

typedef struct ugl_fb_info {
    UGL_UINT32  width;                  /* Width in pixels */
    UGL_UINT32  height;                 /* Height in pixels */
    void *      fbAddress;              /* Frame buffer address */
    UGL_UINT32  displayMemAvail;        /* Avilable display memory */
    UGL_UINT16  flags;                  /* Flags */
} UGL_FB_INFO;

typedef struct ugl_color_info {
    UGL_UINT16  depth;                  /* Color depth */
    UGL_UINT16  clutSize;               /* Size of color lookup-table */
    UGL_UINT16  cmodel;                 /* Color model */
    UGL_UINT16  cspace;                 /* Color space */
    UGL_UINT16  flags;                  /* Flags */
} UGL_COLOR_INFO;

typedef struct ugl_mode_info {
    UGL_UINT16        width;            /* Width in pixels */
    UGL_UINT16        height;           /* Height in pixels */
    UGL_UINT8         colorDepth;       /* Color depth */
    UGL_UINT16        clutSize;         /* Size of palette table */
    UGL_COLOR_MODEL   colorModel;       /* Color model */
    UGL_COLOR_FORMAT  colorFormat;      /* Color format */
    void *            fbAddress;        /* Frame buffer address */
    UGL_UINT32        displayMemAvail;  /* Avaiable display memory */
    UGL_UINT32        flags;            /* Flags */
} UGL_MODE_INFO;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglinfo_h */

