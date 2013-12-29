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

/* udbmffnt.c - Bitmap font driver for Universal Graphics Library */

#include <ugl/ugl.h>
#include <ugl/driver/font/udbmffnt.h>

/* Defines */

#define UGL_BMF_FONT_ENGINE_VERSION     1
#define UGL_BMF_FONT_DRIVER_VERSION     1

/* Imports */

extern const UGL_BMF_FONT_DESC * uglBMFFontData[];

/* Locals */

UGL_LOCAL UGL_STATUS uglBMFFontDriverInfo (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_INFO_REQ        infoRequest,
    void *              pInfo
    );

UGL_LOCAL UGL_STATUS uglBMFFontDriverDestroy (
    UGL_FONT_DRIVER_ID  drvId
    );

/******************************************************************************
 *
 * uglBMFFontDriverCreate - Create bitmap font driver
 *
 * RETURNS: UGL_FONT_DRIVER_ID or UGL_NULL
 */

UGL_FONT_DRIVER_ID uglBMFFontDriverCreate (
    UGL_DEVICE_ID  devId
    ) {
    UGL_BMF_FONT_DRIVER * pBmfDrv;
    UGL_FONT_DRIVER *     pDrv;
    UGL_LOCK_ID           lockId;

    if (uglBMFFontData == NULL) {
        return (UGL_NULL);
    }

    /* Allocate memory for driver */
    pBmfDrv =
        (UGL_BMF_FONT_DRIVER *) UGL_CALLOC (1, sizeof (UGL_BMF_FONT_DRIVER));
    if (pBmfDrv == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Create lock */
    lockId = uglOSLockCreate ();
    if (lockId == UGL_NULL) {
        UGL_FREE (pBmfDrv);
        return (UGL_NULL);
    }

    /* Setup struct */
    pDrv = (UGL_FONT_DRIVER *) pBmfDrv;
    pDrv->pDriver = devId;
    pDrv->fontDriverInfo    = uglBMFFontDriverInfo;
    pDrv->fontDriverDestroy = uglBMFFontDriverDestroy;

    /* Setup driver specific part of struct */
    pBmfDrv->textOrigin = UGL_FONT_TEXT_BASELINE;
    pBmfDrv->lockId     = lockId;

    return (UGL_FONT_DRIVER_ID) pBmfDrv;
}

/******************************************************************************
 *
 * uglBMFFontDriverInfo - Get information about bitmap font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontDriverInfo (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_INFO_REQ        infoRequest,
    void *              pInfo
    ) {
    UGL_BMF_FONT_DRIVER * pDrv   = (UGL_BMF_FONT_DRIVER *) drvId;
    UGL_STATUS            status = UGL_STATUS_ERROR;

    switch (infoRequest) {
        case UGL_FONT_ENGINE_VERSION_GET:
            if (pInfo != UGL_NULL) {
                *(UGL_INT32 *) pInfo = UGL_BMF_FONT_ENGINE_VERSION;
                status = UGL_STATUS_OK;
            }
            break;

        case UGL_FONT_DRIVER_VERSION_GET:
            if (pInfo != UGL_NULL) {
                *(UGL_INT32 *) pInfo = UGL_BMF_FONT_DRIVER_VERSION;
                status = UGL_STATUS_OK;
            }
            break;

        case UGL_FONT_TEXT_ORIGIN:
            if (pInfo != UGL_NULL) {
                if (*(UGL_ORD *) pInfo == UGL_FONT_TEXT_UPPER_LEFT ||
                    *(UGL_ORD *) pInfo == UGL_FONT_TEXT_BASELINE) {
                    if (uglOSLock(pDrv->lockId) == UGL_STATUS_ERROR) {
                        return (status);
                    }
                    pDrv->textOrigin = *(UGL_ORD *) pInfo;
                    uglOSUnlock (pDrv->lockId);
                    status = UGL_STATUS_OK;
                }
            }
            break;

        default:
            if (pInfo != UGL_NULL) {
                *(UGL_BOOL *) pInfo = UGL_FALSE;
            }
            break;
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontDriverDestroy - Destroy bitmap font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontDriverDestroy (
    UGL_FONT_DRIVER_ID  drvId
    ) {
    UGL_BMF_FONT_DRIVER * pDrv   = (UGL_BMF_FONT_DRIVER *) drvId;
    UGL_STATUS            status = UGL_STATUS_OK;

    uglOSLockDestroy (pDrv->lockId);

    return (status);
}

