/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 - 2009 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* uglbmp.h - Universal graphics library bitmap header */

#ifndef _uglbmp_h
#define _uglbmp_h

#define UGL_DIB_INIT_NONE		0
#define UGL_DIB_INIT_VALUE		1
#define UGL_DIB_INIT_DATA		2

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ugl_dib {
  UGL_SIZE		width;		/* Bitmap width */
  UGL_SIZE		height;		/* Bitmap height */
  UGL_SIZE		stride;		/* Distance between 2 scanlines */
  UGL_INT32		imageFormat;	/* Image format */
  UGL_COLOR_FORMAT	colorFormat;	/* Visual mode */
  UGL_SIZE		clutSize;	/* Color lookup table size */
  void			*pClut;		/* Color lookup table */
  void			*pData;		/* Image data */
} UGL_DIB;

typedef UGL_DIB *UGL_DIB_ID;
typedef UGL_UINT32 UGL_DIB_CREATE_MODE;	/* Creation mode */

typedef struct ugl_bmap_header {
  UGL_UINT16	width;			/* Bitmap width */
  UGL_UINT16	height;			/* Bitmap height */
} UGL_BMAP_HEADER;

typedef UGL_BMAP_HEADER UGL_DDB;
typedef UGL_DDB *UGL_DDB_ID;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglbmp_h */

