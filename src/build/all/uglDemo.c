/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2014 Surplus Users Ham Society
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

/* uglDemo.c - Demo program for universal graphics library */

#include "configAll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <a.out.h>

#include <vmx.h>
#include <os/symLib.h>
#include <os/memPartLib.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/vga/udvga.h>
#include <ugl/driver/graphics/vga/udvgamode.h>
#include <ugl/driver/graphics/generic/udgen.h>
#include <ugl/driver/font/udbmffnt.h>
#include <ugl/fonts/uglbmf.h>

#include "vmxball.cbm"
#include "pinball.cbm"
#include "cursor.cbm"
#include "font8x16.cfs"

#define PAL_LENGTH             16
#define BALL_SPEED             4
#define DB_CLEAR_COLOR         0x06
#define FOREGROUND_COLOR       14
#define BACKGROUND_COLOR       4
#define TEXT_FG_COLOR          0
#define TEXT_BG_COLOR          DB_CLEAR_COLOR
#define MAX_FONTS              32

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Exports */
const UGL_BMF_FONT_DESC * uglBMFFontData[] = {
    UGL_BMF_FONT_FAMILY_COURIER,
    UGL_BMF_FONT_FAMILY_HELVETICA,
    UGL_BMF_FONT_FAMILY_TIMES,
    UGL_NULL
};

UGL_MEM_POOL_ID uglBMFGlyphCachePoolId = UGL_NULL;
UGL_SIZE uglBMFGlyphCacheSize = 256;

PART_ID gfxPartId;
UGL_DEVICE_ID gfxDevId;
UGL_FONT_DRIVER_ID fntDrvId;
UGL_DIB bgDib, fgDib;
UGL_MDIB mDib;
UGL_CDIB cDib;
UGL_RASTER_OP rasterOp = UGL_RASTER_OP_COPY;
BOOL doubleBuffer = TRUE;
BOOL fillPattern = FALSE;
int animTreshold = 1;
UGL_SIZE lineWidth = 1;
UGL_LINE_STYLE lineStyle = UGL_LINE_STYLE_SOLID;

int createDib(void)
{
  int i, j;

  bgDib.width = vmxballWidth;
  bgDib.height = vmxballHeight;
  bgDib.stride = vmxballWidth;
  bgDib.imageFormat = UGL_DIRECT;
  bgDib.colorFormat = UGL_DEVICE_COLOR_32;
  bgDib.clutSize = 16;
  bgDib.pClut = malloc(sizeof(UGL_COLOR) * 16);
  if (bgDib.pClut == NULL) {
    printf("Error creating clut\n");
    return 1;
  }
  for (i = 0; i < 16; i++)
    ((UGL_COLOR *) bgDib.pClut)[i] = UGL_MAKE_ARGB(1,
		    		     16 * vmxballClut[i][0] / 255,
		    		     16 * vmxballClut[i][1] / 255,
		    		     16 * vmxballClut[i][2] / 255);
  bgDib.pData = vmxballData;

  fgDib.width = pinballWidth;
  fgDib.height = pinballHeight;
  fgDib.stride = pinballWidth;
  fgDib.imageFormat = UGL_DIRECT;
  fgDib.colorFormat = UGL_DEVICE_COLOR_32;
  fgDib.clutSize = 16;
  fgDib.pClut = malloc(sizeof(UGL_COLOR) * 16);
  if (fgDib.pClut == NULL) {
    printf("Error creating clut\n");
    return 1;
  }
  for (i = 0; i < 16; i++)
    ((UGL_COLOR *) fgDib.pClut)[i] = UGL_MAKE_ARGB(1,
		    		     pinballClut[i][0],
		    		     pinballClut[i][1],
		    		     pinballClut[i][2]);
  fgDib.pData = pinballData;

  mDib.width = pinballWidth;
  mDib.height = pinballHeight;
  mDib.stride = pinballWidth;
  mDib.pData = pinballMask;

  cDib.width = cursorWidth;
  cDib.height = cursorHeight;
  cDib.stride = cursorWidth;
  cDib.hotSpot.x = 0;
  cDib.hotSpot.y = 0;
  cDib.clutSize = fgDib.clutSize;
  cDib.pClut = fgDib.pClut;
  cDib.pData = cursorData;

  return 0;
}

