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

/* udgen8.h - Generic 8-Bit graphics support */

#ifndef _udgen8_h
#define _udgen8_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

extern UGL_STATUS uglGeneric8BitPixelSet(UGL_DEVICE_ID devId,
			   	  	 UGL_POINT *p,
			   	  	 UGL_COLOR c);

extern UGL_DDB_ID uglGeneric8BitBitmapCreate(UGL_DEVICE_ID devId,
				             UGL_DIB *pDib,
				             UGL_DIB_CREATE_MODE createMode,
				             UGL_UINT32 initValue,
				             UGL_MEM_POOL_ID poolId);
extern UGL_STATUS uglGeneric8BitBitmapDestroy(UGL_DEVICE_ID devId,
			       	       	      UGL_DDB_ID ddbId,
			       	       	      UGL_MEM_POOL_ID poolId);
extern UGL_STATUS uglGeneric8BitBitmapBlt(UGL_DEVICE_ID devId,
			   	          UGL_DDB_ID srcBmpId,
			   	          UGL_RECT *pSrcRect,
			   	          UGL_DDB_ID destBmpId,
			   	          UGL_POINT *pDestPoint);
extern UGL_STATUS uglGeneric8BitBitmapWrite(UGL_DEVICE_ID devId,
			     	    	    UGL_DIB *pDib,
			     	    	    UGL_RECT *pSrcRect,
			     	    	    UGL_DDB_ID ddbId,
			     	    	    UGL_POINT *pDestPoint);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udgen8_h */

