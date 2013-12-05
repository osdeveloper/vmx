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

#define OLD_BLT
#define LOOP_PAL_LENGTH         16
#define BALL_SPEED 4

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

PART_ID gfxPartId;
UGL_DEVICE_ID gfxDevId;
UGL_DIB *pBgDib, *pFgDib;
UGL_RASTER_OP rasterOp = UGL_RASTER_OP_COPY;
BOOL doubleBuffer = TRUE;
int animTreshold = 0;
BOOL firstTime = TRUE;
BOOL firstTimel = TRUE;
UGL_DDB *pDbBmp, *pBgBmp, *pFgBmp, *pSaveBmp;
UGL_DDB *pFglBmp, *pSavelBmp, *pDblBmp;

int createDib(void)
{
  int i, j;
  UGL_UINT8 *ptr;

  pBgDib = malloc(sizeof(UGL_DIB));
  pFgDib = malloc(sizeof(UGL_DIB));
  if (pBgDib == NULL || pFgDib == NULL) {
    printf("Error creating dib\n");
    return 1;
  }

  pBgDib->width = vmxballWidth;
  pBgDib->height = vmxballHeight;
  pBgDib->stride = vmxballWidth;
  pBgDib->imageFormat = UGL_INDEXED_8;
  pBgDib->colorFormat = UGL_DEVICE_COLOR_32;
  pBgDib->clutSize = 16;
  pBgDib->pClut = malloc(sizeof(UGL_COLOR) * 16);
  if (pBgDib->pClut == NULL) {
    printf("Error creating clut\n");
    return 1;
  }
  for (i = 0; i < 16; i++)
    ((UGL_COLOR *) pBgDib->pClut)[i] = UGL_MAKE_ARGB(1,
		    		     16 * vmxballClut[i][0] / 255,
		    		     16 * vmxballClut[i][1] / 255,
		    		     16 * vmxballClut[i][2] / 255);
  pBgDib->pData = vmxballData;

  pFgDib->width = pinballWidth;
  pFgDib->height = pinballHeight;
  pFgDib->stride = pinballWidth;
  pFgDib->imageFormat = UGL_INDEXED_8;
  pFgDib->colorFormat = UGL_DEVICE_COLOR_32;
  pFgDib->clutSize = 16;
  pFgDib->pClut = malloc(sizeof(UGL_COLOR) * 16);
  if (pFgDib->pClut == NULL) {
    printf("Error creating clut\n");
    return 1;
  }
  for (i = 0; i < 16; i++)
    ((UGL_COLOR *) pFgDib->pClut)[i] = UGL_MAKE_ARGB(1,
		    		     16 * pinballClut[i][0] / 255,
		    		     16 * pinballClut[i][1] / 255,
		    		     16 * pinballClut[i][2] / 255);
  pFgDib->pData = pinballData;

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

int new4BitImg(UGL_DDB_ID *pDbBmpId,
	       UGL_DDB_ID *pBgBmpId,
	       UGL_DDB_ID *pFgBmpId,
	       UGL_DDB_ID *pSaveBmpId)
{
  *pDbBmpId = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
			 14, gfxPartId);
  if (*pDbBmpId == UGL_NULL)
    return 1;

  *pBgBmpId = uglBitmapCreate(gfxDevId, pBgDib, UGL_DIB_INIT_DATA,
			 8, gfxPartId);
  if (*pBgBmpId == UGL_NULL) {
    uglBitmapDestroy(gfxDevId, *pDbBmpId, gfxPartId);
    return 1;
  }

  *pFgBmpId = uglBitmapCreate(gfxDevId, pFgDib, UGL_DIB_INIT_DATA,
			 8, gfxPartId);

  if (*pFgBmpId == UGL_NULL) {
    uglBitmapDestroy(gfxDevId, *pDbBmpId, gfxPartId);
    uglBitmapDestroy(gfxDevId, *pBgBmpId, gfxPartId);
    return 1;
  }

  *pSaveBmpId = uglBitmapCreate(gfxDevId, pFgDib, UGL_DIB_INIT_DATA,
		  		8, gfxPartId);
  if (*pSaveBmpId == UGL_NULL) {
    uglBitmapDestroy(gfxDevId, *pDbBmpId, gfxPartId);
    uglBitmapDestroy(gfxDevId, *pBgBmpId, gfxPartId);
    uglBitmapDestroy(gfxDevId, *pFgBmpId, gfxPartId);
    return 1;
  }

  return 0;
}

