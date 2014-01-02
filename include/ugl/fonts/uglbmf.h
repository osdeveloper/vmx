/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2014 Surplus Users Ham Society
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

/* uglbmf.h - Bitmap font definitions for Universal Graphics Library */

#ifndef _uglbmf_h
#define _uglbmf_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <ugl/ugl.h>

/* Imports */

extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_Oblique_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_Oblique_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_Oblique_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_Oblique_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_Oblique_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_Oblique_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Bold_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_Oblique_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Courier_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_Oblique_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Oblique_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_Oblique_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Oblique_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_Oblique_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Oblique_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_Oblique_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Oblique_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_Oblique_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Oblique_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_Oblique_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Bold_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_Oblique_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Helvetica_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Italic_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Roman_8;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Italic_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Roman_10;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Italic_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Roman_12;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Italic_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Roman_14;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Italic_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Roman_18;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_Italic_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Bold_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Italic_24;
extern const UGL_BMF_FONT_DESC uglBMFFont_Times_Roman_24;

/* Macros */

#define UGL_BMF_FONT_FAMILY_COURIER \
    &uglBMFFont_Courier_Bold_Oblique_8, \
    &uglBMFFont_Courier_Bold_8, \
    &uglBMFFont_Courier_Oblique_8, \
    &uglBMFFont_Courier_8, \
    &uglBMFFont_Courier_Bold_Oblique_10, \
    &uglBMFFont_Courier_Bold_10, \
    &uglBMFFont_Courier_Oblique_10, \
    &uglBMFFont_Courier_10, \
    &uglBMFFont_Courier_Bold_Oblique_12, \
    &uglBMFFont_Courier_Bold_12, \
    &uglBMFFont_Courier_Oblique_12, \
    &uglBMFFont_Courier_12, \
    &uglBMFFont_Courier_Bold_Oblique_14, \
    &uglBMFFont_Courier_Bold_14, \
    &uglBMFFont_Courier_Oblique_14, \
    &uglBMFFont_Courier_14, \
    &uglBMFFont_Courier_Bold_Oblique_18, \
    &uglBMFFont_Courier_Bold_18, \
    &uglBMFFont_Courier_Oblique_18, \
    &uglBMFFont_Courier_18, \
    &uglBMFFont_Courier_Bold_Oblique_24, \
    &uglBMFFont_Courier_Bold_24, \
    &uglBMFFont_Courier_Oblique_24, \
    &uglBMFFont_Courier_24

#define UGL_BMF_FONT_FAMILY_HELVETICA \
    &uglBMFFont_Helvetica_Bold_Oblique_8, \
    &uglBMFFont_Helvetica_Bold_8, \
    &uglBMFFont_Helvetica_Oblique_8, \
    &uglBMFFont_Helvetica_8, \
    &uglBMFFont_Helvetica_Bold_Oblique_10, \
    &uglBMFFont_Helvetica_Bold_10, \
    &uglBMFFont_Helvetica_Oblique_10, \
    &uglBMFFont_Helvetica_10, \
    &uglBMFFont_Helvetica_Bold_Oblique_12, \
    &uglBMFFont_Helvetica_Bold_12, \
    &uglBMFFont_Helvetica_Oblique_12, \
    &uglBMFFont_Helvetica_12, \
    &uglBMFFont_Helvetica_Bold_Oblique_14, \
    &uglBMFFont_Helvetica_Bold_14, \
    &uglBMFFont_Helvetica_Oblique_14, \
    &uglBMFFont_Helvetica_14, \
    &uglBMFFont_Helvetica_Bold_Oblique_18, \
    &uglBMFFont_Helvetica_Bold_18, \
    &uglBMFFont_Helvetica_Oblique_18, \
    &uglBMFFont_Helvetica_18, \
    &uglBMFFont_Helvetica_Bold_Oblique_24, \
    &uglBMFFont_Helvetica_Bold_24, \
    &uglBMFFont_Helvetica_Oblique_24, \
    &uglBMFFont_Helvetica_24

#define UGL_BMF_FONT_FAMILY_TIMES \
    &uglBMFFont_Times_Bold_Italic_8, \
    &uglBMFFont_Times_Bold_8, \
    &uglBMFFont_Times_Italic_8, \
    &uglBMFFont_Times_Roman_8, \
    &uglBMFFont_Times_Bold_Italic_10, \
    &uglBMFFont_Times_Bold_10, \
    &uglBMFFont_Times_Italic_10, \
    &uglBMFFont_Times_Roman_10, \
    &uglBMFFont_Times_Bold_Italic_12, \
    &uglBMFFont_Times_Bold_12, \
    &uglBMFFont_Times_Italic_12, \
    &uglBMFFont_Times_Roman_12, \
    &uglBMFFont_Times_Bold_Italic_14, \
    &uglBMFFont_Times_Bold_14, \
    &uglBMFFont_Times_Italic_14, \
    &uglBMFFont_Times_Roman_14, \
    &uglBMFFont_Times_Bold_Italic_18, \
    &uglBMFFont_Times_Bold_18, \
    &uglBMFFont_Times_Roman_18, \
    &uglBMFFont_Times_Bold_Italic_24, \
    &uglBMFFont_Times_Bold_24, \
    &uglBMFFont_Times_Italic_24, \
    &uglBMFFont_Times_Roman_24

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglbmf_h */

