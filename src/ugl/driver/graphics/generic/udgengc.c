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

/* udgengc.c - Universal graphics library graphics context functions */

#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/*******************************************************************************
 * uglGenericGcCreate - Create graphics context
 *
 * RETURNS: Graphics context id or UGL_NULL
 ******************************************************************************/

UGL_GC_ID uglGenericGcCreate(UGL_DEVICE_ID devId)
{
  UGL_GC_ID gcId;

  if (devId == UGL_NULL)
    return UGL_NULL;

  gcId = (UGL_GC_ID) malloc(sizeof(UGL_GC));
  if (gcId == NULL)
    return UGL_NULL;

  return gcId;
}

/*******************************************************************************
 * uglGenericGcCopy - Copy graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglGenericGcCopy(UGL_DEVICE_ID devId,
			    UGL_GC_ID srcGcId,
			    UGL_GC_ID destGcId)
{
  if (devId == UGL_NULL || srcGcId == NULL || destGcId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Copy */
  memcpy((char *) destGcId, (char *) srcGcId, sizeof(UGL_GC));

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglGenericGcDestroy - Free graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglGenericGcDestroy(UGL_DEVICE_ID devId, UGL_GC_ID gc)
{
  UGL_GENERIC_DRIVER *pDrv;

  if (devId == NULL)
    return UGL_STATUS_ERROR;

  /* Get driver since its the first entry in the device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;
  if (pDrv == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Unset current gc if it is the same as the one beeing destroyed */
  if (pDrv->gc == gc)
    pDrv->gc = UGL_NULL;

  free(gc);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglGenericGcSet - Set current graphics context
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglGenericGcSet(UGL_DEVICE_ID devId, UGL_GC_ID gc)
{
  UGL_GENERIC_DRIVER *pDrv;

  /* Get driver since it is fist in the device struct */
  pDrv = (UGL_GENERIC_DRIVER *) devId;

  /* Update gc */
  pDrv->gc = gc;

  return UGL_STATUS_OK;
}

