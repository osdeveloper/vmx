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

/* udvgabmp.c - Vga bitmap support */

#include <stdlib.h>
#include <string.h>

#include <vmx.h>
#include <arch/sysArchLib.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/graphics/vga/udvga.h>

/*******************************************************************************
 * uglVgaBitmapCreate - Create vga bitmap
 *
 * RETURNS: Pointer to bitmap
 ******************************************************************************/

UGL_DDB_ID uglVgaBitmapCreate(UGL_DEVICE_ID devId,
			      UGL_DIB *pDib,
			      UGL_DIB_CREATE_MODE createMode,
			      UGL_UINT32 initValue,
			      UGL_MEM_POOL_ID poolId)
{
  UGL_VGA_DRIVER *pDrv;
  UGL_VGA_DDB *pVgaBmp;
  UGL_UINT32 i, numPlanes, planeSize;
  UGL_SIZE width, height;
  UGL_UINT8 *pPlane;
  UGL_UINT32 planeShift;
  UGL_RECT srcRect;
  UGL_POINT destPoint;
  UGL_STATUS status;

  /* Get driver first in device struct */
  pDrv = (UGL_VGA_DRIVER *) devId;

  /* Get number of bitplanes */
  numPlanes = pDrv->colorPlanes;

  /* Get bitmap info, from screen if NULL DIB */
  if (pDib == UGL_NULL) {
    width = devId->pMode->Width;
    height = devId->pMode->Height;
  }

  /* Else get info from device independent bitmap */
  else {
    width = pDib->width;
    height = pDib->height;
  }

  /* Calculate plane size including 1 shift byte for each scanline */
  planeSize = ((width + 7) / 8 + 1) * height;

  /* Allocate storage for header and color planes */
  pVgaBmp = (UGL_VGA_DDB *) UGL_PART_MALLOC(poolId,
	sizeof(UGL_VGA_DDB) +				/* Header */
	numPlanes * sizeof(UGL_UINT8 *) +		/* Plane array */
       	numPlanes * planeSize				/* bitplanes */
	);
  if (pVgaBmp == NULL)
    return UGL_NULL;

  /* Setup color planes */
  /* After the plane pointer array are the storage are of the bitplanes */
  /* Setup all pointers in the array to point to these areas */
  pVgaBmp->pPlaneArray = (UGL_UINT8 **) (((UGL_UINT8 *) pVgaBmp) +
	sizeof(UGL_VGA_DDB));
  pPlane = (UGL_UINT8 *) &pVgaBmp->pPlaneArray[numPlanes];
  for (i = 0; i < numPlanes; i++) {
    pVgaBmp->pPlaneArray[i] = pPlane;
    pPlane += planeSize;
  }

  /* Initialize contents of color planes */
  switch(createMode) {

    /* Fill all planes with initial value */
    case UGL_DIB_INIT_VALUE:

      pVgaBmp->colorDepth = devId->pMode->Depth;
      pVgaBmp->header.type = UGL_DDB_TYPE;
      pVgaBmp->header.width = width;
      pVgaBmp->header.height = height;
      pVgaBmp->stride = width;
      pVgaBmp->shiftValue = 0;
      planeShift = 0x01;

      for (i = 0; i < numPlanes; i++) {

        if ((UGL_UINT8)(initValue & planeShift) != 0)
          memset(pVgaBmp->pPlaneArray[i], 0xff, planeSize);
        else
          memset(pVgaBmp->pPlaneArray[i], 0x00, planeSize);

        planeShift <<= 1;
      }

    break;

    /* Init from general bitmap */
    case UGL_DIB_INIT_DATA:

      pVgaBmp->colorDepth = devId->pMode->Depth;
      pVgaBmp->header.type = UGL_DDB_TYPE;
      pVgaBmp->header.width = width;
      pVgaBmp->header.height = height;
      pVgaBmp->stride = width;
      pVgaBmp->shiftValue = 0;

      /* Clear destination */
      for (i = 0; i < numPlanes; i++) {

        if ((UGL_UINT8)(initValue & planeShift) != 0)
          memset(pVgaBmp->pPlaneArray[i], 0xff, planeSize);
        else
          memset(pVgaBmp->pPlaneArray[i], 0x00, planeSize);

        planeShift <<= 1;
      }

      /* Read from source */
      srcRect.left = 0;
      srcRect.top = 0;
      srcRect.right = width - 1;
      srcRect.bottom = height - 1;
      destPoint.x = 0;
      destPoint.y = 0;
      status = (*devId->bitmapWrite)(devId, pDib, &srcRect,
				     (UGL_DDB_ID) pVgaBmp, &destPoint);

      if (status != UGL_STATUS_OK) {
        (*devId->bitmapDestroy)(devId, (UGL_DDB_ID) pVgaBmp, poolId);
	return UGL_NULL;
      }

    break;

    /* None */
    default:
    break;
  }

  return (UGL_DDB_ID) pVgaBmp;
}

