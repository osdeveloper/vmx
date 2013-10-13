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

/* vga.h - VGA screen output driver */

#ifndef _vga_h
#define _vga_h

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/******************************************************************************
 * vgaInit - Initialize frame buffer 
 * 
 * RETURNS: N/A
 */

void vgaInit(
    void
    );

/******************************************************************************
 * vgaClear - Clear screen
 *
 * RETURNS: N/A
 */

void vgaClear(
    void
    );

/******************************************************************************
 * vgaMoveCsr - Update the hardware cursor, blink, blink
 *
 * RETURNS: N/A
 */

void vgaMoveCsr(
    void
    );

/******************************************************************************
 * vgaScroll - Scroll screen
 *
 * RETURNS: N/A
 */

void vgaScroll(
    void
    );

/******************************************************************************
 * vgaPutch - Put a single character on screen
 *
 * RETURNS: N/A
 */

void vgaPutch(
    char c
    );

/******************************************************************************
 * vgaPuts - Print a line on screen
 *
 * RETURNS: N/A
 */

void vgaPuts(
    const char *str
    );

/******************************************************************************
 * vgaSetColor - Set bg/fg color
 *
 * RETURNS: N/A
 */

void vgaSetColor(
    u_int8_t fg, 
    u_int8_t bg
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _vga_h */

