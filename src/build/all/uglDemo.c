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

#include "vmxball.cbm"
#include "pinball.cbm"
#include "font8x16.cfs"

#define LOOP_PAL_LENGTH         16
#define BALL_SPEED 4

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

PART_ID gfxPartId;
UGL_DEVICE_ID gfxDevId;
UGL_DIB bgDib, fgDib;
UGL_MDIB mDib;
UGL_RASTER_OP rasterOp = UGL_RASTER_OP_COPY;
BOOL doubleBuffer = TRUE;
int animTreshold = 1;

int createDib(void)
{
  int i, j;

  bgDib.width = vmxballWidth;
  bgDib.height = vmxballHeight;
  bgDib.stride = vmxballWidth;
  bgDib.imageFormat = UGL_INDEXED_8;
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
  fgDib.imageFormat = UGL_INDEXED_8;
  fgDib.colorFormat = UGL_DEVICE_COLOR_32;
  fgDib.clutSize = 16;
  fgDib.pClut = malloc(sizeof(UGL_COLOR) * 16);
  if (fgDib.pClut == NULL) {
    printf("Error creating clut\n");
    return 1;
  }
  for (i = 0; i < 16; i++)
    ((UGL_COLOR *) fgDib.pClut)[i] = UGL_MAKE_ARGB(1,
		    		     16 * pinballClut[i][0] / 255,
		    		     16 * pinballClut[i][1] / 255,
		    		     16 * pinballClut[i][2] / 255);
  fgDib.pData = pinballData;

  mDib.width = pinballWidth;
  mDib.height = pinballHeight;
  mDib.stride = pinballWidth;
  mDib.pData = pinballMask;

  return 0;
}

void setPalette(void)
{
  int i;
  UGL_ORD paletteIndex[LOOP_PAL_LENGTH];
  UGL_ARGB palette[LOOP_PAL_LENGTH];
  UGL_COLOR colors[LOOP_PAL_LENGTH];

  for (i = 0; i < LOOP_PAL_LENGTH; i++) {
    paletteIndex[i] = i;
    palette[i] = UGL_MAKE_ARGB(255,			/* ALPHA */
			       vmxballClut[i][0],	/* RED */
			       vmxballClut[i][1],	/* GREEN */
			       vmxballClut[i][2]);	/* BLUE */
  }
  uglColorAlloc(gfxDevId, palette, paletteIndex, colors, LOOP_PAL_LENGTH);
}

void restoreConsole(struct vgaHWRec *regs)
{
  vgaRestore(regs, FALSE);
  vgaLoadFont(font8x16, font8x16Height);
  printf("%c\n", 0x0c);
}

int uglPixel4Test(int maxtimes, UGL_REGION_ID clipRegionId)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  vgaSave(&oldRegs);

  /* Enter video mode */
  gfxMode.width = 640;
  gfxMode.height = 480;
  gfxMode.depth = 4;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);
  if (clipRegionId != NULL) {
    uglClipRegionSet (gfxDevId->defaultGc, clipRegionId);
  }

  for (i = 0; i < maxtimes; i++) {
    uglPixelSet(gfxDevId->defaultGc, rand() % 640, rand() % 480, rand () % 16);
    taskDelay(animTreshold);
  }

  restoreConsole(&oldRegs);

  return 0;
}

int uglBlt4Test(void)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  UGL_DDB *pDbBmp, *pBgBmp, *pFgBmp, *pSaveBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  vgaSave(&oldRegs);

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  /* Enter video mode */
  gfxMode.width = 640;
  gfxMode.height = 480;
  gfxMode.depth = 4;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             14, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  pBgBmp = uglBitmapCreate(gfxDevId, &bgDib, UGL_DIB_INIT_DATA,
                           8, gfxPartId);
  if (pBgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create background image\n");
    return 1;
  }

  pFgBmp = uglBitmapCreate(gfxDevId, &fgDib, UGL_DIB_INIT_DATA,
                           8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglBitmapDestroy(gfxDevId, pBgBmp);
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
    uglBitmapDestroy(gfxDevId, pBgBmp);
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

  srcRect.left = 0;
  srcRect.right = pBgBmp->width;
  srcRect.top = 0;
  srcRect.bottom = pBgBmp->height;
  pt.x = 640 / 2 - pBgBmp->width / 2;
  pt.y = 480 / 2 - pBgBmp->height / 2;
  if (doubleBuffer == TRUE) {
    uglBitmapBlt(gfxDevId->defaultGc, pBgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 pDbBmp, pt.x, pt.y);
  }
  else {
    uglBitmapBlt(gfxDevId->defaultGc, pBgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DISPLAY_ID, pt.x, pt.y);
  }

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
  uglBitmapDestroy(gfxDevId, pBgBmp);
  uglBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglMono4Test(void)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  UGL_MDDB *pMddb;
  UGL_DDB *pSaveBmp;
  UGL_RECT srcRect, saveRect;
  UGL_POINT pt, pt0;

  vgaSave(&oldRegs);

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  /* Enter video mode */
  gfxMode.width = 640;
  gfxMode.height = 480;
  gfxMode.depth = 4;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
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

  srcRect.left = 0;
  srcRect.right = pMddb->width;
  srcRect.top = 0;
  srcRect.bottom = pMddb->height;
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
                 UGL_DISPLAY_ID, pt.x, pt.y);

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

