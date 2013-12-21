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

/* ugldib.h - Universal graphics library bitmap header */

#ifndef _ugldib_h
#define _ugldib_h

#define UGL_DIB_INIT_NONE               0
#define UGL_DIB_INIT_VALUE              1
#define UGL_DIB_INIT_DATA               2

/* Bitmap types */
#define UGL_DDB_TYPE                    0
#define UGL_MDDB_TYPE                   1
#define UGL_TDDB_TYPE                   2

/* Cursor color keys */
#define UGL_CURSOR_COLOR_TRANSPARENT    255
#define UGL_CURSOR_COLOR_INVERT         254

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Color bitmap */
typedef struct ugl_dib {
    UGL_SIZE          width;            /* Bitmap width */
    UGL_SIZE          height;           /* Bitmap height */
    UGL_SIZE          stride;           /* Distance between 2 scanlines */
    UGL_INT32         imageFormat;      /* Image format */
    UGL_COLOR_FORMAT  colorFormat;      /* Visual mode */
    UGL_SIZE          clutSize;         /* Color lookup table size */
    void *            pClut;            /* Color lookup table */
    void *            pData;            /* Image data */
} UGL_DIB;

typedef UGL_DIB *   UGL_DIB_ID;
typedef UGL_UINT32  UGL_DIB_CREATE_MODE;

/* Monochrome bitmap */
typedef struct ugl_mdib {
    UGL_SIZE          width;            /* Bitmap width */
    UGL_SIZE          height;           /* Bitmap height */
    UGL_SIZE          stride;           /* Distance between 2 scanlines */
    void *            pData;            /* Image data */
} UGL_MDIB;

/* Cursor bitmap */
typedef struct ugl_cdib {
    UGL_SIZE    width;                  /* Cursor width */
    UGL_SIZE    height;                 /* Cursor height */
    UGL_SIZE    stride;                 /* Distance between 2 scanlines */
    UGL_POINT   hotSpot;                /* Coordinate for hot-spot */
    UGL_SIZE    clutSize;               /* Color lookup table size */
    UGL_ARGB *  pClut;                  /* Color lookup table */
    UGL_UINT8 * pData;                  /* Image data */
} UGL_CDIB;

/* Generic bitmap header */
typedef struct ugl_bmap_header {
    UGL_UINT16  type;                   /* Bitmap type */
    UGL_UINT16  width;                  /* Bitmap width */
    UGL_UINT16  height;                 /* Bitmap height */
} UGL_BMAP_HEADER;

/* Device dependent bitmap */
typedef UGL_BMAP_HEADER  UGL_DDB;
typedef UGL_DDB *        UGL_DDB_ID;

/* Device dependent monochrome bitmap */
typedef UGL_BMAP_HEADER  UGL_MDDB;
typedef UGL_MDDB *       UGL_MDDB_ID;

/* Deveice dependent transparent bitmap */
typedef UGL_BMAP_HEADER  UGL_TDDB;
typedef UGL_TDDB *       UGL_TDDB_ID;

/* Deveice dependent cursor bitmap */
typedef UGL_BMAP_HEADER  UGL_CDDB;
typedef UGL_CDDB *       UGL_CDDB_ID;

/* Generic bitmap id */
typedef UGL_BMAP_HEADER * UGL_BITMAP_ID;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ugldib_h */

