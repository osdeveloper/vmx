/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 - 2009 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* vgahw.h - Vga driver */

/*
    Copyright © 1995-2001, The AROS Development Team. All rights reserved.
    $Id: vgahw.h,v 1.7 2009/01/11 16:07:49 alexuspol Exp $

    Desc: VGA hardwired.
    Lang: English.
*/

#ifndef _vgahw_h
#define _vgahw_h

/* CRT controller register indices */

#define CRTC_H_TOTAL	    0
#define CRTC_H_DISPLAY	    1
#define CRTC_H_BLANK_START  2
#define CRTC_H_BLANK_END    3
#define CRTC_H_SYNC_START   4
#define CRTC_H_SYNC_END	    5
#define CRTC_V_TOTAL	    6
#define CRTC_OVERFLOW	    7
#define CRTC_PRESET_ROW	    8
#define CRTC_MAX_SCAN	    9
#define CRTC_CURSOR_START   10
#define CRTC_CURSOR_END	    11
#define CRTC_START_HI	    12
#define CRTC_START_LO	    13
#define CRTC_CURSOR_HI	    14
#define CRTC_CURSOR_LO	    15
#define CRTC_V_SYNC_START   16
#define CRTC_V_SYNC_END	    17
#define CRTC_V_DISP_END	    18
#define CRTC_OFFSET	    19
#define CRTC_UNDERLINE	    20
#define CRTC_V_BLANK_START  21
#define CRTC_V_BLANK_END    22
#define CRTC_MODE	    23
#define CRTC_LINE_COMPARE   24

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <ugl/ugl.h>

/* This structure keeps contents of mode registers including palette */

struct vgaHWRec
{
  u_int8_t MiscOutReg;     /* */
  u_int8_t CRTC[25];       /* Crtc Controller */
  u_int8_t Sequencer[5];   /* Video Sequencer */
  u_int8_t Graphics[9];    /* Video Graphics */
  u_int8_t Attribute[21];  /* Video Atribute */
  u_int8_t DAC[768];       /* Internal Colorlookuptable */
  int8_t NoClock;                 /* number of selected clock */
};

struct vgaHWBitmap
{
    unsigned int	width;		/* Width of bitmap */
    unsigned int 	height;		/* Height of bitmap */
    unsigned char	*VideoData;	/* Pointing to video data */
};

struct vgaHWBox
{
    int x1, y1;
    int x2, y2;
};

#define DACDelay \
	{ \
		unsigned char temp = sysInByte(vgaIOBase + 0x0A); \
		temp = sysInByte(vgaIOBase + 0x0A); \
	}

extern void vgaLoadPalette(struct vgaHWRec *regs, unsigned char *pal);
extern void vgaSaveScreen(int start);
extern int vgaBlankScreen(int on);
extern void vgaDACLoad(struct vgaHWRec *restore, unsigned char start, int num);
extern void vgaRestore(struct vgaHWRec *restore, BOOL onlyDac);
extern void *vgaSave(struct vgaHWRec *save);
extern int vgaInitMode(UGL_MODE *mode, struct vgaHWRec *regs);
extern void vgaRefreshArea(struct vgaHWBitmap *bmap, int num,
			   struct vgaHWBox *pbox);
extern void vgaPutPixel(int x, int y, int pixel);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _vgahw_h */