void setPalette(void)
{
  int i;
  UGL_ORD paletteIndex[PAL_LENGTH];
  UGL_ARGB palette[PAL_LENGTH];
  UGL_COLOR colors[PAL_LENGTH];

  for (i = 0; i < PAL_LENGTH; i++) {
    paletteIndex[i] = i;
    palette[i] = UGL_MAKE_ARGB(255,			/* ALPHA */
			       vmxballClut[i][0],	/* RED */
			       vmxballClut[i][1],	/* GREEN */
			       vmxballClut[i][2]);	/* BLUE */
  }

  uglColorAlloc(gfxDevId, palette, paletteIndex, colors, PAL_LENGTH);
}

int mode4Enter(struct vgaHWRec *oldRegs)
{
  UGL_MODE gfxMode;

  if (gfxDevId == UGL_NULL) {
    return 1;
  }

  vgaSave(oldRegs);

  /* Enter video mode */
  gfxMode.width = 640;
  gfxMode.height = 480;
  gfxMode.colorDepth = 4;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
      return 1;
  }

  setPalette();

  return 0;
}

void restoreConsole(struct vgaHWRec *regs)
{
  vgaRestore(regs, FALSE);
  vgaLoadFont(font8x16, font8x16Height);
  printf("%c\n", 0x0c);
}

