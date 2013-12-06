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

/* udcclr.h - Universal graphics library color support */

#ifndef _udcclr_h
#define _udcclr_h

#include <ugl/ugl.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ugl_clut_entry {
    UGL_ARGB   color;                   /* Color argb value */
    UGL_INT32  useCount;                /* Number of users */
    UGL_INT32  nextIndex;               /* Next index */
    UGL_INT32  prevIndex;               /* Previous index */
} UGL_CLUT_ENTRY;

typedef struct ugl_clut {
    UGL_INT32        numColors;         /* Number of colors */
    UGL_INT32        firstFreeIndex;    /* First free slot */
    UGL_INT32        firstUsedIndex;    /* First used slot */
    UGL_CLUT_ENTRY * clut;              /* Clut entry */
} UGL_CLUT;

/* Functions */

/******************************************************************************
 *
 * uglCommonClutCreate - Create palette
 *
 * RETURNS: Pointer to clut
 */

UGL_CLUT * uglCommonClutCreate (
    UGL_SIZE numColors
    );

/******************************************************************************
 *
 * uglCommonClutSet - Set palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglCommonClutSet (
    UGL_CLUT * pClut,
    UGL_ORD    offset,
    UGL_ARGB * pColors,
    UGL_SIZE numColors
    );

/******************************************************************************
 *
 * uglCommonClutGet - Get software palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglCommonClutGet (
    UGL_CLUT * pClut,
    UGL_ORD    offset,
    UGL_ARGB * pColors,
    UGL_SIZE   numColors
    );

/******************************************************************************
 *
 * uglCommonClutMapNearest - Map to nearest match
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglCommonClutMapNearest (
    UGL_CLUT *  pClut,
    UGL_ARGB *  pMapColors,
    UGL_ARGB *  pActualColors,
    UGL_COLOR * pUglColors,
    UGL_SIZE    numColors
    );

/******************************************************************************
 *
 * uglCommonClutAlloc - Allocate color
 *
 * RETURNS: UGL_FALSE if no need to allocate a new color else UGL_TRUE
 */

UGL_BOOL uglCommonClutAlloc (
    UGL_CLUT * pClut,
    UGL_ORD *  pIndex,
    UGL_ARGB   reqColor,
    UGL_ARGB * pActualColor
    );

/******************************************************************************
 *
 * uglCommonClutFree - Free color users
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCommonClutFree (
    UGL_CLUT *  pClut,
    UGL_COLOR * pColors,
    UGL_SIZE    numColors,
    UGL_COLOR   mask
    );

/******************************************************************************
 *
 * uglCommonClutDestroy - Free palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCommonClutDestroy(
    UGL_CLUT * pClut
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udcclr_h */