int uglTrans4Test(void)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  UGL_DDB *pDbBmp, *pBgBmp, *pSaveBmp;
  UGL_TDDB *pFgBmp;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  vgaSave(&oldRegs);

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  /* Enter video mode */
  gfxMode.width = 640;
  gfxMode.height = 480;
  gfxMode.depth = 4;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             14, gfxPartId);
    if (pDbBmp == UGL_NULL) {
      restoreConsole(&oldRegs);
      printf("Unable to create double buffer\n");
      return 1;
    }
  }

  pBgBmp = uglBitmapCreate(gfxDevId, &bgDib, UGL_DIB_INIT_DATA,
                           8, gfxPartId);
  if (pBgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    restoreConsole(&oldRegs);
    printf("Unable to create background image\n");
    return 1;
  }

  pFgBmp = uglTransBitmapCreate(gfxDevId, &fgDib, &mDib, UGL_DIB_INIT_DATA,
                                8, gfxPartId);

  if (pFgBmp == UGL_NULL) {
    if (doubleBuffer == TRUE) {
      uglBitmapDestroy(gfxDevId, pDbBmp);
    }
    uglBitmapDestroy(gfxDevId, pBgBmp);
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
    uglBitmapDestroy(gfxDevId, pBgBmp);
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
  srcRect.right = pBgBmp->width;
  srcRect.top = 0;
  srcRect.bottom = pBgBmp->height;
  pt.x = 640 / 2 - pBgBmp->width / 2;
  pt.y = 480 / 2- pBgBmp->height / 2;
  if (doubleBuffer == TRUE) {
    uglBitmapBlt(gfxDevId->defaultGc, pBgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 pDbBmp, pt.x, pt.y);
  }
  else {
    uglBitmapBlt(gfxDevId->defaultGc, pBgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DISPLAY_ID, pt.x, pt.y);
  }

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
  uglBitmapDestroy(gfxDevId, pBgBmp);
  uglTransBitmapDestroy(gfxDevId, pFgBmp);
  uglBitmapDestroy(gfxDevId, pSaveBmp);
  restoreConsole(&oldRegs);

  return 0;
}

int uglPixel8Test(int maxtimes)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  int i;

  if (maxtimes <= 0) {
    maxtimes = 1000;
  }

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  vgaSave(&oldRegs);

  /* Enter video mode */
  gfxMode.width = 320;
  gfxMode.height = 200;
  gfxMode.depth = 8;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  uglDefaultBitmapSet(gfxDevId->defaultGc, NULL);

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
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  UGL_DDB *pDbBmp, *pFgBmp, *pSaveBmp;
  UGL_RECT srcRect, saveRect, dbSrcRect;
  UGL_POINT pt, pt0, dbPt;

  vgaSave(&oldRegs);

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  /* Enter video mode */
  gfxMode.width = 320;
  gfxMode.height = 200;
  gfxMode.depth = 8;
  gfxMode.refreshRate = 60;
  gfxMode.flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    restoreConsole(&oldRegs);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  setPalette();

  if (doubleBuffer == TRUE) {
    pDbBmp = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                             14, gfxPartId);
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
  {NULL, "_uglBlt4Test", uglBlt4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglMono4Test", uglMono4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglTrans4Test", uglTrans4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglPixel8Test", uglPixel8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglBlt8Test", uglBlt8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglSetAnimTreshold", uglSetAnimTreshold, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpCopy", uglRasterOpCopy, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpAnd", uglRasterOpAnd, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpOr", uglRasterOpOr, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpXor", uglRasterOpXor, 0, N_TEXT | N_EXT},
  {NULL, "_uglToggleDoubleBuffer", uglToggleDoubleBuffer, 0, N_TEXT | N_EXT},
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