int uglPixel4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }
  uglClipRegionSet (gc, clipRegionId);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    uglRasterModeSet(gc, rasterOp);
    uglPixelSet(gc, rand() % 640, rand() % 480, rand () % 16);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    taskDelay(animTreshold);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglHLine4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i, y, x, x1, x2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    x1 = rand () % 640;
    x2 = rand () % 640;
    y = rand () % 480;
    if (x1 > x2) {
      x = x1;
      x1 = x2;
      x2 = x1;
    }

    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet (gc, rand () % 16);
    uglLine (gc, x1, y, x2, y);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    taskDelay(animTreshold);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglVLine4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i, x, y, y1, y2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    x = rand () % 640;
    y1 = rand () % 480;
    y2 = rand () % 480;
    if (y1 > y2) {
      y = y1;
      y1 = y2;
      y2 = y1;
    }

    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet (gc, rand () % 16);
    uglLine (gc, x, y1, x, y2);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglLine4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet (gc, rand () % 16);
    uglLine (gc, rand() % 640, rand() % 480,
             rand () % 640, rand() % 480);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglRect4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_MDDB *pMddb;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  UGL_GC_ID gc;
  struct vgaHWRec oldRegs;
  int i;
  int x, x1, x2;
  int y, y1, y2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (fillPattern == TRUE) {
    pMddb = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
                                0, gfxPartId);
    if (pMddb == UGL_NULL) {
      if (doubleBuffer == TRUE) {
        uglBitmapDestroy(gfxDevId, pDbBmp);
      }
      uglGcDestroy(gc);
      restoreConsole(&oldRegs);
      printf("Unable to create monochrome image\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  if (fillPattern == TRUE) {
    uglFillPatternSet (gc, pMddb);
  }

  for (i = 0; i < maxtimes; i++) {
    do {
      x1 = rand() % 640;
      y1 = rand () % 480;
      x2 = rand () % 640;
      y2 = rand () % 480;

      if (x1 > x2) {
        x = x1;
        x1 = x2;
        x2 = x1;
      }

      if (y1 > y2) {
        y = y1;
        y1 = y2;
        y2 = y1;
      }
    } while ((x2 - x1) <= 1 || (y2 - y1) <= 1);

    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet(gc, rand () % 16);
    uglBackgroundColorSet(gc, rand () % 16);
    uglRectangle(gc, x1, y1, x2, y2);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (fillPattern == TRUE) {
    uglFillPatternSet (gc, UGL_NULL);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  if (fillPattern == TRUE) {
    uglMonoBitmapDestroy(gfxDevId, pMddb);
  }
  restoreConsole(&oldRegs);

  return 0;
}

int uglPoly4Test(int maxtimes, int nPoints, UGL_REGION_ID clipRegionId)
{
  static UGL_POS points[64];
  UGL_DDB *pDbBmp;
  UGL_MDDB *pMddb;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i, j;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (nPoints < 3) {
      nPoints = 3;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (fillPattern == TRUE) {
    pMddb = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
                                0, gfxPartId);
    if (pMddb == UGL_NULL) {
      if (doubleBuffer == TRUE) {
        uglBitmapDestroy(gfxDevId, pDbBmp);
      }
      uglGcDestroy(gc);
      restoreConsole(&oldRegs);
      printf("Unable to create monochrome image\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  if (fillPattern == TRUE) {
    uglFillPatternSet (gc, pMddb);
  }

  for (i = 0; i < maxtimes; i++) {
    for (j = 0; j < nPoints; j++) {
      points[j * 2] = rand() % 640;
      points[j * 2 + 1] = rand () % 480;
    }

    points[nPoints * 2] = points[0];
    points[nPoints * 2 + 1] = points[1];

    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet(gc, rand () % 16);
    uglBackgroundColorSet(gc, rand () % 16);
    uglPolygon(gc, nPoints + 1, points);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (fillPattern == TRUE) {
    uglFillPatternSet (gc, UGL_NULL);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  if (fillPattern == TRUE) {
    uglMonoBitmapDestroy(gfxDevId, pMddb);
  }
  restoreConsole(&oldRegs);

  return 0;
}

int uglBlt4Test(UGL_REGION_ID clipRegionId, int x1, int y1, int x2, int y2)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_DDB *pDbBmp, *pFgBmp, *pSaveBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglClipRegionSet (gc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     gc->pDefaultBitmap,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to write background image\n");
    return 1;
  }

  pFgBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_DATA,
                           8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    uglBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  srcRect.left = x1;
  srcRect.top = y1;
  if (x2 == 0) {
    srcRect.right = pFgBmp->width;
  }
  else {
    srcRect.right = x2;
  }

  if (y2 == 0) {
    srcRect.bottom = pFgBmp->height;
  }
  else {
    srcRect.bottom = y2;
  }

  pt.x = -pFgBmp->width;
  pt.y = -pFgBmp->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 480) {

    /* Copy background */
    uglBitmapBlt(gc, gc->pDefaultBitmap,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    /* Set raster operation and draw ball */
    uglRasterModeSet(gc, rasterOp);

    uglBitmapBlt(gc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  uglBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglMono4Test(UGL_REGION_ID clipRegionId, int x1, int y1, int x2, int y2)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_MDDB *pFgBmp;
  UGL_DDB *pDbBmp, *pSaveBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglClipRegionSet (gc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglForegroundColorSet(gc, FOREGROUND_COLOR);
  uglBackgroundColorSet(gc, BACKGROUND_COLOR);

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     gc->pDefaultBitmap,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to write background image\n");
    return 1;
  }

  pFgBmp = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
                               0, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    uglMonoBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  srcRect.left = x1;
  srcRect.top = y1;
  if (x2 == 0) {
    srcRect.right = pFgBmp->width;
  }
  else {
    srcRect.right = x2;
  }

  if (y2 == 0) {
    srcRect.bottom = pFgBmp->height;
  }
  else {
    srcRect.bottom = y2;
  }

  pt.x = -pFgBmp->width;
  pt.y = -pFgBmp->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 480) {

    /* Copy background */
    uglBitmapBlt(gc, gc->pDefaultBitmap,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    /* Set raster operation and draw ball */
    uglRasterModeSet(gc, rasterOp);

    uglBitmapBlt(gc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  uglBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglTrans4Test(UGL_REGION_ID clipRegionId)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_DDB *pDbBmp, *pSaveBmp;
  UGL_TDDB *pFgBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglClipRegionSet (gc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, UGL_NULL);
  }

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     gc->pDefaultBitmap,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to write background image\n");
    return 1;
  }

  pFgBmp = uglTransBitmapCreate(gfxDevId, &fgDib, &mDib, UGL_DIB_INIT_DATA,
                                8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    uglTransBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  srcRect.left = 0;
  srcRect.right = pFgBmp->width;
  srcRect.top = 0;
  srcRect.bottom = pFgBmp->height;
  pt.x = -pFgBmp->width;
  pt.y = -pFgBmp->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 480) {

    /* Copy background */
    uglBitmapBlt(gc, gc->pDefaultBitmap,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    /* Set raster operation and draw ball */
    uglRasterModeSet(gc, rasterOp);

    uglBitmapBlt(gc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  uglTransBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglCursor4Test(UGL_REGION_ID clipRegionId)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_CDDB *pFgBmp;
  UGL_GENERIC_DRIVER *pDrv;
  UGL_GEN_CURSOR_DATA *pCursorData;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (uglCursorInit (gfxDevId, cDib.width, cDib.height, 0, 0)
                     != UGL_STATUS_OK) {
      restoreConsole(&oldRegs);
      printf("Unable to initialize cursor.\n");
    return 1;
  }

  pDrv = (UGL_GENERIC_DRIVER *) gfxDevId;
  pCursorData = pDrv->pCursorData;

  uglClipRegionSet (gc, clipRegionId);

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     UGL_DISPLAY_ID,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to write background image\n");
    return 1;
  }

  pFgBmp = uglCursorBitmapCreate(gfxDevId, &cDib);
  if (pFgBmp == UGL_NULL) {
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  if (uglCursorImageSet(gfxDevId, pFgBmp) != UGL_STATUS_OK) {
    uglGcDestroy(gc);
    uglCursorBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to set cursor image\n");
    return 1;
  }

  uglCursorOn(gfxDevId);
  while (pCursorData->position.y < 480) {
    uglCursorMove (gfxDevId,
                   pCursorData->position.x + BALL_SPEED,
                   pCursorData->position.y + BALL_SPEED);

    /* Delay */
    taskDelay(animTreshold);
  }

  uglCursorBitmapDestroy(gfxDevId, pFgBmp);
  uglCursorDeinit(gfxDevId);
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int mode8Enter(struct vgaHWRec *oldRegs)
{
  UGL_MODE gfxMode;

  if (gfxDevId == UGL_NULL) {
    return 1;
  }

  vgaSave(oldRegs);

  /* Enter video mode */
  gfxMode.width = 320;
  gfxMode.height = 200;
  gfxMode.colorDepth = 8;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    return 1;
  }

  setPalette();

  return 0;
}

void mode8Background(UGL_GC_ID gc)
{
  int i, j;

  for (j = 0; j < 200; j++) {
    for(i = 0; i < 320; i++) {
      uglPixelSet(gc, i, j, j / 16);
    }
  }
}

int uglText4Test(char *fontfam, char *str)
{
  static char defFnt[] = "Times";
  static char defStr[] = "Hello World!";
  struct vgaHWRec oldRegs;
  UGL_FONT_DESC fontDesc;
  UGL_FONT_DEF  fontDef;
  UGL_SEARCH_ID searchId;
  UGL_FONT_ID fontId[MAX_FONTS + 1];
  UGL_SIZE height;
  UGL_GC_ID gc;
  UGL_INT32 i;
  UGL_POS y = 0;
  UGL_INT32 numFonts = 0;

  if (fontfam == UGL_NULL) {
      fontfam = defFnt;
  }

  if (str == UGL_NULL) {
      str = defStr;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglDefaultBitmapSet(gc, NULL);
  uglForegroundColorSet(gc, UGL_COLOR_TRANSPARENT);
  uglBackgroundColorSet(gc, TEXT_BG_COLOR);
  uglRectangle(gc, 0, 0, 639, 479);

  uglRasterModeSet(gc, rasterOp);
  uglForegroundColorSet(gc, TEXT_FG_COLOR);

  searchId = uglFontFindFirst(fntDrvId, &fontDesc);
  if (searchId == UGL_NULL) {
    restoreConsole(&oldRegs);
    printf("Unable to retreive fonts.\n");
    return 1;
  }

  do {
    if (strcmp(fontfam, fontDesc.familyName) == 0) {
      fontDef.structSize = sizeof (UGL_FONT_DEF);
      fontDef.pixelSize  = fontDesc.pixelSize.min;
      fontDef.weight = fontDesc.weight.min;
      fontDef.italic = fontDesc.italic;
      fontDef.charSet = fontDesc.charSet;
      strcpy (fontDef.faceName, fontDesc.faceName);
      strcpy (fontDef.familyName, fontDesc.familyName);

      fontId[numFonts] = uglFontCreate (fntDrvId, &fontDef);
      if (fontId[numFonts] == UGL_NULL) {
        restoreConsole(&oldRegs);
        printf("Unable to load font %s.\n", fontDesc.faceName);
        return 1;
      }
      if (++numFonts == MAX_FONTS) {
        break;
      }
    }
  } while (uglFontFindNext(fntDrvId, &fontDesc, searchId) == UGL_STATUS_OK);

  uglFontFindClose(fntDrvId, searchId);

  if (numFonts == 0) {
    restoreConsole(&oldRegs);
    printf("No fonts found for font family %s.\n", fontfam);
    return 1;
  }

  for (i = 0; i < numFonts; i++) {

    if (uglTextSizeGet(fontId[i], UGL_NULL, &height,
                       strlen(str), str) != UGL_STATUS_OK) {
      restoreConsole(&oldRegs);
      printf("Unable to retreive text size for %s.\n", str);
      return 1;
    }

    y += height;

    uglFontSet(gc, fontId[i]);
    uglTextDraw(gc, 0, y, strlen(str), str);
  }

  getchar();

  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglPixel8Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }
  uglClipRegionSet (gc, clipRegionId);

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    uglRasterModeSet(gc, rasterOp);
    uglPixelSet(gc, rand() % 320, rand() % 200, rand () % 16);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    taskDelay(animTreshold);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglHLine8Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i, y, x, x1, x2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    x1 = rand () % 320;
    x2 = rand () % 320;
    y = rand () % 200;
    if (x1 > x2) {
      x = x1;
      x1 = x2;
      x2 = x1;
    }

    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet (gc, rand () % 256);
    uglLine (gc, x1, y, x2, y);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    taskDelay(animTreshold);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglVLine8Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i, x, y, y1, y2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    x = rand () % 320;
    y1 = rand () % 200;
    y2 = rand () % 200;
    if (y1 > y2) {
      y = y1;
      y1 = y2;
      y2 = y1;
    }

    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet (gc, rand () % 256);
    uglLine (gc, x, y1, x, y2);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglLine8Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglClipRegionSet (gc, clipRegionId);
  uglLineWidthSet (gc, lineWidth);
  uglLineStyleSet (gc, lineStyle);

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    uglRasterModeSet(gc, rasterOp);
    uglForegroundColorSet (gc, rand () % 256);
    uglLine (gc, rand() % 320, rand() % 200,
             rand () % 320, rand() % 200);
    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  restoreConsole(&oldRegs);

  return 0;
}

int uglBlt8Test(UGL_REGION_ID clipRegionId, int x1, int y1, int x2, int y2)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_DDB *pDbBmp, *pFgBmp, *pSaveBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglClipRegionSet (gc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  /* Draw background */
  mode8Background(gc);

  pFgBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_DATA,
                           8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    uglBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  srcRect.left = x1;
  srcRect.top = y1;
  if (x2 == 0) {
    srcRect.right = pFgBmp->width;
  }
  else {
    srcRect.right = x2;
  }

  if (y2 == 0) {
    srcRect.bottom = pFgBmp->height;
  }
  else {
    srcRect.bottom = y2;
  }

  pt.x = -pFgBmp->width;
  pt.y = -pFgBmp->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 200) {

    /* Copy background */
    uglBitmapBlt(gc, gc->pDefaultBitmap,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    /* Set raster operation and draw ball */
    uglRasterModeSet(gc, rasterOp);

    uglBitmapBlt(gc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  uglBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglMono8Test(UGL_REGION_ID clipRegionId, int x1, int y1, int x2, int y2)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_MDDB *pFgBmp;
  UGL_DDB *pDbBmp, *pSaveBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglClipRegionSet (gc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, NULL);
  }

  uglForegroundColorSet(gc, FOREGROUND_COLOR);
  uglBackgroundColorSet(gc, BACKGROUND_COLOR);

  /* Draw background */
  mode8Background(gc);

  pFgBmp = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
                               0, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    uglMonoBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  srcRect.left = x1;
  srcRect.top = y1;
  if (x2 == 0) {
    srcRect.right = pFgBmp->width;
  }
  else {
    srcRect.right = x2;
  }

  if (y2 == 0) {
    srcRect.bottom = pFgBmp->height;
  }
  else {
    srcRect.bottom = y2;
  }

  pt.x = -pFgBmp->width;
  pt.y = -pFgBmp->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 200) {

    /* Copy background */
    uglBitmapBlt(gc, gc->pDefaultBitmap,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    /* Set raster operation and draw ball */
    uglRasterModeSet(gc, rasterOp);

    uglBitmapBlt(gc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  uglBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglTrans8Test(UGL_REGION_ID clipRegionId)
{
  struct vgaHWRec oldRegs;
  UGL_GC_ID gc;
  UGL_DDB *pDbBmp, *pSaveBmp;
  UGL_TDDB *pFgBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  gc = uglGcCreate(gfxDevId);
  if (gc == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create graphics context.\n");
    return 1;
  }

  uglClipRegionSet (gc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gc, UGL_NULL);
  }

  /* Draw background */
  mode8Background(gc);

  pFgBmp = uglTransBitmapCreate(gfxDevId, &fgDib, &mDib, UGL_DIB_INIT_DATA,
                                8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglGcDestroy(gc);
    uglTransBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  srcRect.left = 0;
  srcRect.right = pFgBmp->width;
  srcRect.top = 0;
  srcRect.bottom = pFgBmp->height;
  pt.x = -pFgBmp->width;
  pt.y = -pFgBmp->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 200) {

    /* Copy background */
    uglBitmapBlt(gc, gc->pDefaultBitmap,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    /* Set raster operation and draw ball */
    uglRasterModeSet(gc, rasterOp);

    uglBitmapBlt(gc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }
  uglGcDestroy(gc);
  uglTransBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglSetAnimTreshold(int value)
{
  if (value < 0) {
    value = 0;
  }
  animTreshold = value;
  printf("Set animation treshold to %d\n", animTreshold);
}

int uglRasterOpCopy(void)
{
  rasterOp = UGL_RASTER_OP_COPY;
  printf("Raster operation set to COPY.\n");
}

int uglRasterOpAnd(void)
{
  rasterOp = UGL_RASTER_OP_AND;
  printf("Raster operation set to AND.\n");
}

int uglRasterOpOr(void)
{
  rasterOp = UGL_RASTER_OP_OR;
  printf("Raster operation set to OR.\n");
}

int uglRasterOpXor(void)
{
  rasterOp = UGL_RASTER_OP_XOR;
  printf("Raster operation set to XOR.\n");
}

int uglToggleDoubleBuffer(void)
{
  if (doubleBuffer == TRUE) {
    doubleBuffer = FALSE;
    printf("Double buffered animation disabled.\n");
  }
  else {
    doubleBuffer = TRUE;
    printf("Double buffered animation enabled.\n");
  }
}

int uglToggleFillPattern(void)
{
  if (fillPattern == TRUE) {
    fillPattern = FALSE;
    printf("Fill pattern set to off.\n");
  }
  else {
    fillPattern = TRUE;
    printf("Fill pattern set to on.\n");
  }
}

int uglSetLineWidth(UGL_SIZE width)
{
    lineWidth = width;

    return (int) lineWidth;
}

int uglSetLineStyle(int n)
{
    switch (n) {
        case 0:
            lineStyle = UGL_LINE_STYLE_SOLID;
            break;

        case 1:
            lineStyle = UGL_LINE_STYLE_DASHED;
            break;

        case 2:
            lineStyle = UGL_LINE_STYLE_DOTTED;
            break;

        case 3:
            lineStyle = UGL_LINE_STYLE_DASH_DOTTED;
            break;

        default:
            lineStyle = (UGL_LINE_STYLE) n;
            break;
    }

    return (int) lineStyle;
}

int uglConvertTest(void)
{
  int i;
  UGL_COLOR colorArray[16];

  for (i = 0; i < 16; i++)
    printf("input[%d] = 0x%x\n", i, ((UGL_COLOR *) bgDib.pClut)[i]);
  /*
  (*gfxDevId->colorConvert)(gfxDevId, bgDib.pClut, UGL_DEVICE_COLOR,
		            colorArray, UGL_DEVICE_COLOR_32, 16);
  */
  for (i = 0; i < 16; i++)
    printf("output[%d] = 0x%x\n", i, colorArray[i]);
}

UGL_RECT* uglRectCreate(int x1, int y1, int x2, int y2)
{
  UGL_RECT *pRect;

  pRect = (UGL_RECT *) malloc(sizeof(UGL_RECT));
  if (pRect == NULL) {
    printf("Unable to create rectangle\n");
    return (NULL);
  }

  pRect->left   = x1;
  pRect->top    = y1;
  pRect->right  = x2;
  pRect->bottom = y2;

  printf("Created rectangle: %d %d %d %d\n",
    pRect->left,
    pRect->top,
    pRect->right,
    pRect->bottom);

  return (pRect);
}

int uglFontList(int index)
{
  struct vgaHWRec oldRegs;
  UGL_FONT_DESC fontDesc;
  UGL_FONT_DESC desiredFontDesc;
  UGL_FONT_DEF  fontDef;
  UGL_SEARCH_ID searchId;
  UGL_FONT_METRICS metrics;
  UGL_FONT_ID fontId;
  UGL_INT32 i = 1;
  UGL_INT32 data = -1;

  if (uglFontDriverInfo(fntDrvId, UGL_FONT_DRIVER_VERSION_GET,
                         &data) != UGL_STATUS_OK) {
    printf("Unable to retreive font driver version.\n");
    return 1;
  }

  printf("Font driver version: %d\n", data);
  printf("Searching for fonts.\n");

  searchId = uglFontFindFirst(fntDrvId, &fontDesc);
  if (searchId == UGL_NULL) {
    printf("Unable to retreive fonts.\n");
    return 1;
  }

  do {
    printf("%-02d : %-012s %-032s %02d %04s %06s\n",
           i, fontDesc.familyName, fontDesc.faceName,
           fontDesc.pixelSize.min,
           (fontDesc.weight.min == UGL_FONT_BOLD) ? "bold" : "",
           (fontDesc.italic == UGL_FONT_ITALIC) ? "italic" : "");
    if (i == index) {
        memcpy (&desiredFontDesc, &fontDesc, sizeof (UGL_FONT_DESC));
    }
    i++;
  } while (uglFontFindNext(fntDrvId, &fontDesc, searchId) == UGL_STATUS_OK);

  uglFontFindClose(fntDrvId, searchId);

  if (index > 0) {
    fontDef.structSize = sizeof (UGL_FONT_DEF);
    fontDef.pixelSize  = desiredFontDesc.pixelSize.min;
    fontDef.weight = desiredFontDesc.weight.min;
    fontDef.italic = desiredFontDesc.italic;
    fontDef.charSet = desiredFontDesc.charSet;
    strcpy (fontDef.faceName, desiredFontDesc.faceName);
    strcpy (fontDef.familyName, desiredFontDesc.familyName);

    fontId = uglFontCreate (fntDrvId, &fontDef);
    if (fontId == UGL_NULL) {
      printf("Unable to load font %s.\n", desiredFontDesc.faceName);
      return 1;
    }

    if (uglFontMetricsGet(fontId, &metrics) != UGL_STATUS_OK) {
      uglFontDestroy(fontId);
      printf("Unable to retreive metrics for font.\n", fontDef.faceName);
      return 1;
    }

    printf("Font face:\t%s\n", metrics.faceName);
    printf("Font family:\t%s\n", metrics.familyName);
    printf("Pixel size:\t%d\n", metrics.pixelSize);
    printf("Style bold:\t%s\n",
           (metrics.weight == UGL_FONT_BOLD) ? "yes" : "no");
    printf("Style italic:\t%s\n",
           (metrics.italic == UGL_FONT_ITALIC) ? "yes" : "no");
    printf("Avrage height:\t%d\n", metrics.height);
    printf("Max ascent:\t%d\n", metrics.maxAscent);
    printf("Max descent:\t%d\n", metrics.maxDescent);
    printf("Max advance:\t%d\n", metrics.maxAdvance);
    printf("Leading:\t%d\n", metrics.leading);
    printf("Spacing:\t%d\n", metrics.spacing);
    printf("Font type:\t%d\n", metrics.fontType);
    printf("Character set:\t%d\n", metrics.charSet);
    printf("Scalable:\t%s\n", (metrics.scalable == UGL_TRUE) ? "yes" : "no");

    uglFontDestroy(fontId);
  }

  return 0;
}

int uglDemoInit(void)
{
static SYMBOL symTableUglDemo[] = {
  {NULL, "_uglPixel4Test", uglPixel4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglHLine4Test", uglHLine4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglVLine4Test", uglVLine4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglLine4Test", uglLine4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglRect4Test", uglRect4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglPoly4Test", uglPoly4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglBlt4Test", uglBlt4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglMono4Test", uglMono4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglTrans4Test", uglTrans4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglCursor4Test", uglCursor4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglText4Test", uglText4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglPixel8Test", uglPixel8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglHLine8Test", uglHLine8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglVLine8Test", uglVLine8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglLine8Test", uglLine8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglBlt8Test", uglBlt8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglMono8Test", uglMono8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglTrans8Test", uglTrans8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglSetAnimTreshold", uglSetAnimTreshold, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpCopy", uglRasterOpCopy, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpAnd", uglRasterOpAnd, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpOr", uglRasterOpOr, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpXor", uglRasterOpXor, 0, N_TEXT | N_EXT},
  {NULL, "_uglToggleDoubleBuffer", uglToggleDoubleBuffer, 0, N_TEXT | N_EXT},
  {NULL, "_uglToggleFillPattern", uglToggleFillPattern, 0, N_TEXT | N_EXT},
  {NULL, "_uglSetLineWidth", uglSetLineWidth, 0, N_TEXT | N_EXT},
  {NULL, "_uglSetLineStyle", uglSetLineStyle, 0, N_TEXT | N_EXT},
  {NULL, "_uglConvertTest", uglConvertTest, 0, N_TEXT | N_EXT},
  {NULL, "_uglFontList", uglFontList, 0, N_TEXT | N_EXT},
  {NULL, "_uglRectCreate", uglRectCreate, 0, N_TEXT | N_EXT}
};

    int i;

    gfxPartId = memSysPartId;
    uglBMFGlyphCachePoolId = memSysPartId;
    uglMemDefaultPoolSet(gfxPartId);
    createDib();
    gfxDevId = UGL_GRAPHICS_CREATE(0, 0, 0);
    if (gfxDevId == UGL_NULL) {
      printf("Unable to create graphics device.\n");
      return 1;
    }

    fntDrvId = UGL_FONT_DRIVER_CREATE (gfxDevId);
    if (fntDrvId == UGL_NULL) {
      printf("Unable to create font driver for graphics device.\n");
      return 1;
    }

    for (i = 0; i < NELEMENTS(symTableUglDemo); i++)
    {
        symTableAdd(sysSymTable, &symTableUglDemo[i]);
    }

    return 0;
}

