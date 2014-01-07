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

/* udbmffnt.h - Bitmap font driver for Universal Graphics Library */

#ifndef _udbmffnt_h
#define _udbmffnt_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include "ugltypes.h"
#include "uglfont.h"
#include "uglugi.h"

/* Defines */

#define UGL_BMF_DRIVER_NAME             "BMF Font"
#define UGL_FONT_DRIVER_CREATE          uglBMFFontDriverCreate

#define UGL_BMF_GLYPH_CACHE_SIZE_MAX    -1
#define UGL_BMF_GLYPH_CACHE_SIZE_NONE   0
#define UGL_BMF_GLYPH_CACHE_SIZE_MIN    UGL_BMF_GLPYH_CACHE_SIZE_NONE

#define UGL_BMF_FONT_PAGE_SIZE          256
#define UGL_BMF_FONT_PAGE_TABLE_SIZE    256

#define UGL_BMF_GLYPH_IN_CACHE          0xabcd

/* Types */

typedef void * UGL_FONT_PAGE[UGL_BMF_FONT_PAGE_SIZE];

typedef struct ugl_bmf_font_desc {
    UGL_FONT_DESC             header;
    UGL_SIZE                  leading;
    UGL_SIZE                  maxAscent;
    UGL_SIZE                  maxDescent;
    UGL_SIZE                  maxAdvance;
    const UGL_UINT8 * const * pageData;
} UGL_BMF_FONT_DESC;

typedef struct ugl_bmf_font {
    UGL_FONT                  header;
    UGL_ORD                   textOrigin;
    const UGL_BMF_FONT_DESC * pBMFFontDesc;
    UGL_UINT32                referenceCount;
    UGL_FONT_PAGE             pageZero;
    UGL_FONT_PAGE *           pageTable[UGL_BMF_FONT_PAGE_TABLE_SIZE];
    struct ugl_bmf_font *     pNextFont;
    struct ugl_bmf_font *     pPrevFont;
} UGL_BMF_FONT;

typedef struct ugl_glyph_cache_element {
    UGL_UINT16                       cacheFlag;
    UGL_UINT8                        width;
    UGL_UINT8                        height;
    UGL_UINT8                        ascent;
    UGL_RECT                         bitmapRect;
    UGL_MDDB_ID                      bitmapId;
    void *                           pGlyphData;
    void **                          ppPageElement;
    struct ugl_glyph_cache_element * pNext;
    struct ugl_glyph_cache_element * pPrev;
} UGL_GLYPH_CACHE_ELEMENT;

typedef struct ugl_bmf_font_driver {
    UGL_FONT_DRIVER           header;
    UGL_BMF_FONT *            pFirstFont;
    UGL_BMF_FONT *            pLastFont;
    UGL_FONT_DESC *           pFontList;
    UGL_SIZE                  numCachedGlyphs;
    UGL_ORD                   textOrigin;
    UGL_LOCK_ID               lockId;
    UGL_GLYPH_CACHE_ELEMENT * pFirstCacheElement;
    UGL_GLYPH_CACHE_ELEMENT * pLastCacheElement;
} UGL_BMF_FONT_DRIVER;

/* Functions */

/******************************************************************************
 *
 * uglBMFFontDriverCreate - Create bitmap font driver
 *
 * RETURNS: UGL_FONT_DRIVER_ID or UGL_NULL
 */

UGL_FONT_DRIVER_ID uglBMFFontDriverCreate (
    UGL_DEVICE_ID  devId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _udbmffnt_h */