int new8BitImg(UGL_DDB_ID *pDbBmpId,
               UGL_DDB_ID *pFgBmpId,
               UGL_DDB_ID *pSaveBmpId)
{
  *pDbBmpId = uglBitmapCreate(gfxDevId, UGL_NULL, UGL_DIB_INIT_VALUE,
                         14, gfxPartId);
  if (*pDbBmpId == UGL_NULL)
    return 1;

  *pFgBmpId = uglBitmapCreate(gfxDevId, pFgDib, UGL_DIB_INIT_DATA,
                         8, gfxPartId);

  if (*pFgBmpId == UGL_NULL) {
    uglBitmapDestroy(gfxDevId, *pDbBmpId, gfxPartId);
    return 1;
  }

  *pSaveBmpId = uglBitmapCreate(gfxDevId, pFgDib, UGL_DIB_INIT_DATA,
                                8, gfxPartId);
  if (*pSaveBmpId == UGL_NULL) {
    uglBitmapDestroy(gfxDevId, *pDbBmpId, gfxPartId);
    uglBitmapDestroy(gfxDevId, *pFgBmpId, gfxPartId);
    return 1;
  }

  return 0;
}

int uglPixel4Test(void)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  int i, j;

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  vgaSave(&oldRegs);

  /* Enter video mode */
  gfxMode.Width = 640;
  gfxMode.Height = 480;
  gfxMode.Depth = 4;
  gfxMode.RefreshRate = 60;
  gfxMode.Flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    vgaRestore(&oldRegs, FALSE);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  gfxDevId->defaultGc->pDefaultBitmap = UGL_DISPLAY_ID;
  UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);

  for (j = 0; j < 16; j++) {
    for (i = 0; i < 80; i++) {
      uglPixelSet(gfxDevId->defaultGc, i, j, j);
    }
  }

  taskDelay(animTreshold);

  vgaRestore(&oldRegs, FALSE);
  vgaLoadFont(font8x16, font8x16Height);

  return 0;
}

int uglBlt4Test(void)
{
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  UGL_RECT dbSrcRect, srcRect, saveRect;
  UGL_POINT dbPt, pt, pt0;

  vgaSave(&oldRegs);

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  /* Enter video mode */
  gfxMode.Width = 640;
  gfxMode.Height = 480;
  gfxMode.Depth = 4;
  gfxMode.RefreshRate = 60;
  gfxMode.Flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    vgaRestore(&oldRegs, FALSE);
    printf("Unable to set graphics mode to 640x480 @60Hz, 16 color.\n");
    return 1;
  }

  /* Set palette */
  setPalette();

  if (firstTime == TRUE) {

    if (new4BitImg(&pDbBmp, &pBgBmp, &pFgBmp, &pSaveBmp) != 0) {
      vgaRestore(&oldRegs, FALSE);
      printf("Error initializing images.\n");
      return 1;
    }

    firstTime = FALSE;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 640;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 480;
  dbPt.x = 0;
  dbPt.y = 0;

  /* Blit image */
  gfxDevId->defaultGc->pDefaultBitmap = UGL_DISPLAY_ID;

  srcRect.left = 0;
  srcRect.right = pBgBmp->width;
  srcRect.top = 0;
  srcRect.bottom = pBgBmp->height;
  pt.x = 640 / 2 - pBgBmp->width / 2;
  pt.y = 480 / 2- pBgBmp->height / 2;
  if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
    (*gfxDevId->bitmapBlt)(gfxDevId, pBgBmp, &srcRect, pDbBmp, &pt);
#else
    uglBitmapBlt(gfxDevId->defaultGc, pBgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 pDbBmp, pt.x, pt.y);
#endif
  }
  else {
#ifdef OLD_BLT
    (*gfxDevId->bitmapBlt)(gfxDevId, pBgBmp, &srcRect, UGL_DISPLAY_ID, &pt);
#else
    uglBitmapBlt(gfxDevId->defaultGc, pBgBmp,
                 srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                 UGL_DISPLAY_ID, pt.x, pt.y);
#endif
  }

  srcRect.left = 0;
  srcRect.right = pFgBmp->width;
  srcRect.top = 0;
  srcRect.bottom = pFgBmp->height;
  pt.x = 0;
  pt.y = 0;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgBmp->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgBmp->height;
  pt0.x = 0;
  pt0.y = 0;

  while (pt.y < 480 - pFgBmp->height) {

    /* Copy background */
    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pDbBmp, &saveRect, pSaveBmp, &pt0);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
#endif
    }
    else {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, UGL_DISPLAY_ID, &saveRect, pSaveBmp, &pt0);
#else
      uglBitmapBlt(gfxDevId->defaultGc, UGL_DISPLAY_ID,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
#endif
    }

    /* Set raster operation and draw ball */
    if (rasterOp != UGL_RASTER_OP_COPY) {
      gfxDevId->defaultGc->rasterOp = rasterOp;
      UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);
    }

    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pFgBmp, &srcRect, pDbBmp, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pFgBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   pDbBmp, pt.x, pt.y);
