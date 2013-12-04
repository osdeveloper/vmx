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

/* uglcolor.c - Universal graphics library color support */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/*******************************************************************************
 * uglClutSet
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglClutSet(UGL_DEVICE_ID devId,
		      UGL_ORD offset,
		      UGL_ARGB *pColors,
		      UGL_SIZE numColors)
{
  if (devId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  if ((*devId->clutSet)(devId, offset, pColors, numColors) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglClutGet
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglClutGet(UGL_DEVICE_ID devId,
		      UGL_ORD offset,
		      UGL_ARGB *pColors,
		      UGL_SIZE numColors)
{
  if (devId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  if ((*devId->clutGet)(devId, offset, pColors, numColors) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglColorAllocExt - Allocate color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglColorAllocExt(UGL_DEVICE_ID devId,
			    UGL_ARGB *pReqColors,
			    UGL_ORD *pIndex,
			    UGL_ARGB  *pActualColors,
			    UGL_COLOR *pUglColors,
			    UGL_SIZE numColors)
{
  if (devId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  if ((*devId->colorAlloc)(devId, pReqColors, pIndex, pActualColors,
	pUglColors, numColors) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglColorAlloc - Allocate color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglColorAlloc(UGL_DEVICE_ID devId,
			 UGL_ARGB *pReqColors,
			 UGL_ORD *pIndex,
			 UGL_COLOR *pUglColors,
			 UGL_SIZE numColors)
{
  if (devId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  if ((*devId->colorAlloc)(devId, pReqColors, pIndex, UGL_NULL,
	pUglColors, numColors) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglColorFree - Free color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglColorFree(UGL_DEVICE_ID devId,
			UGL_COLOR *pColors,
			UGL_SIZE numColors)
{
  if (devId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  if ((*devId->colorFree)(devId, pColors, numColors) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

}

