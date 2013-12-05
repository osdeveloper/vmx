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

/* uglugi.c - Universal graphics library common device */

#include <ugl/ugl.h>

/******************************************************************************
 *
 * uglUgiDevInit - Initialize graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglUgiDevInit (
    UGL_DEVICE_ID  devId
    ) {

    devId->lockId = uglOsLockCreate ();
    if (devId->lockId == UGL_NULL) {
        return UGL_STATUS_ERROR;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglUgiDevDeinit - Deinitialize graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglUgiDevDeinit (
    UGL_DEVICE_ID  devId
    ) {

    if (uglOsLockDestroy (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGraphicsDevDestroy - Free graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglGraphicsDevDestroy (
    UGL_DEVICE_ID  devId
    ) {

    if (devId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Destroy default graphics context */
    if (devId->defaultGc != UGL_NULL) {
        uglGcDestroy (devId->defaultGc);
    }

    /* Call driver specific device destroy method */
    return (*devId->destroy) (devId);
}