/*******************************************************************************
 * uglVgaBitmapDestroy - Free vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglVgaBitmapDestroy(UGL_DEVICE_ID devId,
			       UGL_DDB_ID ddbId,
			       UGL_MEM_POOL_ID poolId)
{
  /* Free memory */
  UGL_PART_FREE(poolId, ddbId);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglVgaShiftBitmap - Shift bitmap
 *
 * RETURNS: N/A
 ******************************************************************************/

UGL_LOCAL void uglVgaShiftBitmap(UGL_DEVICE_ID devId,
				 UGL_BMAP_HEADER *pBmp,
				 UGL_UINT8 **pPlaneArray,
				 UGL_INT32 prevShift,
				 UGL_INT32 shift)
{
  UGL_VGA_DRIVER *pDrv;
  UGL_INT32 x, y, planeIndex, numPlanes, bytesPerLine;
  UGL_UINT8 *ptr;
  register UGL_UINT16 shiftReg;
  register UGL_UINT8 shiftRest;
  register UGL_INT16 shiftDelta;

  /* Get driver first in devide struct */
  pDrv = (UGL_VGA_DRIVER *) devId;

  /* Setup variables for shift */
  shiftDelta = (UGL_INT16) (shift - prevShift);
  numPlanes = pDrv->colorPlanes;
  bytesPerLine = (pBmp->width + 7) / 8 + 1;

  /* shiftDelta > 0 */
  if (shiftDelta > 0) {

    /* Recalculate shift delta */
    shiftDelta = 8 - shiftDelta;

    /* For all planes */
    for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

      /* Get next plane pointer */
      ptr = pPlaneArray[planeIndex];
      if (ptr == UGL_NULL)
        break;

      /* Calculate height */
      y = pBmp->height + 1;

      /* Over y */
      while (--y) {

        shiftRest = 0;

	/* Calculate length */
        x = bytesPerLine + 1;

	/* Over x */
        while (--x) {

          shiftReg = *ptr;
          shiftReg <<= shiftDelta;
          *(ptr++) = shiftRest | UGL_HI_BYTE(shiftReg);
          shiftRest = UGL_LO_BYTE(shiftReg);

        }/* End over x */

      } /* End over y */

    } /* End for all planes */

  } /* End if shiftDelta > 0 */

  /* Else shiftDelta <= 0 */
  else {

    /* Recalculate shift delta */
    shiftDelta = -shiftDelta;

    /* For all planes */
    for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

      /* Get next plane pointer */
      ptr = pPlaneArray[planeIndex];
      if (ptr == UGL_NULL)
        break;

      /* Calculate height */
      y = pBmp->height + 1;

      /* Over y */
      while (--y) {

        shiftReg = *ptr;
        shiftReg <<= shiftDelta;
        shiftRest = UGL_LO_BYTE(shiftReg);

	/* Calculate length */
        x = bytesPerLine;

	/* Over x */
        while (--x) {

          shiftReg = ptr[1];
          shiftReg <<= shiftDelta;
          *(ptr++) = shiftRest | UGL_HI_BYTE(shiftReg);
          shiftRest = UGL_LO_BYTE(shiftReg);

        }/* End over x */

        /* One extra for each line */
        *(ptr++) = shiftRest;

      } /* End over y */

    } /* End for all planes */

  } /* End else shiftDelta <= 0 */
}

/******************************************************************************
 * uglVgaBltAlign - Align source bitmap with destination
 *
 * RETURNS: N/A
 ******************************************************************************/

UGL_LOCAL uglVgaBltAlign(UGL_DEVICE_ID devId,
			 UGL_BMAP_HEADER *pSrcBmp,
			 UGL_RECT *pSrcRect,
			 UGL_VGA_DDB *pDestBmp,
			 UGL_RECT *pDestRect)
{
  UGL_INT32 shift, prevShift;
  UGL_VGA_DDB *pDdb;
  UGL_UINT8 **img;

  /* Setup variables */
  pDdb = (UGL_VGA_DDB *) pSrcBmp;

  /* Calculate shift value */
  if (pDestBmp != UGL_NULL) {
    shift = pDestBmp->shiftValue +
	(pDestRect->left & 0x07) - (pSrcRect->left & 0x07);
    pDestRect->left += pDestBmp->shiftValue;
    pDestRect->right += pDestBmp->shiftValue;
  }
  else
    shift = (pDestRect->left & 0x07) - (pSrcRect->left & 0x07);

  /* Check shift value range */
  if (shift < 0)  shift += 8;
  else if (shift > 7) shift -= 8;

  prevShift = pDdb->shiftValue;
  img = pDdb->pPlaneArray;
  pDdb->shiftValue = shift;

  if (prevShift != shift)
    uglVgaShiftBitmap(devId, pSrcBmp, img, prevShift, shift);

  pSrcRect->left += shift;
  pSrcRect->right += shift;
}

/*******************************************************************************
 * uglVgaBltPlane - Blit to one bitplane
 *
 * RETURNS: N/A
 ******************************************************************************/

