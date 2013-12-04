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

/* uglmode.c - Universal graphics library mode support library */

#include <ugl/ugl.h>

/*******************************************************************************
 * uglModeAvailGet - Get avilable graphics modes
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglModeAvailGet(UGL_DEVICE_ID devId,
			   UGL_UINT32 *pNumModes,
			   const UGL_MODE **pModeArray)
{
  if (devId == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  /* Call specific method */
  if ((*devId->modeAvailGet)(devId, pNumModes, pModeArray) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

  return UGL_STATUS_OK;
}

/*******************************************************************************
 * uglModeSet - Set graphics modes
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglModeSet(UGL_DEVICE_ID devId, UGL_MODE *pMode)
{
  if (devId == UGL_NULL || pMode == UGL_NULL)
    return UGL_STATUS_ERROR;

  /* Lock device */
  if (uglOsLock(devId->lockId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  /* Call specific method */
  if ( (*devId->modeSet)(devId, pMode) != UGL_STATUS_OK) {
    uglOsUnLock(devId->lockId);
    return UGL_STATUS_ERROR;
  }

  /* Create default graphics context */
  if (devId->defaultGc == UGL_NULL) {

    devId->defaultGc = uglGcCreate(devId);
    if (devId->defaultGc == UGL_NULL) {
      uglOsUnLock(devId->lockId);
      uglGraphicsDevDestroy(devId);
      return UGL_STATUS_ERROR;
    }

    if (uglGcSet(devId, devId->defaultGc) != UGL_STATUS_OK) {
      uglOsUnLock(devId->lockId);
      uglGcDestroy(devId->defaultGc);
      uglGraphicsDevDestroy(devId);
      return UGL_STATUS_ERROR;
    }
  }

  /* Unlock */
  uglOsUnLock(devId->lockId);

  return UGL_STATUS_OK;
}