#endif
    }
    else {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pFgBmp, &srcRect, UGL_DISPLAY_ID, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pFgBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   UGL_DISPLAY_ID, pt.x, pt.y);
#endif
    }

    if (rasterOp != UGL_RASTER_OP_COPY) {
      gfxDevId->defaultGc->rasterOp = UGL_RASTER_OP_COPY;
      UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);
    }

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pDbBmp, &dbSrcRect, UGL_DISPLAY_ID, &dbPt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pDbBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
#endif
    }

    /* Erase ball */
    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pSaveBmp, &srcRect, pDbBmp, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   pDbBmp, pt.x, pt.y);
#endif
    }
    else {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pSaveBmp, &srcRect, UGL_DISPLAY_ID, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   UGL_DISPLAY_ID, pt.x, pt.y);
#endif
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  vgaRestore(&oldRegs, FALSE);
  vgaLoadFont(font8x16, font8x16Height);

  return 0;
}

int uglBlt8Test(void)
{
  int i, j;
  UGL_MODE gfxMode;
  struct vgaHWRec oldRegs;
  UGL_RECT srcRect, saveRect, dbSrcRect;
  UGL_POINT pt, pt0, dbPt;

  vgaSave(&oldRegs);

  if (gfxDevId == UGL_NULL) {
    printf("No compatible graphics device found.\n");
    return 1;
  }

  /* Enter video mode */
  gfxMode.Width = 320;
  gfxMode.Height = 200;
  gfxMode.Depth = 8;
  gfxMode.RefreshRate = 60;
  gfxMode.Flags = UGL_MODE_INDEXED_COLOR;

  if (uglModeSet(gfxDevId, &gfxMode) != UGL_STATUS_OK) {
    vgaRestore(&oldRegs, FALSE);
    printf("Unable to set graphics mode to 320x200 @60Hz, 256 color.\n");
    return 1;
  }

  setPalette();

  if (firstTimel == TRUE) {

    if (new8BitImg(&pDblBmp, &pFglBmp, &pSavelBmp) != 0) {
      vgaRestore(&oldRegs, FALSE);
      printf("Error initializing images.\n");
      return 1;
    }

    firstTimel = FALSE;
  }

  dbSrcRect.left = 0;
  dbSrcRect.right = 320;
  dbSrcRect.top = 0;
  dbSrcRect.bottom = 200;
  dbPt.x = 0;
  dbPt.y = 0;

  if (doubleBuffer == TRUE) {
    gfxDevId->defaultGc->pDefaultBitmap = pDblBmp;
    UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);
  }

  /* Draw background */
  for (j = 0; j < 200; j++) {
    for(i = 0; i < 320; i++) {
      uglPixelSet(gfxDevId->defaultGc, i, j, (i + j) % 255);
    }
  }

  if (doubleBuffer == TRUE) {
    gfxDevId->defaultGc->pDefaultBitmap = UGL_DISPLAY_ID;
    UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);
  }

  srcRect.left = 0;
  srcRect.right = pFgDib->width;
  srcRect.top = 0;
  srcRect.bottom = pFgDib->height;
  pt.x = 0;
  pt.y = 0;

  saveRect.left = pt.x;
  saveRect.right = pt.x + pFgDib->width;
  saveRect.top = pt.y;
  saveRect.bottom = pt.y + pFgDib->height;
  pt0.x = 0;
  pt0.y = 0;

  while(pt.y < 200 - pFgDib->height) {

    /* Copy background */
    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pDblBmp, &saveRect, pSavelBmp, &pt0);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pDblBmp,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
#endif
    }
    else {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, UGL_DISPLAY_ID, &saveRect, pSavelBmp, &pt0);
#else
      uglBitmapBlt(gfxDevId->defaultGc, UGL_DISPLAY_ID,
                   saveRect.left, saveRect.top, saveRect.right, saveRect.bottom,
                   pSaveBmp, pt0.x, pt0.y);
#endif
    }

    /* Set raster operation and draw ball */
    if (rasterOp != UGL_RASTER_OP_COPY) {
      gfxDevId->defaultGc->rasterOp = rasterOp;
      UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);
    }

    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pFglBmp, &srcRect, pDblBmp, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pFglBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   pDblBmp, pt.x, pt.y);
#endif
    }
    else {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pFglBmp, &srcRect, UGL_DISPLAY_ID, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pFglBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   UGL_DISPLAY_ID, pt.x, pt.y);
#endif
    }

    if (rasterOp != UGL_RASTER_OP_COPY) {
      gfxDevId->defaultGc->rasterOp = UGL_RASTER_OP_COPY;
      UGL_GC_SET(gfxDevId, gfxDevId->defaultGc);
    }

    /* Draw double buffer on screen */
    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pDblBmp, &dbSrcRect, UGL_DISPLAY_ID, &dbPt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pDblBmp,
                   dbSrcRect.left, dbSrcRect.top,
                   dbSrcRect.right, dbSrcRect.bottom,
                   UGL_DISPLAY_ID, dbPt.x, dbPt.y);