void uglVgaBltPlane(UGL_DEVICE_ID devId,
			      UGL_UINT8 *pSrc,
			      UGL_RECT *pSrcRect,
			      UGL_SIZE srcStride,
			      UGL_UINT8 *pDest,
			      UGL_RECT *pDestRect,
			      UGL_SIZE destStride,
			      UGL_RASTER_OP rasterOp)
{
  int i, y, width, height;
  UGL_UINT8 *src, *dest;
  UGL_UINT8 startMask, endMask;

  /* Calculate vars */
  width = (pDestRect->right >> 3) - (pDestRect->left >> 3) + 1;
  height = UGL_RECT_HEIGHT(*pDestRect) - 1;
  src = pSrc + pSrcRect->top * srcStride + (pSrcRect->left >> 3);
  dest = pDest + pDestRect->top * destStride + (pDestRect->left >> 3);

  /* Generate masks */
  startMask = 0xff >> (pDestRect->left & 0x07);
  endMask = 0xff << (7 - (pDestRect->right & 0x07));

  /* if pSrc == 0 => Fill entire plane with zeros */
  if (pSrc == (UGL_UINT8 *) 0) {

    /* Select rasterOp */
    switch(rasterOp) {

      /* Case raster operation copy or and */
      case UGL_RASTER_OP_COPY:
      case UGL_RASTER_OP_AND:

      /* Over height */
      for (y = 0; y < height; y++) {

	/* Just one pixel */
        if (width == 1) {

	  /* Fill start and end */
	  dest[0] &= ~(startMask & endMask);

	} /* End if just one pixel */

	/* More pixels */
	else {
	
	  /* Fill start mask */
	  dest[0] &= ~startMask;

	  /* Fill middle */
	  if (width > 2)
	    memset(&dest[1], 0, width - 2);

	  /* Fill end mask */
	  dest[width - 1] &= ~endMask;

	} /* End if more pixels than one */

	/* Advance to next line */
	dest += destStride;

      } /* End over height */

      break; /* End case raster operation copy or and */

    } /* End select raster op */

    /* Done */
    return;

  } /* End if pSrc == 0 => fill with zeros */

  /* if pSrc == -1 => Fill entire plane with ones */
  if (pSrc == (UGL_UINT8 *) -1) {

    /* Select rasterOp */
    switch(rasterOp) {

      /* Case raster operation copy or or */
      case UGL_RASTER_OP_COPY:
      case UGL_RASTER_OP_OR:

      /* Over height */
      for (y = 0; y < height; y++) {

	/* Just one pixel */
        if (width == 1) {

	  /* Fill start and end */
	  dest[0] |= startMask & endMask;

	} /* End if just one pixel */

	/* More pixels */
	else {
	
	  /* Fill start mask */
	  dest[0] |= startMask;

	  /* Fill middle */
	  if (width > 2)
	    memset(&dest[1], 0xff, width - 2);

	  /* Fill end mask */
	  dest[width - 1] |= endMask;

	} /* End if more pixels than one */

	/* Advance to next line */
	dest += destStride;

      } /* End over height */

      break; /* End case raster operation copy or or */

      /* Case raster operation xor */
      case UGL_RASTER_OP_XOR:

      /* Over height */
      for (y = 0; y < height; y++) {

	/* Just one pixel */
        if (width == 1) {

	  /* Fill start and end */
	  dest[0] ^= startMask & endMask;

	} /* End if just one pixel */

	/* More pixels */
	else {
	
	  /* Fill start mask */
	  dest[0] ^= startMask;

	  /* Fill middle */
	  if (width > 2)
	    for (i = 1; i < width - 1; i++)
	      dest[i] ^= 0xff;

	  /* Fill end mask */
	  dest[i] ^= endMask;

	} /* End if more pixels than one */

	/* Advance to next line */
	dest += destStride;

      } /* End over height */

      break; /* End case raster operation xor */

    } /* End select raster op */

    /* Done */
    return;

  } /* End if pSrc == -1 => fill with ones */

  /* If here this is a plane copy */
  /* Select raster op */
  switch(rasterOp) {

    /* Case raster operation copy */
    case UGL_RASTER_OP_COPY:

    /* Over height */
    for (y = 0; y < height; y++) {

      /* Check if just one pixel */
      if (width == 1) {

	/* Blit start and end mask */
        dest[0] |= src[0] & (startMask & endMask);
	dest[0] &= src[0] | ~(startMask & endMask);

      } /* End if just one pixel */

      /* Else more than one pixel */
      else {

	/* Blit start mask */
        dest[0] |= src[0] & startMask;
	dest[0] &= src[0] | ~startMask;

	/* Blit middle */
	if (width > 2)
	  memcpy(&dest[1], &src[1], width - 2);

	/* Blit end mask */
	dest[width - 1] |= src[width - 1] & endMask;
	dest[width - 1] &= src[width - 1] | ~endMask;

      } /* End else more than one pixel */

      /* Advance one line */
      src += srcStride;
      dest += destStride;

    } /* End over height */

    break; /* End case raster operation copy */

    /* Case raster operation and */
    case UGL_RASTER_OP_AND:

    /* Over height */
    for (y = 0; y < height; y++) {

      /* Check if just one pixel */
      if (width == 1) {

	/* Blit start and end mask */
	dest[0] &= src[0] | ~(startMask & endMask);

      } /* End if just one pixel */

      /* Else more than one pixel */
      else {

	/* Blit start mask */
        dest[0] &= src[0] | ~startMask;

	/* Blit middle */
	if (width > 2)
          for (i = 1; i < width - 1; i++)
	    dest[i] &= src[i];

	/* Blit end mask */
	dest[i] &= src[i] | ~endMask;

      } /* End else more than one pixel */

      /* Advance one line */
      src += srcStride;
      dest += destStride;

    } /* End over height */

    break; /* End case raster operation and */

    /* Case raster operation or */
    case UGL_RASTER_OP_OR:

    /* Over height */
    for (y = 0; y < height; y++) {

      /* Check if just one pixel */
      if (width == 1) {

	/* Blit start and end mask */
	dest[0] |= src[0] & (startMask & endMask);

      } /* End if just one pixel */

      /* Else more than one pixel */
      else {

	/* Blit start mask */
        dest[0] |= src[0] & startMask;

	/* Blit middle */
	if (width > 2)
          for (i = 1; i < width - 1; i++)
	    dest[i] |= src[i];

	/* Blit end mask */
	dest[i] |= src[i] & endMask;

      } /* End else more than one pixel */

      /* Advance one line */
      src += srcStride;
      dest += destStride;

    } /* End over height */

    break; /* End case raster operation or */

    /* Case raster operation xor */
    case UGL_RASTER_OP_XOR:

    /* Over height */
    for (y = 0; y < height; y++) {

      /* Check if just one pixel */
      if (width == 1) {

	/* Blit start and end mask */
	dest[0] ^= src[0] & (startMask & endMask);

      } /* End if just one pixel */

      /* Else more than one pixel */
      else {

	/* Blit start mask */
        dest[0] ^= src[0] & startMask;

	/* Blit middle */
	if (width > 2)
          for (i = 1; i < width - 1; i++)
	    dest[i] ^= src[i];

	/* Blit end mask */
	dest[i] ^= src[i] & endMask;

      } /* End else more than one pixel */

      /* Advance one line */
      src += srcStride;
      dest += destStride;

    } /* End over height */

    break; /* End case raster operation xor */

  } /* End select raster op */

}

