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

UGL_LOCAL UGL_SEARCH_ID uglBMFFontFindFirst (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor
    );

UGL_LOCAL UGL_STATUS uglBMFFontFindNext (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor,
    UGL_SEARCH_ID       searchId
    );

UGL_LOCAL UGL_STATUS uglBMFFontFindClose (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_SEARCH_ID       searchId
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
    UGL_SIZE              numFonts;
    UGL_LOCK_ID           lockId;

    if (uglBMFFontData == NULL) {
        return (UGL_NULL);
    }

    /* Check number of fonts */
    for (numFonts = 0; uglBMFFontData[numFonts] != UGL_NULL; numFonts++);
    if (numFonts == 0) {
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
    pDrv->fontFindFirst     = uglBMFFontFindFirst;
    pDrv->fontFindNext      = uglBMFFontFindNext;
    pDrv->fontFindClose     = uglBMFFontFindClose;

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

/******************************************************************************
 *
 * uglBMFFontFindFirst - Find first font in font driver
 *
 * RETURNS: UGL_SEARCH_ID or UGL_NULL
 */

UGL_LOCAL UGL_SEARCH_ID uglBMFFontFindFirst (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor
    ) {
    UGL_SEARCH_ID  searchId = UGL_NULL;

    if (uglBMFFontData != UGL_NULL) {
        searchId = (UGL_SEARCH_ID) UGL_MALLOC (sizeof (UGL_SEARCH_ID));
        if (searchId != UGL_NULL) {
            *pFontDescriptor = *(UGL_FONT_DESC *) uglBMFFontData[0];
            *(UGL_UINT32 *) searchId = 1;
        }
    }

    return (searchId);
}

/******************************************************************************
 *
 * uglBMFFontFindNext - Find next font in font driver
 *
 * RETURNS: UGL_STATUS_OK, UGL_STATUS_FINISHED or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontFindNext (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_FONT_DESC *     pFontDescriptor,
    UGL_SEARCH_ID       searchId
    ) {
    UGL_UINT32  index;
    UGL_STATUS  status = UGL_STATUS_ERROR;

    if (searchId != UGL_NULL) {
        index = *(UGL_UINT32 *) searchId;
        if (uglBMFFontData[index] != UGL_NULL) {
            *pFontDescriptor = *(UGL_FONT_DESC *) uglBMFFontData[index++];
            *(UGL_UINT32 *) searchId = index;
            status = UGL_STATUS_OK;
        }
        else {
            status = UGL_STATUS_FINISHED;
        }
    }

    return (status);
}

/******************************************************************************
 *
 * uglBMFFontFindClose - Terminate font search for font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglBMFFontFindClose (
    UGL_FONT_DRIVER_ID  drvId,
    UGL_SEARCH_ID       searchId
    ) {
    UGL_STATUS  status = UGL_STATUS_ERROR;

    if (searchId != UGL_NULL) {
        UGL_FREE (searchId);
        status = UGL_STATUS_OK;
    }

    return (status);
}

