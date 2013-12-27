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

#include "vmxball.cbm"
#include "pinball.cbm"
#include "cursor.cbm"
#include "font8x16.cfs"

#define PAL_LENGTH             16
#define BALL_SPEED              4
#define DB_CLEAR_COLOR          0x06

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

PART_ID gfxPartId;
UGL_DEVICE_ID gfxDevId;
UGL_DIB bgDib, fgDib;
UGL_MDIB mDib;
UGL_CDIB cDib;
UGL_RASTER_OP rasterOp = UGL_RASTER_OP_COPY;
BOOL doubleBuffer = TRUE;
BOOL fillPattern = FALSE;
int animTreshold = 1;

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
  gfxMode.depth = 4;
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
  struct vgaHWRec oldRegs;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  for (i = 0; i < maxtimes; i++) {
    uglPixelSet(gfxDevId->defaultGc, rand() % 640, rand() % 480, rand () % 16);
    taskDelay(animTreshold);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglHLine4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  int i, y, x, x1, x2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

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

    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);
    uglForegroundColorSet (gfxDevId->defaultGc, rand () % 16);
    uglLine (gfxDevId->defaultGc, x1, y, x2, y);
    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    taskDelay(animTreshold);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglVLine4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  int i, x, y, y1, y2;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

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

    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);
    uglForegroundColorSet (gfxDevId->defaultGc, rand () % 16);
    uglLine (gfxDevId->defaultGc, x, y1, x, y2);
    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglLine4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
  struct vgaHWRec oldRegs;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  for (i = 0; i < maxtimes; i++) {
    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);
    uglForegroundColorSet (gfxDevId->defaultGc, rand () % 16);
    uglLine (gfxDevId->defaultGc, rand() % 640, rand() % 480,
             rand () % 640, rand() % 480);
    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglRect4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_DDB *pDbBmp;
  UGL_MDDB *pMddb;
  UGL_RECT dbSrcRect;
  UGL_POINT dbPt;
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

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  if (fillPattern == TRUE) {
    pMddb = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
                                0, gfxPartId);
    if (pMddb == UGL_NULL) {
      if (doubleBuffer == TRUE) {
        uglBitmapDestroy(gfxDevId, pDbBmp);
      }
      restoreConsole(&oldRegs);
      printf("Unable to create monochrome image\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  if (fillPattern == TRUE) {
    uglFillPatternSet (gfxDevId->defaultGc, pMddb);
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

    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);
    uglForegroundColorSet(gfxDevId->defaultGc, rand () % 16);
    uglBackgroundColorSet(gfxDevId->defaultGc, rand () % 16);
    uglRectangle(gfxDevId->defaultGc, x1, y1, x2, y2);
    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (fillPattern == TRUE) {
    uglFillPatternSet (gfxDevId->defaultGc, UGL_NULL);
  }

  if (fillPattern == TRUE) {
    uglMonoBitmapDestroy(gfxDevId, pMddb);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
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

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  if (fillPattern == TRUE) {
    pMddb = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
                                0, gfxPartId);
    if (pMddb == UGL_NULL) {
      if (doubleBuffer == TRUE) {
        uglBitmapDestroy(gfxDevId, pDbBmp);
      }
      restoreConsole(&oldRegs);
      printf("Unable to create monochrome image\n");
      return 1;
    }
  }

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  if (fillPattern == TRUE) {
    uglFillPatternSet (gfxDevId->defaultGc, pMddb);
  }

  for (i = 0; i < maxtimes; i++) {
    for (j = 0; j < nPoints; j++) {
      points[j * 2] = rand() % 640;
      points[j * 2 + 1] = rand () % 480;
    }

    points[nPoints * 2] = points[0];
    points[nPoints * 2 + 1] = points[1];

    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);
    uglForegroundColorSet(gfxDevId->defaultGc, rand () % 16);
    uglBackgroundColorSet(gfxDevId->defaultGc, rand () % 16);
    uglPolygon(gfxDevId->defaultGc, nPoints + 1, points);
    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    taskDelay(animTreshold);
  }

  if (fillPattern == TRUE) {
    uglFillPatternSet (gfxDevId->defaultGc, UGL_NULL);
  }

  if (fillPattern == TRUE) {
    uglMonoBitmapDestroy(gfxDevId, pMddb);
  }

  if (doubleBuffer == TRUE) {
    uglBitmapDestroy(gfxDevId, pDbBmp);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglBlt4Test(UGL_REGION_ID clipRegionId, int x1, int y1, int x2, int y2)
{
  struct vgaHWRec oldRegs;
  UGL_DDB *pDbBmp, *pFgBmp, *pSaveBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     gfxDevId->defaultGc->pDefaultBitmap,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
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
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
    }
    else {
      uglBitmapBlt(gfxDevId->defaultGc, UGL_DISPLAY_ID,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
    }

    /* Set raster operation and draw ball */
    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);

    uglBitmapBlt(gfxDevId->defaultGc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
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
  uglBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglMono4Test(UGL_REGION_ID clipRegionId, int x1, int y1, int x2, int y2)
{
  struct vgaHWRec oldRegs;
  UGL_MDDB *pMddb;
  UGL_DDB *pSaveBmp;
  UGL_RECT srcRect, saveRect;
  UGL_POINT pt, pt0;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  pMddb = uglMonoBitmapCreate(gfxDevId, &mDib, UGL_DIB_INIT_DATA,
			         0, gfxPartId);
  if (pMddb == UGL_NULL) {
    restoreConsole(&oldRegs);
    printf("Unable to create monochrome image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_VALUE,
                             0, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    uglMonoBitmapDestroy(gfxDevId, pMddb);
    restoreConsole(&oldRegs);
    printf("Unable to create background save image\n");
    return 1;
  }

  uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  uglForegroundColorSet(gfxDevId->defaultGc, 14);
  uglBackgroundColorSet(gfxDevId->defaultGc, 4);
  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  srcRect.left = x1;
  srcRect.top = y1;
  if (x2 == 0) {
    srcRect.right = pMddb->width;
  }
  else {
    srcRect.right = x2;
  }

  if (y2 == 0) {
    srcRect.bottom = pMddb->height;
  }
  else {
    srcRect.bottom = y2;
  }

  pt.x = -pMddb->width;
  pt.y = -pMddb->height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pMddb->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pMddb->height;
  pt0.x = pt.x;
  pt0.y = pt.y;

  while (pt.y < 480) {
    uglBitmapBlt(gfxDevId->defaultGc, UGL_DISPLAY_ID,
                 saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                 pSaveBmp, pt0.x, pt0.y);

    uglBitmapBlt(gfxDevId->defaultGc, pMddb,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    /* Delay */
    taskDelay(animTreshold);

    uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DISPLAY_ID, pt.x, pt.y);

    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  uglMonoBitmapDestroy(gfxDevId, pMddb);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglTrans4Test(UGL_REGION_ID clipRegionId)
{
  struct vgaHWRec oldRegs;
  UGL_DDB *pDbBmp, *pSaveBmp;
  UGL_TDDB *pFgBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);
  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, UGL_NULL);
  }

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     gfxDevId->defaultGc->pDefaultBitmap,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
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
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
    }
    else {
      uglBitmapBlt(gfxDevId->defaultGc, UGL_DISPLAY_ID,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
    }

    /* Set raster operation and draw ball */
    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);

    uglBitmapBlt(gfxDevId->defaultGc, pFgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DEFAULT_ID, pt.x, pt.y);

    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
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
  uglTransBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglCursor4Test(UGL_REGION_ID clipRegionId)
{
  struct vgaHWRec oldRegs;
  UGL_CDDB *pFgBmp;
  UGL_GENERIC_DRIVER *pDrv;
  UGL_GEN_CURSOR_DATA *pCursorData;

  if (mode4Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (uglCursorInit (gfxDevId, cDib.width, cDib.height, 320, 240)
                     != UGL_STATUS_OK) {
      restoreConsole(&oldRegs);
      printf("Unable to initialize cursor.\n");
    return 1;
  }

  pDrv = (UGL_GENERIC_DRIVER *) gfxDevId;
  pCursorData = pDrv->pCursorData;

  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  if (uglBitmapWrite(gfxDevId, &bgDib, 0, 0, bgDib.width, bgDib.height,
                     UGL_DISPLAY_ID,
                     640 / 2 - bgDib.width / 2,
                     480 / 2 - bgDib.height / 2) != UGL_STATUS_OK) {
    restoreConsole(&oldRegs);
    printf("Unable to write background image\n");
    return 1;
  }

  pFgBmp = uglCursorBitmapCreate(gfxDevId, &cDib);
  if (pFgBmp == UGL_NULL) {
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  if (uglCursorImageSet(gfxDevId, pFgBmp) != UGL_STATUS_OK) {
    uglCursorBitmapDestroy(gfxDevId, pFgBmp);
    restoreConsole(&oldRegs);
    printf("Unable to set cursor image\n");
    return 1;
  }

  while (pCursorData->position.y < 480) {

    uglCursorOn(gfxDevId);

    /* Delay */
    taskDelay(animTreshold);

    if (uglCursorOff(gfxDevId) != UGL_STATUS_OK) {
        break;
    }

    pCursorData->position.x += BALL_SPEED;
    pCursorData->position.y += BALL_SPEED;
  }

  uglCursorBitmapDestroy(gfxDevId, pFgBmp);
  uglCursorDeinit(gfxDevId);
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
  gfxMode.depth = 8;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    return 1;
  }

  setPalette();

  return 0;
}

int uglPixel8Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  struct vgaHWRec oldRegs;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);

  for (i = 0; i < maxtimes; i++) {
    uglPixelSet(gfxDevId->defaultGc, rand() % 320, rand() % 200, rand () % 256);
    taskDelay(animTreshold);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglBlt8Test(void)
{
  int i, j;
  struct vgaHWRec oldRegs;
  UGL_DDB *pDbBmp, *pFgBmp, *pSaveBmp;
  UGL_RECT srcRect, saveRect, dbSrcRect;
  UGL_POINT pt, pt0, dbPt;

  if (mode8Enter(&oldRegs)) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             DB_CLEAR_COLOR, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer image\n");
      return 1;
    }
  }

  pFgBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_DATA,
                            8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create foreground image\n");
    return 1;
  }

  pSaveBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_DATA,
                             8, gfxPartId);
  if (pSaveBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
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

  if (doubleBuffer == TRUE) {
    uglDefaultBitmapSet(gfxDevId->defaultGc, pDbBmp);
  }
  else {
    uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  }

  /* Draw background */
  for (j = 0; j < 200; j++) {
    for(i = 0; i < 320; i++) {
      uglPixelSet(gfxDevId->defaultGc, i, j, (i + j) % 255);
    }
  }

  srcRect.left = 0;
  srcRect.right = fgDib.width;
  srcRect.top = 0;
  srcRect.bottom = fgDib.height;
  pt.x = -fgDib.width;
  pt.y = -fgDib.height;

  saveRect.left = pt.x;
  saveRect.right = pt.x + fgDib.width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + fgDib.height;
  pt0.x = 0;
  pt0.y = 0;

  while(pt.y < 200) {

    /* Copy background */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
    }
    else {
      uglBitmapBlt(gfxDevId->defaultGc, UGL_DISPLAY_ID,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
    }

    /* Set raster operation and draw ball */
    uglRasterModeSet(gfxDevId->defaultGc, rasterOp);

    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pFgBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   pDbBmp, pt.x, pt.y);
    }
    else {
      uglBitmapBlt(gfxDevId->defaultGc, pFgBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   UGL_DISPLAY_ID, pt.x, pt.y);
    }

    uglRasterModeSet(gfxDevId->defaultGc, UGL_RASTER_OP_COPY);

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Erase ball */
    if (doubleBuffer == TRUE) {
      uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   pDbBmp, pt.x, pt.y);
    }
    else {
      uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   UGL_DISPLAY_ID, pt.x, pt.y);
    }

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
  uglBitmapDestroy(gfxDevId, pFgBmp);
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

void uglDemoInit()
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
  {NULL, "_uglPixel8Test", uglPixel8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglBlt8Test", uglBlt8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglSetAnimTreshold", uglSetAnimTreshold, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpCopy", uglRasterOpCopy, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpAnd", uglRasterOpAnd, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpOr", uglRasterOpOr, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpXor", uglRasterOpXor, 0, N_TEXT | N_EXT},
  {NULL, "_uglToggleDoubleBuffer", uglToggleDoubleBuffer, 0, N_TEXT | N_EXT},
  {NULL, "_uglToggleFillPattern", uglToggleFillPattern, 0, N_TEXT | N_EXT},
  {NULL, "_uglConvertTest", uglConvertTest, 0, N_TEXT | N_EXT},
  {NULL, "_uglRectCreate", uglRectCreate, 0, N_TEXT | N_EXT}
};

    int i;

    gfxPartId = memSysPartId;
    uglMemDefaultPoolSet(gfxPartId);
    createDib();
    gfxDevId = UGL_GRAPHICS_CREATE(0, 0, 0);

    for (i = 0; i < NELEMENTS(symTableUglDemo); i++)
    {
        symTableAdd(sysSymTable, &symTableUglDemo[i]);
    }
}

