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

/* udgclri.c - Indexed color palette support */

#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcclr.h>
#include <ugl/driver/graphics/generic/udgen.h>

/******************************************************************************
 *
 * uglGenericClutCreate - Create palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutCreate (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_SIZE             numColors
    ) {

    pDrv->pClut = uglCommonClutCreate (numColors);
    if (pDrv->pClut == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericClutSet - Set generic indexed palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutSet (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_ORD              offset,
    UGL_ARGB *           pColors,
    UGL_SIZE             numColors
    ) {

    return uglCommonClutSet (pDrv->pClut, offset, pColors, numColors);
}

/******************************************************************************
 *
 * uglGenericClutGet - Get palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutGet (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_ORD              offset,
    UGL_ARGB *           pColors,
    UGL_SIZE             numColors
    ) {

    return uglCommonClutGet (pDrv->pClut, offset, pColors, numColors);
}

/******************************************************************************
 *
 * uglGenericClutMapNearest - Map to nearest match
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutMapNearest (
    UGL_GENERIC_DRIVER * pDrv,
    UGL_ARGB *           pMapColors,
    UGL_ARGB  *          pActualColors,
    UGL_COLOR *          pUglColors,
    UGL_SIZE             numColors
    ) {

    return uglCommonClutMapNearest (pDrv->pClut, pMapColors,
                                    pActualColors, pUglColors, numColors);
}

/******************************************************************************
 *
 * uglGenericClutAllocIndexed - Allocate color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericColorAllocIndexed (
    UGL_DEVICE_ID  devId,
    UGL_ARGB *     pReqColors,
    UGL_ORD *      pIndex,
    UGL_ARGB *     pActualColors,
    UGL_COLOR *    pUglColors,
    UGL_SIZE       numColors
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_CLUT *           pClut;
    UGL_ARGB             reqColor;
    UGL_ARGB             actualColor;
    UGL_INT32            index;
    UGL_INT32            i;

    if (devId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get data */
    pDrv  = (UGL_GENERIC_DRIVER *) devId;
    pClut = pDrv->pClut;

    for (i = 0; i < numColors; i++) {
        reqColor = pReqColors[i];

        /* Check if index is set or not */
        if (pIndex == UGL_NULL) {
            index = -1;
        }
        else {
            index = pIndex[i];
        }

        /* Call common method and update hardware palette if new allocated */
        if (uglCommonClutAlloc (pClut, &index,
                                reqColor, &actualColor) == UGL_TRUE) {
            (*devId->clutSet) (devId, index, &reqColor, 1);
        }

        /* Store actual color */
        if (pActualColors != NULL) {
            pActualColors[i] = actualColor;
        }

        /* Store index */
        if (pUglColors != NULL) {
            pUglColors[i] = (UGL_COLOR) index;
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericClutFreeIndexed - Free color
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericColorFreeIndexed (
    UGL_DEVICE_ID  devId,
    UGL_COLOR *    pColors,
    UGL_SIZE       numColors
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_CLUT *           pClut;
    UGL_COLOR            mask;

    if (devId == UGL_NULL) {
        return UGL_STATUS_ERROR;
    }

    /* Setup vars */
    pDrv  = (UGL_GENERIC_DRIVER *) devId;
    pClut = pDrv->pClut;
    mask  = (1 << devId->pMode->colorDepth);

    return uglCommonClutFree (pClut, pColors, numColors, mask);
}

/******************************************************************************
 *
 * uglGenericClutDestroy - Free palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericClutDestroy (
    UGL_GENERIC_DRIVER * pDrv
    ) {

    if (uglCommonClutDestroy (pDrv->pClut) != UGL_STATUS_OK) {
        return (UGL_STATUS_ERROR);
    }

    pDrv->pClut = UGL_NULL;

    return (UGL_STATUS_OK);
}

