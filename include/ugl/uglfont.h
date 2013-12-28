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

/* uglfont.h - Font driver support for Universal Graphics Library */

#ifndef _uglfont_h
#define _uglfont_h

/* Defines */

#define UGL_FONT_FACE_NAME_MAX_LENGTH   80
#define UGL_FONT_FAMILY_NAME_MAX_LENGTH 40

/* Used by uglFontDriverInfo */
#define UGL_FONT_ENGINE_VERSION_GET     100
#define UGL_FONT_DRIVER_VERSION_GET     101
#define UGL_FONT_TEXT_ORIGIN            110

#define UGL_FONT_TEXT_BASELINE          0
#define UGL_FONT_TEXT_UPPER_LEFT        1

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */

struct ugl_font_driver;

/* Types */

typedef struct ugl_font_def {
    UGL_SIZE   structSize;
    UGL_RANGE  pixelSize;
    UGL_RANGE  weight;
    UGL_ORD    italic;
    UGL_ORD    charSet;
    char       faceName[UGL_FONT_FACE_NAME_MAX_LENGTH];
    char       familyName[UGL_FONT_FAMILY_NAME_MAX_LENGTH];
} UGL_FONT_DEF;

typedef struct ugl_font_desc {
    UGL_RANGE  pixelSize;
    UGL_RANGE  weight;
    UGL_ORD    italic;
    UGL_ORD    spacing;
    UGL_ORD    charSet;
    char       faceName[UGL_FONT_FACE_NAME_MAX_LENGTH];
    char       familyName[UGL_FONT_FAMILY_NAME_MAX_LENGTH];
} UGL_FONT_DESC;

typedef struct ugl_font {
    struct ugl_font_driver * pFontDriver;
} UGL_FONT;
typedef UGL_FONT * UGL_FONT_ID;

typedef struct ugl_font_driver {
    UGL_UGI_DRIVER * pDriver;

    UGL_STATUS    (*fontDriverInfo) (
        struct ugl_font_driver * pFontDriver,
        UGL_INFO_REQ             infoRequest,
        void *                   pInfo
        );

    UGL_STATUS    (*fontDriverDestroy) (
        struct ugl_font_driver * pFontDriver
        );

    UGL_SEARCH_ID (*fontFindFirst) (
        struct ugl_font_driver * pFontDriver,
        struct ugl_font_desc *   pFontDesc
        );

    UGL_FONT *    (*fontCreate) (
        struct ugl_font_driver * pFontDriver,
        struct ugl_font_def *    pFontDefinition
        );

    UGL_STATUS    (*fontDestroy) (
        struct ugl_font * pFont
        );

} UGL_FONT_DRIVER;
typedef UGL_FONT_DRIVER * UGL_FONT_DRIVER_ID;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglfont_h */