#endif
    }

    /* Erase ball */
    if (doubleBuffer == TRUE) {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pSavelBmp, &srcRect, pDblBmp, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   pDblBmp, pt.x, pt.y);
#endif
    }
    else {
#ifdef OLD_BLT
      (*gfxDevId->bitmapBlt)(gfxDevId, pSavelBmp, &srcRect, UGL_DISPLAY_ID, &pt);
#else
      uglBitmapBlt(gfxDevId->defaultGc, pSaveBmp,
                   srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                   UGL_DISPLAY_ID, pt.x, pt.y);
#endif
    }

    /* Delay */
    taskDelay(animTreshold);

    /* Move ball */
    pt.x += BALL_SPEED;
    pt.y += BALL_SPEED;
    saveRect.left += BALL_SPEED;
    saveRect.right += BALL_SPEED;
    saveRect.top += BALL_SPEED;
    saveRect.bottom += BALL_SPEED;
  }

  vgaRestore(&oldRegs, FALSE);
  vgaLoadFont(font8x16, font8x16Height);

  return 0;
}

int uglDestroyTest(void)
{
  if (firstTime == FALSE) {
    printf("Freeing up resources for hires gfx test:\n");

    printf("Freeing double buffer image @0x%d...", pDbBmp);
    uglBitmapDestroy(gfxDevId, pDbBmp, gfxPartId);
    printf("done.\n");

    printf("Freeing background image @0x%d...", pBgBmp);
    uglBitmapDestroy(gfxDevId, pBgBmp, gfxPartId);
    printf("done.\n");

    printf("Freeing foreground image @0x%d...", pFgBmp);
    uglBitmapDestroy(gfxDevId, pFgBmp, gfxPartId);
    printf("done.\n");

    printf("Freeing background save image @0x%d...", pSaveBmp);
    uglBitmapDestroy(gfxDevId, pSaveBmp, gfxPartId);
    printf("done.\n");

    firstTime = TRUE;
  }
  else
    printf("Hires gfx test not run yet!\n");


  if (firstTimel == FALSE) {
    printf("Freeing up resources for lores linear gfx test:\n");

    printf("Freeing double buffer image @0x%d...", pDbBmp);
    uglBitmapDestroy(gfxDevId, pDblBmp, gfxPartId);
    printf("done.\n");

    printf("Freeing foreground image @0x%d...", pFgBmp);
    uglBitmapDestroy(gfxDevId, pFglBmp, gfxPartId);
    printf("done.\n");

    printf("Freeing background save image @0x%d...", pSaveBmp);
    uglBitmapDestroy(gfxDevId, pSavelBmp, gfxPartId);
    printf("done.\n");

    firstTimel = TRUE;
  }
  else
    printf("Lores linear gfx test not run yet!\n");
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
    printf("input[%d] = 0x%x\n", i, ((UGL_COLOR *) pBgDib->pClut)[i]);
  /*
  (*gfxDevId->colorConvert)(gfxDevId, pBgDib->pClut, UGL_DEVICE_COLOR,
		            colorArray, UGL_DEVICE_COLOR_32, 16);
  */
  for (i = 0; i < 16; i++)
    printf("output[%d] = 0x%x\n", i, colorArray[i]);
}

void uglDemoInit()
{
static SYMBOL symTableUglDemo[] = {
  {NULL, "_uglPixel4Test", uglPixel4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglBlt4Test", uglBlt4Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglBlt8Test", uglBlt8Test, 0, N_TEXT | N_EXT},
  {NULL, "_uglDestroyTest", uglDestroyTest, 0, N_TEXT | N_EXT},
  {NULL, "_uglSetAnimTreshold", uglSetAnimTreshold, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpCopy", uglRasterOpCopy, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpAnd", uglRasterOpAnd, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpOr", uglRasterOpOr, 0, N_TEXT | N_EXT},
  {NULL, "_uglRasterOpXor", uglRasterOpXor, 0, N_TEXT | N_EXT},
  {NULL, "_uglToggleDoubleBuffer", uglToggleDoubleBuffer, 0, N_TEXT | N_EXT},
  {NULL, "_uglConvertTest", uglConvertTest, 0, N_TEXT | N_EXT}
};

    int i;

    gfxPartId = memSysPartId;
    createDib();
    gfxDevId = UGL_GRAPHICS_CREATE(0, 0, 0);

    for (i = 0; i < NELEMENTS(symTableUglDemo); i++)
    {
        symTableAdd(sysSymTable, &symTableUglDemo[i]);
    }
}

