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

/* udgencrs.c - Universal graphics library generic cursor support */

#include <stdlib.h>
#include <string.h>
#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/* Imports */

UGL_STATUS uglGenericScratchBitmapCreate (
    UGL_DEVICE_ID  devId,
    UGL_SIZE       width,
    UGL_SIZE       height
    );

/******************************************************************************
 *
 * uglGenericCursorInit - Initialize generic cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorInit (
    UGL_DEVICE_ID  devId,
    UGL_SIZE       maxWidth,
    UGL_SIZE       maxHeight,
    UGL_POS        xPosition,
    UGL_POS        yPosition
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;
    UGL_DIB               dib;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Allocate cursor data */
    pCursorData =
        (UGL_GEN_CURSOR_DATA *) UGL_CALLOC (1, sizeof (UGL_GEN_CURSOR_DATA));
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Create graphics context */
    pCursorData->gc = uglGcCreate (devId);
    if (pCursorData->gc == UGL_NULL) {
        UGL_FREE (pCursorData);
        return (UGL_STATUS_ERROR);
    }

    /* Initialize data structure */
    pCursorData->maxWidth   = maxWidth;
    pCursorData->maxHeight  = maxHeight;
    pCursorData->position.x = xPosition;
    pCursorData->position.y = yPosition;
    pCursorData->on         = UGL_FALSE;
    pCursorData->hidden     = UGL_FALSE;

    /* Create screen bitmap */
    dib.width       = maxWidth;
    dib.height      = maxHeight;
    dib.stride      = maxWidth;
    dib.clutSize    = 0;
    dib.colorFormat = UGL_DEVICE_COLOR_32;
    dib.imageFormat = UGL_DIRECT;

    pCursorData->screenBitmap = (*devId->bitmapCreate) (devId, &dib,
                                                        UGL_DIB_INIT_NONE, 0,
                                                        UGL_NULL);
    if (pCursorData->screenBitmap == UGL_NULL) {
        UGL_FREE (pCursorData);
        return (UGL_STATUS_ERROR);
    }

    /* Create scratch bitmap */
    pCursorData->scratchBitmap = (*devId->bitmapCreate) (devId, &dib,
                                                         UGL_DIB_INIT_NONE, 0,
                                                         UGL_NULL);
    if (pCursorData->scratchBitmap == UGL_NULL) {
        (*devId->bitmapDestroy) (devId, pCursorData->screenBitmap);
        UGL_FREE (pCursorData);
        return (UGL_STATUS_ERROR);
    }

    /* Set driver cursor data */
    pDrv->pCursorData = pCursorData;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorDeinit - Deinitialize generic cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorDeinit (
    UGL_DEVICE_ID  devId
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Free resources */
    (*devId->bitmapDestroy) (devId, pCursorData->screenBitmap);
    (*devId->bitmapDestroy) (devId, pCursorData->scratchBitmap);
    uglGcDestroy (pCursorData->gc);
    UGL_FREE (pCursorData);

    /* Mark cursor data as deallocated */
    pDrv->pCursorData = UGL_NULL;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorBitmapCreate - Create generic cursor bitmap
 *
 * RETURNS: UGL_CDDB_ID or UGL_NULL
 */

UGL_CDDB_ID uglGenericCursorBitmapCreate (
    UGL_DEVICE_ID  devId,
    UGL_CDIB *     pCdib
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;
    UGL_GEN_CDDB *        pCddb;
    UGL_DIB               dib;
    UGL_MDIB *            pMdib;
    UGL_INT32             x;
    UGL_INT32             y;
    UGL_INT32             maskIndex;
    UGL_UINT8 *           pSrc;
    UGL_UINT8 *           pDest; 
    UGL_UINT8 *           pMask;
    UGL_UINT8             mask;
    UGL_DDB *             pDdb;
    UGL_MDDB *            pMddb;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Allocate transparent bitmap for cursor */
    pCddb = (UGL_GEN_CDDB *) UGL_CALLOC (1, sizeof (UGL_GEN_CDDB));
    if (pCddb == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Initialize bitmap */
    dib.width       = pCdib->width;
    dib.height      = pCdib->height;
    dib.stride      = pCdib->width;
    dib.colorFormat = UGL_ARGB8888;
    dib.clutSize    = pCdib->clutSize + 1;
    dib.imageFormat = UGL_INDEXED_8;

    /* Allocate memory for color table */
    dib.pClut = (UGL_ARGB *) UGL_MALLOC (dib.clutSize * sizeof (UGL_ARGB));
    if (dib.pClut == UGL_NULL) {
        UGL_FREE (pCddb);
        return (UGL_NULL);
    }

    /* Copy color table data */
    memcpy (dib.pClut, pCdib->pClut, pCdib->clutSize * sizeof (UGL_ARGB));
    ((UGL_ARGB *) dib.pClut)[dib.clutSize - 1] =
        UGL_MAKE_ARGB (0x00, 0x00, 0x00, 0x00);

    /* Allocate storage for image data */
    dib.pData = (UGL_UINT8 *)
        UGL_MALLOC (pCdib->width * pCdib->height * sizeof (UGL_UINT8));
    if (dib.pData == UGL_NULL) {
        UGL_FREE (dib.pClut);
        UGL_FREE (pCddb);
        return (UGL_NULL);
    }

    /* Allocate memory for bitmask */
    pMdib = (UGL_MDIB *) UGL_CALLOC (1, sizeof (UGL_MDIB) +
                                     (pCdib->width + 7) / 8 * pCdib->height);
    if (pMdib == UGL_NULL) {
        UGL_FREE (dib.pData);
        UGL_FREE (dib.pClut);
        UGL_FREE (pCddb);
        return (UGL_NULL);
    }

    /* Setup monochrome bitmask data structure */
    pMdib->width  = pCdib->width;
    pMdib->height = pCdib->height;
    pMdib->stride = (pCdib->width + 7) / 8 * 8;
    pMdib->pData  = (UGL_UINT8 *) &pMdib[1];

    /* Setup source, destination and bitmask */
    pSrc  = pCdib->pData;
    pDest = dib.pData;
    pMask = pMdib->pData;

    /* Copy bitmap data */
    for (y = 0; y < pCdib->height; y++) {
        mask = 0x80;
        maskIndex = 0;

        for (x = 0; x < pCdib->width; x++) {
            if (pSrc[x] == UGL_CURSOR_COLOR_TRANSPARENT) {
                pDest[x] = (UGL_UINT8) (dib.clutSize - 1);
            }
            else if (pSrc[x] == UGL_CURSOR_COLOR_INVERT) {
                pDest[x] = (UGL_UINT8) (dib.clutSize - 1);
                pMask[maskIndex] |= mask;
            }
            else {
                pDest[x] = pSrc[x];
                pMask[maskIndex] |= mask;
            }

            /* Advance */
            if ((mask >>= 1) == 0x00) {
                mask = 0x80;
                maskIndex++;
            }
        }

        /* Advance */
        pSrc  += pCdib->stride;
        pDest += dib.stride;
        pMask += (pMdib->stride >> 3);
    }

    /* Create cursor image bitmap */
    pDdb = (*devId->bitmapCreate) (devId, &dib, UGL_DIB_INIT_DATA, 0, UGL_NULL);
    if (pDdb == UGL_NULL) {
        UGL_FREE (pMdib);
        UGL_FREE (dib.pData);
        UGL_FREE (dib.pClut);
        UGL_FREE (pCddb);
        return (UGL_NULL);
    }

    /* Create cursor monochrome bitmask */
    pMddb = (*devId->monoBitmapCreate) (devId, pMdib, UGL_DIB_INIT_DATA,
                                        0, UGL_NULL);
    if (pMddb == UGL_NULL) {
        (*devId->bitmapDestroy) (devId, pDdb);
        UGL_FREE (pMdib);
        UGL_FREE (dib.pData);
        UGL_FREE (dib.pClut);
        UGL_FREE (pCddb);
        return (UGL_NULL);
    }

    /* Create scratch bitmap for transparent blit */
    if (uglGenericScratchBitmapCreate (devId, pCdib->width,
                                       pCdib->height) == UGL_STATUS_ERROR) {
        (*devId->bitmapDestroy) (devId, pDdb);
        (*devId->monoBitmapDestroy) (devId, pMddb);
        UGL_FREE (pMdib);
        UGL_FREE (dib.pData);
        UGL_FREE (dib.pClut);
        UGL_FREE (pCddb);
        return (UGL_NULL);
    }

    /* Setup cursor bitmap structure */
    pCddb->tddb.header.width  = pCdib->width;
    pCddb->tddb.header.height = pCdib->height;
    pCddb->tddb.header.type   = UGL_TDDB_TYPE;
    pCddb->tddb.ddb           = pDdb;
    pCddb->tddb.mask          = pMddb;
    UGL_POINT_COPY (&pCddb->hotSpot, &pCdib->hotSpot);

    /* Free temporary buffers */
    UGL_FREE (pMdib);
    UGL_FREE (dib.pData);
    UGL_FREE (dib.pClut);

    return (UGL_CDDB_ID) pCddb;
}

/******************************************************************************
 *
 * uglGenericCursorBitmapDestroy - Destroy generic cursor bitmap
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglGenericCursorBitmapDestroy (
    UGL_DEVICE_ID  devId,
    UGL_CDDB_ID    cDdbId
    ) {
    UGL_GEN_CDDB  * pCddb;

    /* Get generic cursor bitmap */
    pCddb = (UGL_GEN_CDDB *) cDdbId;

    (*devId->transBitmapDestroy) (devId, (UGL_TDDB *) &pCddb->tddb);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorImageSet - Set cursor bitmap image
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorImageSet (
    UGL_DEVICE_ID  devId,
    UGL_CDDB *     pCddb
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Hide cursor */
    (*devId->cursorHide) (devId, UGL_NULL);

    /* Set cursor image */
    pCursorData->imageBitmap = (UGL_GEN_CDDB *) pCddb;

    /* Show cursor */
    (*devId->cursorShow) (devId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorImageGet - Get cursor bitmap image
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorImageGet (
    UGL_DEVICE_ID  devId,
    UGL_CDDB **    ppCddb
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Store image bitmap */
    *ppCddb = (UGL_CDDB *) pCursorData->imageBitmap;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorOn - Turn on cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorOn (
    UGL_DEVICE_ID  devId
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL || pCursorData->imageBitmap == UGL_NULL ||
        pCursorData->on == UGL_TRUE) {
        return (UGL_STATUS_ERROR);
    }

    /* Mark cursor as on but hidden so far */
    pCursorData->on     = UGL_TRUE;
    pCursorData->hidden = UGL_TRUE;

    /* Unhide cursor */
    (*devId->cursorShow) (devId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorOff - Turn off cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorOff (
    UGL_DEVICE_ID  devId
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL || pCursorData->on == UGL_FALSE) {
        return (UGL_STATUS_ERROR);
    }

    /* Hide cursor */
    (*devId->cursorHide) (devId, UGL_NULL);

    /* Mark cursor as off */
    pCursorData->on = UGL_FALSE;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorHide - Hide cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorHide (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pRect
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;
    UGL_GEN_CDDB *        pCddb;
    UGL_RECT              cursorRect;
    UGL_RECT              displayRect;
    UGL_RECT              srcRect;
    UGL_RECT              destRect;
    UGL_POINT             destPoint;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get current cursor image */
    pCddb = pCursorData->imageBitmap;

    if (pCursorData->hidden == UGL_TRUE || pCursorData->on == UGL_FALSE) {
        return (UGL_STATUS_OK);
    }

    /* Set geometry */
    cursorRect.left   = pCursorData->position.x - pCddb->hotSpot.x;
    cursorRect.top    = pCursorData->position.y - pCddb->hotSpot.y;
    cursorRect.right  = cursorRect.left + pCddb->tddb.header.width - 1;
    cursorRect.bottom = cursorRect.top + pCddb->tddb.header.height - 1;

    if (pRect != UGL_NULL) {
        UGL_RECT_INTERSECT (cursorRect, *pRect, destRect);
        if (UGL_RECT_WIDTH (destRect) <= 0 || UGL_RECT_HEIGHT (destRect) <= 0) {
            return (UGL_STATUS_OK);
        }
    }

    displayRect.left   = 0;
    displayRect.top    = 0;
    displayRect.right  = devId->pMode->width - 1;
    displayRect.bottom = devId->pMode->height - 1;

    UGL_RECT_INTERSECT (cursorRect, displayRect, destRect);

    if (UGL_RECT_WIDTH (destRect) <= 0 || UGL_RECT_HEIGHT (destRect) <= 0) {
        return (UGL_STATUS_OK);
    }

    srcRect.left   = 0;
    srcRect.top    = 0;
    UGL_RECT_MOVE_TO (srcRect, destRect.left - cursorRect.left,
                      destRect.top - cursorRect.top);
    UGL_RECT_SIZE_TO (srcRect, UGL_RECT_WIDTH (destRect),
                      UGL_RECT_HEIGHT (destRect));

    destPoint.x = destRect.left;
    destPoint.y = destRect.top;

    UGL_GC_SET (devId, pCursorData->gc);

    /* Erase mouse cursor by drawing screen bitmap */
    (*devId->bitmapBlt) (devId, pCursorData->screenBitmap, &srcRect,
                         UGL_DISPLAY_ID, &destPoint);

    /* Set cursor as hidden */
    pCursorData->hidden = UGL_TRUE;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorShow - Show cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorShow (
    UGL_DEVICE_ID  devId
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;
    UGL_GEN_CDDB *        pCddb;
    UGL_RECT              cursorRect;
    UGL_RECT              displayRect;
    UGL_RECT              srcRect;
    UGL_RECT              destRect;
    UGL_POINT             srcPoint;
    UGL_POINT             destPoint;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get current cursor image */
    pCddb = pCursorData->imageBitmap;

    if (pCursorData->hidden == UGL_FALSE || pCursorData->on == UGL_FALSE ||
        devId->batchCount > 0) {
        return (UGL_STATUS_OK);
    }

    /* Set geometry */
    cursorRect.left   = pCursorData->position.x - pCddb->hotSpot.x;
    cursorRect.top    = pCursorData->position.y - pCddb->hotSpot.y;
    cursorRect.right  = cursorRect.left + pCddb->tddb.header.width - 1;
    cursorRect.bottom = cursorRect.top + pCddb->tddb.header.height - 1;

    displayRect.left   = 0;
    displayRect.top    = 0;
    displayRect.right  = devId->pMode->width - 1;
    displayRect.bottom = devId->pMode->height - 1;

    UGL_RECT_INTERSECT (cursorRect, displayRect, destRect);

    if (UGL_RECT_WIDTH (destRect) <= 0 || UGL_RECT_HEIGHT (destRect) <= 0) {
        return (UGL_STATUS_OK);
    }

    srcRect.left   = 0;
    srcRect.top    = 0;
    UGL_RECT_MOVE_TO (srcRect, destRect.left - cursorRect.left,
                      destRect.top - cursorRect.top);
    UGL_RECT_SIZE_TO (srcRect, UGL_RECT_WIDTH (destRect),
                      UGL_RECT_HEIGHT (destRect));

    srcPoint.x = srcRect.left;
    srcPoint.y = srcRect.top;

    destPoint.x = destRect.left;
    destPoint.y = destRect.top;

    UGL_GC_SET (devId, pCursorData->gc);

    /* Store screen contents in order to erase cursor later */
    (*devId->bitmapBlt) (devId, UGL_DISPLAY_ID, &destRect,
                         pCursorData->screenBitmap, &srcPoint);

    /* Draw mouse cursor bitmap on screen */
    (*devId->transBitmapBlt) (devId, (UGL_TDDB *) &pCddb->tddb, &srcRect,
                              UGL_DISPLAY_ID, &destPoint);

    /* Set cursor as visible */
    pCursorData->hidden = UGL_FALSE;

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglGenericCursorMove - Move cursor
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglGenericCursorMove (
    UGL_DEVICE_ID  devId,
    UGL_POINT *    pCursorPos
    ) {
    UGL_GENERIC_DRIVER *  pDrv;
    UGL_GEN_CURSOR_DATA * pCursorData;
    UGL_DDB_ID            ddbId;
    UGL_GEN_CDDB *        pCddb;
    UGL_RECT              srcRect;
    UGL_POINT             destPoint;
    UGL_RECT              currRect;
    UGL_RECT              nextRect;
    UGL_RECT              intersectRect;

    /* Get generic driver */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get cursor data field */
    pCursorData = (UGL_GEN_CURSOR_DATA *) pDrv->pCursorData;
    if (pCursorData == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Check if cursor needs to be redrawn */
    if (pCursorData->on == UGL_TRUE && pCursorData->hidden == UGL_FALSE) {
        pCddb = pCursorData->imageBitmap;

        currRect.left   = pCursorData->position.x - pCddb->hotSpot.x;
        currRect.top    = pCursorData->position.x - pCddb->hotSpot.y;
        currRect.right  = currRect.left + pCddb->tddb.header.width - 1;
        currRect.bottom = currRect.top + pCddb->tddb.header.height - 1;

        UGL_RECT_COPY (&nextRect, &currRect);
        UGL_RECT_MOVE_TO (nextRect, pCursorPos->x - pCddb->hotSpot.x,
                          pCursorPos->y - pCddb->hotSpot.y);

        UGL_GC_SET (devId, pCursorData->gc);

        /* Check if next position overlaps current */
        UGL_RECT_INTERSECT (currRect, nextRect, intersectRect);
        if (UGL_RECT_WIDTH (intersectRect) > 0 &&
            UGL_RECT_HEIGHT (intersectRect) > 0) {

            /* Copy background to scratch bitmap */
            UGL_RECT_COPY (&srcRect, &nextRect);
            destPoint.x = 0;
            destPoint.y = 0;
            (*devId->bitmapBlt) (devId, UGL_DISPLAY_ID, &srcRect,
                                 pCursorData->scratchBitmap, &destPoint);

            /* Update changed portion of scratch bitmap */
            srcRect.left   = intersectRect.left - currRect.left;
            srcRect.top    = intersectRect.top - currRect.top;
            srcRect.right  = intersectRect.right - currRect.left;
            srcRect.bottom = intersectRect.bottom - currRect.top;
            destPoint.x = intersectRect.left - nextRect.left;
            destPoint.y = intersectRect.top - nextRect.top;
            (*devId->bitmapBlt) (devId, pCursorData->screenBitmap, &srcRect,
                                 pCursorData->scratchBitmap, &destPoint);

            /* Draw cursor image to screen bitmap */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = UGL_RECT_WIDTH (currRect) - 1;
            srcRect.bottom = UGL_RECT_HEIGHT (currRect) - 1;
            destPoint.x = nextRect.left - currRect.left;
            destPoint.y = nextRect.top - currRect.top;
            (*devId->transBitmapBlt) (
                devId, (UGL_TDDB *) &pCursorData->imageBitmap->tddb, &srcRect,
                (UGL_DDB *) pCursorData->screenBitmap, &destPoint);

            /* Draw cursor to screen */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = UGL_RECT_WIDTH (nextRect) - 1;
            srcRect.bottom = UGL_RECT_HEIGHT (nextRect) - 1;
            destPoint.x = nextRect.left;
            destPoint.y = nextRect.top;
            (*devId->transBitmapBlt) (
                devId, (UGL_TDDB *) &pCursorData->imageBitmap->tddb, &srcRect,
                UGL_DISPLAY_ID, &destPoint);

            /* Erase cursor at old position */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = UGL_RECT_WIDTH (currRect) - 1;
            srcRect.bottom = UGL_RECT_HEIGHT (currRect) - 1;
            destPoint.x = currRect.left;
            destPoint.y = currRect.top;
            (*devId->bitmapBlt) (devId, pCursorData->screenBitmap,
                                 &srcRect, UGL_DISPLAY_ID, &destPoint);

            /* Swap screen bitmap and scratch bitmap */
            ddbId = pCursorData->screenBitmap;
            pCursorData->screenBitmap = pCursorData->scratchBitmap;
            pCursorData->scratchBitmap = ddbId;
        }
        else {

            /* Copy background to scratch bitmap */
            UGL_RECT_COPY (&srcRect, &nextRect);
            destPoint.x = 0;
            destPoint.y = 0;
            (*devId->bitmapBlt) (devId, UGL_DISPLAY_ID, &srcRect,
                                 pCursorData->scratchBitmap, &destPoint);

            /* Draw cursor at new location on screen */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = pCursorData->imageBitmap->tddb.header.width - 1;
            srcRect.bottom = pCursorData->imageBitmap->tddb.header.height - 1;
            destPoint.x = nextRect.left;
            destPoint.y = nextRect.top;
            (*devId->transBitmapBlt) (
                devId, (UGL_TDDB *) &pCursorData->imageBitmap->tddb, &srcRect,
                UGL_DISPLAY_ID, &destPoint);

            /* Copy contents at current location of screen */
            srcRect.left   = 0;
            srcRect.top    = 0;
            srcRect.right  = pCursorData->imageBitmap->tddb.header.width - 1;
            srcRect.bottom = pCursorData->imageBitmap->tddb.header.height - 1;
            destPoint.x = currRect.left;
            destPoint.y = currRect.top;
            (*devId->bitmapBlt) (devId, pCursorData->screenBitmap, &srcRect,
                                 UGL_DISPLAY_ID, &destPoint);

            /* Swap screen bitmap and scratch bitmap */
            ddbId = pCursorData->screenBitmap;
            pCursorData->screenBitmap = pCursorData->scratchBitmap;
            pCursorData->scratchBitmap = ddbId;
        }
    }

    /* Update cursor position */
    UGL_POINT_COPY (&pCursorData->position, pCursorPos);

    return (UGL_STATUS_OK);
}

