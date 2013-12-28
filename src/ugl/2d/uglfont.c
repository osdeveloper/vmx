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

/* uglfont.c - Universal graphics library font driver support */

#include <ugl/ugl.h>

/******************************************************************************
 *
 * uglFontDriverInfo - Get information about font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglFontDriverInfo (
    UGL_FONT_DRIVER_ID  fontDriverId,
    UGL_INFO_REQ        infoRequest,
    void *              pInfo
    ) {
    UGL_STATUS  status;

    /* Validate */
    if (fontDriverId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Call driver specific method */
    status = (*fontDriverId->fontDriverInfo) (fontDriverId, infoRequest, pInfo);

    return (status);
}

/******************************************************************************
 *
 * uglFontDriverDestroy - Destroy font driver
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglFontDriverDestroy (
    UGL_FONT_DRIVER_ID  fontDriverId
    ) {
    UGL_STATUS  status;

    /* Validate */
    if (fontDriverId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Call driver specific method */
    status = (*fontDriverId->fontDriverDestroy) (fontDriverId);

    return (status);
}

/******************************************************************************
 *
 * uglFontFindFirst - Get the first avilable font
 *
 * RETURNS: UGL_SEARCH_ID or UGL_NULL
 */

UGL_SEARCH_ID uglFontFindFirst (
    UGL_FONT_DRIVER_ID  fontDriverId,
    UGL_FONT_DESC *     pFontDescriptor
    ) {
    UGL_SEARCH_ID  searchId;

    /* Validate */
    if (fontDriverId == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Call driver specific method */
    searchId = (*fontDriverId->fontFindFirst) (fontDriverId, pFontDescriptor);

    return (searchId);
}

/******************************************************************************
 *
 * uglFontCreate - Create font
 *
 * RETURNS: UGL_FONT_ID or UGL_NULL
 */

UGL_FONT_ID uglFontCreate (
    UGL_FONT_DRIVER_ID  fontDriverId,
    UGL_FONT_DEF *      pFontDefinition
    ) {
    UGL_FONT * pFont;

    if (fontDriverId == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Call driver specific method */
    pFont = (*fontDriverId->fontCreate) (fontDriverId, pFontDefinition);

    return (pFont);
}

/******************************************************************************
 *
 * uglFontDestroy - Destroy font
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglFontDestroy (
    UGL_FONT_ID  fontId
    ) {
    UGL_FONT_DRIVER * pFontDriver;
    UGL_STATUS        status;

    /* Validate */
    if (fontId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get font driver */
    pFontDriver = fontId->pFontDriver;
    if (pFontDriver == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Call driver specific method */
    status = (*pFontDriver->fontDestroy) (fontId);

    return (status);
}

