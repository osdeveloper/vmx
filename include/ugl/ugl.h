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

/* ugl.h - Universal graphics library header */

#ifndef _ugl_h
#define _ugl_h

#include <ugl/ugltypes.h>
#include <ugl/uglclr.h>
#include <ugl/uglinfo.h>
#include <ugl/uglmode.h>
#include <ugl/uglbmp.h>
#include <ugl/uglos.h>
#include <ugl/uglugi.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Mode support functions */
extern UGL_STATUS uglModeAvailGet(UGL_DEVICE_ID devId,
			          UGL_UINT32 *pNumModes,
			          const UGL_MODE **pModeArray);
extern UGL_STATUS uglModeSet(UGL_DEVICE_ID devId, UGL_MODE *pMode);

/* Graphics context support functions */
extern UGL_GC_ID uglGcCreate(UGL_DEVICE_ID devId);
extern UGL_STATUS uglGcCopy(UGL_GC_ID src, UGL_GC_ID dest);
extern UGL_STATUS uglGcDestroy(UGL_GC_ID gc);
extern UGL_STATUS uglGcSet(UGL_DEVICE_ID devId, UGL_GC_ID gc);

/* Pixel support functions */
extern UGL_STATUS uglPixelSet(UGL_GC_ID gc,
			      UGL_POS x,
			      UGL_POS y,
			      UGL_COLOR color);

/* Color support functions */
extern UGL_STATUS uglClutSet(UGL_DEVICE_ID devId,
		             UGL_ORD offset,
		             UGL_ARGB *pColors,
		             UGL_SIZE numColors);
extern UGL_STATUS uglClutGet(UGL_DEVICE_ID devId,
		      	     UGL_ORD offset,
		      	     UGL_ARGB *pColors,
		      	     UGL_SIZE numColors);
extern UGL_STATUS uglColorAllocExt(UGL_DEVICE_ID devId,
			    	   UGL_ARGB *pReqColors,
			    	   UGL_ORD *pIndex,
			    	   UGL_ARGB  *pActualColors,
			    	   UGL_COLOR *pUglColors,
			    	   UGL_SIZE numColors);
extern UGL_STATUS uglColorAlloc(UGL_DEVICE_ID devId,
			 	UGL_ARGB *pReqColors,
			 	UGL_ORD *pIndex,
			 	UGL_COLOR *pUglColors,
			 	UGL_SIZE numColors);
extern UGL_STATUS uglColorFree(UGL_DEVICE_ID devId,
			       UGL_COLOR *pColors,
			       UGL_SIZE numColors);
extern UGL_DDB_ID uglBitmapCreate(UGL_DEVICE_ID devId,
			          UGL_DIB *pDib,
			          UGL_DIB_CREATE_MODE createMode,
			          UGL_COLOR initValue,
			          UGL_MEM_POOL_ID poolId);
extern UGL_STATUS uglBitmapDestroy(UGL_DEVICE_ID devId,
			           UGL_DDB *pDdb,
			           UGL_MEM_POOL_ID poolId);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ugl_h */

