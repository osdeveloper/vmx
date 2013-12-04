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

/* udgen.h - Universal graphics library generic driver */

#ifndef _udgen_h
#define _udgen_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcclr.h>

typedef struct ugl_generic_driver {
  UGL_UGI_DRIVER	ugi;		/* UGI driver (required) */
  UGL_GC_ID		gc;		/* Graphics context */
  UGL_CLUT*		pClut;		/* Palette */
  void*			fbAddress;	/* Frame buffer address */
} UGL_GENERIC_DRIVER;

typedef struct ugl_gen_bitmap {
  UGL_BMAP_HEADER header;
} UGL_GENERIC_BMAP;

typedef UGL_GENERIC_BMAP *UGL_BMAP_ID;

typedef struct ugl_gen_ddb {
  UGL_BMAP_HEADER	header;		/* Header */
  UGL_UINT16		colorDepth;	/* Bits-per-pixel */
  UGL_UINT16		stride;		/* Pixels per line */
  UGL_UINT32		transColorKey;	/* Transparent color key */
  void			*pData;		/* Image data */
} UGL_GEN_DDB;

/* Mode support */
extern UGL_INT32 uglGenericModeFind(UGL_MODE *pList,
			            UGL_MODE *pReqMode,
			            UGL_UINT32 numModes);

/* Graphics context support */
extern UGL_GC_ID uglGenericGcCreate(UGL_DEVICE_ID devId);
extern UGL_STATUS uglGenericGcCopy(UGL_DEVICE_ID devId,
			           UGL_GC_ID srcGcId,
			           UGL_GC_ID destGcId);
extern UGL_STATUS uglGenericGcDestroy(UGL_DEVICE_ID devId, UGL_GC_ID gc);
extern UGL_STATUS uglGenericGcSet(UGL_DEVICE_ID devId, UGL_GC_ID gc);

/* Palette support */
extern UGL_STATUS uglGenericClutCreate(UGL_GENERIC_DRIVER *pDrv,
				       UGL_SIZE numColors);
extern UGL_STATUS uglGenericClutSet(UGL_GENERIC_DRIVER *pDrv,
			     	    UGL_ORD offset,
			     	    UGL_ARGB *pColors,
			     	    UGL_SIZE numColors);
extern UGL_STATUS uglGenericClutGet(UGL_GENERIC_DRIVER *pDrv,
			     	    UGL_ORD offset,
			     	    UGL_ARGB *pColors,
			     	    UGL_SIZE numColors);
extern UGL_STATUS uglGenericClutMapNearest(UGL_GENERIC_DRIVER *pDrv,
				    	  UGL_ARGB *pMapColors,
				    	  UGL_ARGB  *pActualColors,
				    	  UGL_COLOR *pUglColors,
				    	  UGL_SIZE numColors);
extern UGL_STATUS uglGenericColorAllocIndexed(UGL_DEVICE_ID devId,
				     	      UGL_ARGB *pReqColors,
				     	      UGL_ORD *pIndex,
				     	      UGL_ARGB *pActualColors,
				     	      UGL_COLOR *pUglColors,
				     	      UGL_SIZE numColors);
extern UGL_STATUS uglGenericColorFreeIndexed(UGL_DEVICE_ID devId,
				      	     UGL_COLOR *pColors,
				      	     UGL_SIZE numColors);
extern UGL_STATUS uglGenericClutDestroy(UGL_GENERIC_DRIVER *pDrv);

extern UGL_STATUS uglGenericClipDdb(UGL_DEVICE_ID devId,
			     	    UGL_RECT *pClipRect,
			     	    UGL_BMAP_ID *pSrcBmpId,
			     	    UGL_RECT *pSrcRect,
			     	    UGL_BMAP_ID *pDestBmpId,
			     	    UGL_POINT *pDestPoint);
extern UGL_BOOL uglGenericClipDibToDdb(UGL_DEVICE_ID devId,
				       UGL_DIB *pDib,
				       UGL_RECT *pSrcRect,
				       UGL_BMAP_ID *pBmpId,
				       UGL_POINT *pDestPoint);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udgen_h */