/*******************************************************************************
 * uglVgaBltColorToColor - Blit from memory to memory bitmap
 *
 * RETURNS: N/A
 ******************************************************************************/

UGL_LOCAL void uglVgaBltColorToColor(UGL_DEVICE_ID devId,
				     UGL_VGA_DDB *pSrcBmp,
				     UGL_RECT *pSrcRect,
				     UGL_VGA_DDB *pDestBmp,
				     UGL_RECT *pDestRect)
{
  UGL_GENERIC_DRIVER *pDrv;
  UGL_RASTER_OP rasterOp;
  int planeIndex, numPlanes;
  UGL_SIZE srcStride, destStride;
  UGL_UINT8 *src, *dest;

  /* Get driver first in device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;

  /* Cache raster op */
  rasterOp = pDrv->gc->rasterOp;

  /* Calculate vars */
  numPlanes = pDestBmp->colorDepth;
  srcStride = (pSrcBmp->header.width + 7) / 8 + 1;
  destStride = (pDestBmp->header.width + 7) / 8 + 1;

  /* Align source bitmap to dest */
  uglVgaBltAlign(devId, (UGL_BMAP_HEADER *) pSrcBmp, pSrcRect,
	         pDestBmp, pDestRect);

  /* Over all blit planes */
  for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

    src = pSrcBmp->pPlaneArray[planeIndex];
    dest = pDestBmp->pPlaneArray[planeIndex];

    uglVgaBltPlane(devId,
		   src, pSrcRect, srcStride,
		   dest, pDestRect, destStride,
		   pDrv->gc->rasterOp);

  } /* End over all blit planes */

}

/*******************************************************************************
 * uglVgaBltColorToFrameBuffer - Blit from memory bitmap to frame buffer
 *
 * RETURNS: N/A
 ******************************************************************************/

