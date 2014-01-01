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

/* uglcursr.c - Universal graphics library cursor support */

#include <ugl/ugl.h>

/* Locals */

UGL_LOCAL UGL_BOOL uglCursorInitialized = UGL_FALSE;

/******************************************************************************
 *
 * uglCursorInit - Initialize cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorInit (
    UGL_DEVICE_ID  devId,
    UGL_SIZE       maxWidth,
    UGL_SIZE       maxHeight,
    UGL_POS        xPosition,
    UGL_POS        yPosition
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (devId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Invoke driver specific method */
    status = (*devId->cursorInit) (devId, maxWidth, maxHeight,
                                   xPosition, yPosition);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    /* Mark as initialized */
    uglCursorInitialized = UGL_TRUE;

    return (status);
}

/******************************************************************************
 *
 * uglCursorDeinit - Deinitialize cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorDeinit (
    UGL_DEVICE_ID  devId
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (devId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    if (uglCursorInitialized != UGL_TRUE) {
        return (UGL_STATUS_OK);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Invoke driver specific method */
    status = (*devId->cursorDeinit) (devId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    /* Mark as deinitialized */
    uglCursorInitialized = UGL_FALSE;

    return (status);
}

/******************************************************************************
 *
 * uglCursorBitmapCreate - Create cursor bitmap
 *
 * RETURNS: UGL_CDDB_ID or UGL_NULL
 */

UGL_CDDB_ID uglCursorBitmapCreate (
    UGL_DEVICE_ID  devId,
    UGL_CDIB *     pCdib
    ) {
    UGL_CDDB_ID  bmpId;

    /* Check params */
    if (devId == UGL_NULL || pCdib == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_NULL);
    }

    /* Invoke driver specific method */
    bmpId = (*devId->cursorBitmapCreate) (devId, pCdib);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (bmpId);
}

/******************************************************************************
 *
 * uglCursorBitmapDestroy - Destroy cursor bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorBitmapDestroy (
    UGL_DEVICE_ID  devId,
    UGL_CDDB_ID    cddbId
    ) {
    UGL_STATUS   status;
    UGL_CDDB_ID  cursorImageId;

    /* Check params */
    if (devId == UGL_NULL || cddbId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Check if image used */
    uglCursorImageGet (devId, &cursorImageId);
    if (cursorImageId == cddbId) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Invoke driver specific method */
    status = (*devId->cursorBitmapDestroy) (devId, cddbId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

/******************************************************************************
 *
 * uglCursorImageGet - Get cursor image bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorImageGet (
    UGL_DEVICE_ID  devId,
    UGL_CDDB_ID *  pImageBitmapId
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (devId == UGL_NULL || uglCursorInitialized == UGL_FALSE) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Invoke driver specific method */
    status = (*devId->cursorImageGet) (devId, pImageBitmapId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

/******************************************************************************
 *
 * uglCursorImageSet - Set cursor image bitmap
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorImageSet (
    UGL_DEVICE_ID  devId,
    UGL_CDDB_ID    imageBitmapId
    ) {
    UGL_STATUS   status;
    UGL_CDDB_ID  currImageBitmapId;

    status = uglCursorImageGet (devId, &currImageBitmapId);
    if (status == UGL_STATUS_OK && imageBitmapId != currImageBitmapId) {

        /* Lock device */
        if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }

        /* Invoke driver specific method */
        status = (*devId->cursorImageSet) (devId, imageBitmapId);

        /* Unlock */
        uglOSUnlock (devId->lockId);
    }

    return (status);
}

/******************************************************************************
 *
 * uglCursorOn - Turn on cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorOn (
    UGL_DEVICE_ID  devId
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (devId == UGL_NULL || uglCursorInitialized == UGL_FALSE) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Invoke driver specific method */
    status = (*devId->cursorOn) (devId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

/******************************************************************************
 *
 * uglCursorOff - Turn off cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCursorOff (
    UGL_DEVICE_ID  devId
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (devId == UGL_NULL || uglCursorInitialized == UGL_FALSE) {
        return (UGL_STATUS_ERROR);
    }

    /* Lock device */
    if (uglOSLock (devId->lockId) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    /* Invoke driver specific method */
    status = (*devId->cursorOff) (devId);

    /* Unlock */
    uglOSUnlock (devId->lockId);

    return (status);
}

