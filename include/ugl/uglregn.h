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

/* uglregn.h - Universal graphics library region support */

#ifndef _uglregn_h
#define _uglregn_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef struct ugl_region_rect {
    UGL_POS                  left;
    UGL_POS                  top;
    UGL_POS                  right;
    UGL_POS                  bottom;
    struct ugl_region_rect * pNextTL2BR;
    struct ugl_region_rect * pPrevTL2BR;
    struct ugl_region_rect * pNextTR2BL;
    struct ugl_region_rect * pPrevTR2BL;
} UGL_REGION_RECT;

typedef struct ugl_region_block {
    UGL_REGION_RECT *         pRectBlock;
    struct ugl_region_block * pNextBlock;
} UGL_REGION_BLOCK;

typedef struct ugl_region {
    UGL_REGION_RECT * pFirstTL2BR;
    UGL_REGION_RECT * pLastTL2BR;
    UGL_REGION_RECT * pFirstTR2BL;
    UGL_REGION_RECT * pLastTR2BL;
} UGL_REGION;

typedef UGL_REGION * UGL_REGION_ID;

/* Functions */

/******************************************************************************
 *
 * uglRegionCreate - Create a new region
 *
 * RETURNS: REGION_ID or UGL_NULL
 */

UGL_REGION_ID uglRegionCreate (
    void
    );

/******************************************************************************
 *
 * uglRegionInit - Initialize region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionInit (
    UGL_REGION_ID  regionId
    );

/******************************************************************************
 *
 * uglRegionDeinit - Deinitialize region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionDeinit (
    UGL_REGION_ID  regionId
    );

/******************************************************************************
 *
 * uglRegionDestroy - Destroy region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionDestroy (
    UGL_REGION_ID  regionId
    );

/******************************************************************************
 *
 * uglRegionEmpty - Clear region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionEmpty (
    UGL_REGION_ID  regionId
    );

/******************************************************************************
 *
 * uglRegionRectAdd - Add rectangle to region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionRectAdd (
    UGL_REGION_ID  regionId,
    UGL_RECT *     pRect
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglregn_h */