UGL_LOCAL void uglVgaBltColorToFrameBuffer(UGL_DEVICE_ID devId,
				           UGL_VGA_DDB *pBmp,
				           UGL_RECT *pSrcRect,
				           UGL_RECT *pDestRect)
{
  UGL_GENERIC_DRIVER *pDrv;
  UGL_VGA_DRIVER *pVgaDrv;
  UGL_RASTER_OP rasterOp;
  volatile register UGL_UINT8 tmp;
  UGL_INT32 i, y, height, planeIndex, numPlanes, width;
  UGL_INT32 destBytesPerLine, srcBytesPerLine, srcOffset;
  UGL_UINT8 *destStart, startMask, endMask, regValue, byteValue;
  UGL_UINT8 *src, *dest;

  /* Get driver first in device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;
  pVgaDrv = (UGL_VGA_DRIVER *) devId;

  /* Cache raster op */
  rasterOp = pDrv->gc->rasterOp;

  /* Align source bitmap to dest */
  uglVgaBltAlign(devId, (UGL_BMAP_HEADER *) pBmp, pSrcRect,
	         UGL_NULL, pDestRect);

  /* Setup variables for blit */
  width = (pDestRect->right >> 3) - (pDestRect->left >> 3) + 1;
  height = UGL_RECT_HEIGHT(*pDestRect) - 1;
  numPlanes = devId->pMode->Depth;
  destBytesPerLine = pVgaDrv->bytesPerLine;
  srcBytesPerLine = (pBmp->header.width + 7) / 8 + 1;
  destStart = (UGL_UINT8 *) pDrv->fbAddress +
	pDestRect->top * destBytesPerLine + (pDestRect->left >> 3);
  srcOffset = pSrcRect->top * srcBytesPerLine +
	(pSrcRect->left >> 3);

  /* Generate masks */
  startMask = 0xff >> (pDestRect->left & 0x07);
  endMask = 0xff << (7 - (pDestRect->right & 0x07));
  if (width == 1)
    startMask &= endMask;

  /* Setup registers */
  /* Write mode 0 */
  UGL_OUT_BYTE(0x3ce, 0x05);
  regValue = UGL_IN_BYTE(0x3cf);
  regValue &= 0xf8;
  UGL_OUT_BYTE(0x3cf, regValue);

  /* Disable set/reset registers */
  UGL_OUT_WORD(0x3ce, 0x0001);

  /* Select bit mask register */
  UGL_OUT_BYTE(0x3ce, 0x08);

  /* Select write plane */
  UGL_OUT_BYTE(0x3c4, 0x02);

  /* Blit */
  /* Over height */
  for (y = 0; y < height; y++) {

    /* For all planes */
    for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

      /* Calulcate plane vars */
      src = pBmp->pPlaneArray[planeIndex] + srcOffset;
      dest = destStart;
      i = 0;

      /* Select plane */
      regValue = 0x01 << planeIndex;
      UGL_OUT_BYTE(0x3c5, regValue);

      /* Write start mask */
      UGL_OUT_BYTE(0x3cf, startMask);
      tmp = dest[i];
      dest[i] = src[i];
      i++;

      /* Check if anything to more blit */
      if (width > 1) {

        /* Check if bigger blit */
        if (width > 2) {

	  /* Blit */
	  /* Set register to all bits to visible */
          UGL_OUT_BYTE(0x3cf, 0xff);

	  /* Check if just a straight copy */
	  if (rasterOp == UGL_RASTER_OP_COPY) {

            memcpy(&dest[i], &src[i], width - 2);
	    i += width - 2;

	  } /* End if just a straight copy */

	  /* Else other raster operation, must to bytevise copy */
	  else {

	    for (i = 1; i < width - 1; i++) {
	      tmp = dest[i];
	      dest[i] = src[i];
	    }

	  } /* End else other raster operation */

        } /* End if bigger blit */

        /* Write end mask */
        UGL_OUT_BYTE(0x3cf, endMask);
        tmp = dest[i];
	dest[i] = src[i];

      } /* End if anything more to blit */

    } /* End for all planes */

    /* Advance line */
    srcOffset += srcBytesPerLine;
    destStart += destBytesPerLine;

  } /* End for height */

  /* Restore registers */
  /* Restore mask register */
  UGL_OUT_BYTE(0x3c5, 0x0f);

  /* Restore bitmask register */
  UGL_OUT_BYTE(0x3cf, 0xff);

  /* Set write mode 3 */
  UGL_OUT_BYTE(0x3ce, 0x05);
  byteValue = UGL_IN_BYTE(0x3cf);
  UGL_OUT_BYTE(0x3cf, byteValue | 0x03);

  /* Enable set reset registers */
  UGL_OUT_WORD(0x3ce, 0x0f01);
}

/*******************************************************************************
 * uglVgaBltFrameBuffetToColor - Blit from frame buffer to memry bitmap
 *
 * RETURNS: N/A
 ******************************************************************************/

UGL_LOCAL void uglVgaBltFrameBufferToColor(UGL_DEVICE_ID devId,
				           UGL_RECT *pSrcRect,
				           UGL_VGA_DDB *pBmp,
				           UGL_RECT *pDestRect)
{
  UGL_GENERIC_DRIVER *pDrv;
  UGL_VGA_DRIVER *pVgaDrv;
  UGL_RASTER_OP rasterOp;
  UGL_INT32 i, y, height, planeIndex, numPlanes, width;
  UGL_INT32 destBytesPerLine, srcBytesPerLine, destOffset;
  UGL_UINT8 startMask, endMask, byteValue;
  UGL_UINT8 *src, *srcStart, *dest;

  /* Get driver first in device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;
  pVgaDrv = (UGL_VGA_DRIVER *) devId;

  /* Cache raster op */
  rasterOp = pDrv->gc->rasterOp;

  /* Align source bitmap to dest */
  uglVgaBltAlign(devId, (UGL_BMAP_HEADER *) pBmp, pDestRect,
	         UGL_NULL, pSrcRect);

  /* Setup variables for blit */
  width = (pSrcRect->right >> 3) - (pSrcRect->left >> 3) + 1;
  height = UGL_RECT_HEIGHT(*pDestRect) - 1;
  numPlanes = pBmp->colorDepth;
  srcBytesPerLine = pVgaDrv->bytesPerLine;
  destBytesPerLine = (pBmp->header.width + 7) / 8 + 1;
  srcStart = (UGL_UINT8 *) pDrv->fbAddress +
	pSrcRect->top * srcBytesPerLine + (pSrcRect->left >> 3);
  destOffset = pDestRect->top * destBytesPerLine +
	(pDestRect->left >> 3);

  /* Generate masks */
  startMask = 0xff >> (pDestRect->left & 0x07);
  endMask = 0xff << (7 - (pDestRect->right & 0x07));
  if (width == 1)
    startMask &= endMask;

  /* Select read plane */
  UGL_OUT_BYTE(0x3ce, 0x04);

  /* Blit */
  /* For all planes */
  for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

    /* Calulcate plane vars */
    src = srcStart;
    dest = pBmp->pPlaneArray[planeIndex] + destOffset;

    /* Select plane */
    UGL_OUT_BYTE(0x3cf, planeIndex);

    /* Select raster op */
    switch(rasterOp) {

      /* Case raster operation copy */
      case UGL_RASTER_OP_COPY:

      /* Over height */
      for (y = 0; y < height; y++) {
        i = 0;

        /* Blit start */
        byteValue = src[i];
        dest[i] |= byteValue & startMask;
        dest[i] &= byteValue | ~startMask;
        i++;

        /* Check if anything to more blit */
        if (width > 1) {

          /* Check if bigger blit */
          if (width > 2) {

	    /* Blit */
            memcpy(&dest[i], &src[i], width - 2);
	    i += width - 2;

          } /* End if bigger blit */

          /* Blit end */
          byteValue = src[i];
          dest[i] |= byteValue & endMask;
          dest[i] &= byteValue | ~endMask;

        } /* End if anything to more blit */

        /* Advance to next line */
        src += srcBytesPerLine;
        dest += destBytesPerLine;

      } /* End over height */

      break; /* End case raster operation copy */

      /* Case raster operation and */
      case UGL_RASTER_OP_AND:

      /* Over height */
      for (y = 0; y < height; y++) {
        i = 0;

        /* Blit start */
	dest[i] &= src[i] | ~startMask;
        i++;

        /* Check if anything to more blit */
        if (width > 1) {

          /* Check if bigger blit */
          if (width > 2) {

	    /* Blit */
	    for (i = 1; i < width - 1; i++)
	      dest[i] &= src[i];

          } /* End if bigger blit */

          /* Blit end */
	  dest[i] &= src[i] | ~endMask;

        } /* End if anything to more blit */

        /* Advance to next line */
        src += srcBytesPerLine;
        dest += destBytesPerLine;

      } /* End over height */

      break; /* End case raster operation and */

      /* Case raster operation or */
      case UGL_RASTER_OP_OR:

      /* Over height */
      for (y = 0; y < height; y++) {
        i = 0;

        /* Blit start */
	dest[i] |= src[i] & startMask;
        i++;

        /* Check if anything to more blit */
        if (width > 1) {

          /* Check if bigger blit */
          if (width > 2) {

	    /* Blit */
	    for (i = 1; i < width - 1; i++)
	      dest[i] |= src[i];

          } /* End if bigger blit */

          /* Blit end */
	  dest[i] |= src[i] & endMask;

        } /* End if anything to more blit */

        /* Advance to next line */
        src += srcBytesPerLine;
        dest += destBytesPerLine;

      } /* End over height */

      break; /* End case raster operation or */

      /* Case raster operation xor */
      case UGL_RASTER_OP_XOR:

      /* Over height */
      for (y = 0; y < height; y++) {
        i = 0;

        /* Blit start */
	dest[i] ^= src[i] & startMask;
        i++;

        /* Check if anything to more blit */
        if (width > 1) {

          /* Check if bigger blit */
          if (width > 2) {

	    /* Blit */
	    for (i = 1; i < width - 1; i++)
	      dest[i] ^= src[i];

          } /* End if bigger blit */

          /* Blit end */
	  dest[i] ^= src[i] & endMask;

        } /* End if anything to more blit */

        /* Advance to next line */
        src += srcBytesPerLine;
        dest += destBytesPerLine;

      } /* End over height */

      break; /* End case raster operation xor */

      /* Default case */
      default: break;

    } /* End select raster op */

  } /* End for all planes */

}

/*******************************************************************************
 * uglVgaBitmapBlt - Blit from one bitmap memory area to another
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglVgaBitmapBlt(UGL_DEVICE_ID devId,
			   UGL_DDB_ID srcBmpId,
			   UGL_RECT *pSrcRect,
			   UGL_DDB_ID destBmpId,
			   UGL_POINT *pDestPoint)
{
  UGL_GENERIC_DRIVER *pDrv;
  UGL_GC_ID gc;
  UGL_VGA_DDB *pSrcBmp, *pDestBmp;
  UGL_RECT srcRect, destRect, clipRect;
  UGL_POINT destPoint;

  /* Get driver first in device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;

  /* Get gc */
  gc = pDrv->gc;

  /* Store source and dest */
  pSrcBmp = (UGL_VGA_DDB *) srcBmpId;
  pDestBmp = (UGL_VGA_DDB *) destBmpId;

  /* Store starting point */
  destPoint.x = pDestPoint->x;
  destPoint.y = pDestPoint->y;

  /* Store source rectangle */
  srcRect.top = pSrcRect->top;
  srcRect.left = pSrcRect->left;
  srcRect.right = pSrcRect->right;
  srcRect.bottom = pSrcRect->bottom;

  /* Store destination rectangle */
  destRect.top = pDestPoint->y;
  destRect.left = pDestPoint->x;
  destRect.right = pDestPoint->x;
  destRect.bottom = pDestPoint->y;

  /* Store clip rectangle */
  if (destBmpId == UGL_DEFAULT_ID) {
    clipRect.top = 0;
    clipRect.bottom = devId->pMode->Height;
    clipRect.left = 0;
    clipRect.right = devId->pMode->Width;
  }

  /* Clip */
  if (uglGenericClipDdb(devId, &clipRect,
	(UGL_BMAP_ID *) &pSrcBmp, &srcRect,
	(UGL_BMAP_ID *) &pDestBmp, &destPoint) != UGL_TRUE)
    return UGL_STATUS_ERROR;

  /* Calculate destination */
  UGL_RECT_MOVE_TO_POINT(destRect, destPoint);
  UGL_RECT_SIZE_TO(destRect, UGL_RECT_WIDTH(srcRect),
		UGL_RECT_HEIGHT(srcRect));

  /* Blit */
  if (srcBmpId != UGL_DISPLAY_ID && destBmpId == UGL_DISPLAY_ID)
    uglVgaBltColorToFrameBuffer(devId, pSrcBmp, &srcRect, &destRect);
  else if (srcBmpId == UGL_DISPLAY_ID && destBmpId != UGL_DISPLAY_ID)
    uglVgaBltFrameBufferToColor(devId, &srcRect, pDestBmp, &destRect);
  else
    uglVgaBltColorToColor(devId, pSrcBmp, &srcRect, pDestBmp, &destRect);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglVgaBitmapWrite - Write a device independet bitmap to vga bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglVgaBitmapWrite(UGL_DEVICE_ID devId,
			     UGL_DIB *pDib,
			     UGL_RECT *pSrcRect,
			     UGL_DDB_ID ddbId,
			     UGL_POINT *pDestPoint)
{
  UGL_GENERIC_DRIVER *pDrv;
  UGL_VGA_DDB *pVgaBmp;
  UGL_RECT srcRect, destRect;
  UGL_POINT destPoint;
  UGL_INT32 x, y, left, numPlanes, planeIndex,
	destBytesPerLine, startIndex, srcOffset;
  UGL_UINT8 startMask;
  UGL_UINT8 **pPlaneArray;
  UGL_UINT8 *pSrc, *pClut;
  UGL_INT32 bpp, ppb, nPixels, shift, srcMask, destIndex, destMask;
  UGL_COLOR pixel, planeMask;

  /* Get driver first in device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;

  /* Get device dependent bitmap */
  pVgaBmp = (UGL_VGA_DDB *) ddbId;

  /* Get dimensions */
  srcRect.top = pSrcRect->top;
  srcRect.bottom = pSrcRect->bottom;
  srcRect.left = pSrcRect->left;
  srcRect.right = pSrcRect->right;
  destPoint.x = pDestPoint->x;
  destPoint.y = pDestPoint->y;

  /* Calulcate destination dimensions */
  destRect.left = destPoint.x;
  destRect.top = destPoint.y;
  destRect.right = destPoint.x;
  destRect.bottom = destPoint.y;
  UGL_RECT_SIZE_TO(destRect, UGL_RECT_WIDTH(srcRect),
			     UGL_RECT_HEIGHT(srcRect));

  /* Precaculate variables */
  numPlanes = devId->pMode->Depth;
  srcOffset = (srcRect.top * pDib->stride) + srcRect.left;

  /* Handle case if the display is not the destination */
  if (ddbId != UGL_DISPLAY_ID) {

    /* Precaculate variables */
    left = destRect.left + pVgaBmp->shiftValue;
    destBytesPerLine = (pVgaBmp->header.width + 7) / 8 + 1;
    pPlaneArray = pVgaBmp->pPlaneArray;
    startIndex = destRect.top * destBytesPerLine + (left >> 3);
    startMask = 0x80 >> (left & 0x07);

    /* Handle case when source has a color lookup table */
    if (pDib->imageFormat != UGL_DIRECT) {

      /* Check if temporary clut should be generated */
      if (pDib->colorFormat != UGL_DEVICE_COLOR_32) {

        /* Generate */
	pClut = malloc(pDib->clutSize * sizeof(UGL_COLOR));
	if (pClut == UGL_NULL)
	  return UGL_STATUS_ERROR;

	/* Convert to 32-bit color */
	(*devId->colorConvert)(devId, pDib->pClut, pDib->colorFormat,
			       pClut, UGL_DEVICE_COLOR_32, pDib->clutSize);

      } /* End if temporary clut should be generated */

      /* Else keep current clut */
      else {

        pClut = pDib->pClut;

      } /* End else keep current clut */

      /* Select color mode */
      switch(pDib->imageFormat) {

	/* Byte aligned image */
	case UGL_INDEXED_8:

	  /* Calculate source pointer */
	  pSrc = (UGL_UINT8 *) pDib->pData + srcRect.top * pDib->stride;

	  /* For source height */
	  for (y = srcRect.top; y <= srcRect.bottom; y++) {

	    /* Variable recalculate for each line */
	    destIndex = startIndex;
	    destMask = startMask;

	    /* For source width */
	    for (x = srcRect.left; x <= srcRect.right; x++) {

	      /* Initialize pixel */
	      //pixel = ((UGL_COLOR *) pClut) [pSrc[x]];
	      pixel = pSrc[x];
	      planeMask = 0x01;

	      /* For each plane */
	      for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

	        if ((pixel & planeMask) != 0)
		  pPlaneArray[planeIndex][destIndex] |= destMask;
		else
		  pPlaneArray[planeIndex][destIndex] &= ~destMask;

	        /* Advance plane mask */
	        planeMask <<= 1;

	      } /* End for each plane */

	      /* Advance to next pixel */
	      destMask >>= 1;

	      /* Check if a new byte was reached */
	      if (destMask == 0) {
	        destIndex++;
		destMask = 0x80;
	      }

	    } /* End for source width */

	    /* Advance to next line */
	    pSrc += pDib->stride;
	    startIndex += destBytesPerLine;

	  } /* End for source height */

	break;

	/* Less than one byte used for each pixel */
        case UGL_INDEXED_4:
	case UGL_INDEXED_2:
	case UGL_INDEXED_1:

          /* Precalulate pixel vars */
	  bpp = pDib->imageFormat & UGL_INDEX_MASK;
	  ppb = bpp / 8;

	  /* For source height */
	  for (y = srcRect.top; y <= srcRect.bottom; y++) {

	    /* Variable recalculate for each line */
	    pSrc = (UGL_UINT8 *) pDib->pData + srcOffset / ppb;
	    nPixels = srcOffset & ppb;
	    shift = (ppb - nPixels - 1) * bpp;
	    srcMask = (0xff >> (8 - bpp)) << shift;
	    destIndex = startIndex;
	    destMask = startMask;

	    /* For source width */
	    for (x = srcRect.left; x <= srcRect.right; x++) {

	      /* Initialize pixel */
	      pixel = (*pSrc & srcMask) >> shift;
	      planeMask = 0x01;

	      /* For each plane */
	      for (planeIndex = 0; planeIndex < numPlanes; planeIndex++) {

	        if ((pixel & planeMask) != 0)
		  pPlaneArray[planeIndex][destIndex] |= destMask;
		else
		  pPlaneArray[planeIndex][destIndex] &= ~destMask;

	        /* Advance plane mask */
	        planeMask <<= 1;

	      } /* End for each plane */

	      /* Advance to next pixel */
	      srcMask >>= bpp;
	      destMask >>= 1;
	      shift -= bpp;

	      /* Check if a new byte was reached */
	      /* For source */
	      if (srcMask == 0) {
	        pSrc++;
		shift = (bpp - 1) * bpp;
		srcMask = (0xff >> (8 - bpp)) << shift;
	      }

	      /* Check if a new byte was reached */
	      /* For destination */
	      if (destMask == 0) {
	        destIndex++;
		destMask = 0x80;
	      }

	    } /* End for source width */

	    /* Advance to next line */
	    srcOffset += pDib->stride;
	    startIndex += destBytesPerLine;

	  } /* End for source height */

	break;

	default:
	break;

      } /* End select color mode */

    } /* End not direct color */

  } /* End not display */

  return UGL_STATUS_OK;
}

