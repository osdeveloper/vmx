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

/* udgenclp.c - Generic clipping routines */

#include "ugl.h"
#include "driver/graphics/generic/udgen.h"

/******************************************************************************
 *
 * uglGenericClipDdbToDbd - Generic clip of device dependent bitmap
 *
 * RETURNS: UGL_TRUE or UGL_FALSE
 */

UGL_STATUS uglGenericClipDdbToDdb (
    UGL_DEVICE_ID  devId,
    UGL_RECT *     pClipRect,
    UGL_BMAP_ID *  pSrcBmpId,
    UGL_RECT *     pSrcRect,
    UGL_BMAP_ID *  pDestBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_GENERIC_DRIVER * pDrv;
    UGL_GC_ID            gc;
    UGL_POS              x;
    UGL_POS              y;
    UGL_RECT             srcRect;
    UGL_RECT             srcClip;
    UGL_RECT             destRect;
    UGL_RECT             destClip;

    /* Get driver first in device struct */
    pDrv = (UGL_GENERIC_DRIVER *) devId;

    /* Get graphics context */
    gc = pDrv->gc;

    /* Setup geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    destRect.top   = pDestPoint->y;
    destRect.left  = pDestPoint->x;
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));

    /* Setup source clip rect */
    if ((UGL_DDB_ID) *pSrcBmpId == UGL_DEFAULT_ID) {
        *pSrcBmpId = (UGL_BMAP_ID) gc->pDefaultBitmap;
        UGL_RECT_COPY (&srcClip, &gc->boundRect);
    }
    else if ((UGL_DDB_ID) *pSrcBmpId == UGL_DISPLAY_ID) {
        srcClip.top    = 0;
        srcClip.bottom = devId->pMode->height - 1;
        srcClip.left   = 0;
        srcClip.right  = devId->pMode->width - 1;
    }
    else {
        srcClip.top    = 0;
        srcClip.bottom = (*pSrcBmpId)->header.height - 1;
        srcClip.left   = 0;
        srcClip.right  = (*pSrcBmpId)->header.width - 1;
    }

    /* Set dest clip rect */
    if ((UGL_DDB_ID) *pDestBmpId == UGL_DEFAULT_ID) {
        *pDestBmpId = (UGL_BMAP_ID) gc->pDefaultBitmap;
        UGL_RECT_COPY (&destClip, pClipRect);
    }
    else if ((UGL_DDB_ID) *pDestBmpId == UGL_DISPLAY_ID) {
        destClip.top    = 0;
        destClip.bottom = devId->pMode->height - 1;
        destClip.left   = 0;
        destClip.right  = devId->pMode->width - 1;
    }
    else {
        destClip.top    = 0;
        destClip.bottom = (*pDestBmpId)->header.height - 1;
        destClip.left   = 0;
        destClip.right  = (*pDestBmpId)->header.width - 1;
    }

    /* Get source starting position and clip to source */
    x = srcRect.left;
    y = srcRect.top;
    UGL_RECT_INTERSECT (srcRect, srcClip, srcRect);
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));
    UGL_RECT_MOVE (destRect, srcRect.left - x, srcRect.top - y);

    /* Get destination starting point and clip to destination */
    x = destRect.left;
    y = destRect.top;
    UGL_RECT_INTERSECT (destRect, destClip, destRect);
    UGL_RECT_SIZE_TO (srcRect, UGL_RECT_WIDTH (destRect),
                      UGL_RECT_HEIGHT(destRect));
    UGL_RECT_MOVE (srcRect, destRect.left - x, destRect.top - y);

    /* Copy the resulting geometry */
    UGL_RECT_COPY (pSrcRect, &srcRect);
    pDestPoint->x = destRect.left;
    pDestPoint->y = destRect.top;

    /* Check result */
    if (UGL_RECT_WIDTH (destRect) <= 0 || UGL_RECT_HEIGHT (destRect) <= 0) {
        return (UGL_FALSE);
    }

    return (UGL_TRUE);
}

/******************************************************************************
 *
 * uglGenericClipDibToDdb - Generic clip of device independent to dependent
 *
 * RETURNS: UGL_TRUE or UGL_FALSE
 */

UGL_BOOL uglGenericClipDibToDdb (
    UGL_DEVICE_ID  devId,
    UGL_DIB *      pDib,
    UGL_RECT *     pSrcRect,
    UGL_BMAP_ID *  pBmpId,
    UGL_POINT *    pDestPoint
    ) {
    UGL_POS              x;
    UGL_POS              y;
    UGL_RECT             srcRect;
    UGL_RECT             srcClip;
    UGL_RECT             destRect;
    UGL_RECT             destClip;

    /* Setup geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    destRect.top   = pDestPoint->y;
    destRect.left  = pDestPoint->x;
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));

    /* Setup source clip rect */
    srcClip.top    = 0;
    srcClip.bottom = pDib->height - 1;
    srcClip.left   = 0;
    srcClip.right  = pDib->width - 1;

    /* Setup dest clip rect */
    if ((UGL_DDB_ID) *pBmpId == UGL_DISPLAY_ID) {
        destClip.top    = 0;
        destClip.bottom = devId->pMode->height - 1;
        destClip.left   = 0;
        destClip.right  = devId->pMode->width - 1;
    }
    else {
      destClip.top    = 0;
      destClip.bottom = (*pBmpId)->header.height - 1;
      destClip.left   = 0;
      destClip.right  = (*pBmpId)->header.width - 1;
    }

    /* Clip to source */
    UGL_RECT_INTERSECT (srcRect, srcClip, srcRect);
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));

    /* Clip to destination */
    x = destRect.left;
    y = destRect.top;
    UGL_RECT_INTERSECT (destRect, destClip, destRect);
    UGL_RECT_SIZE_TO (srcRect, UGL_RECT_WIDTH (destRect),
                      UGL_RECT_HEIGHT (destRect));
    UGL_RECT_MOVE (srcRect, destRect.left - x, destRect.top - y);

    /* Copy the resulting geometry */
    UGL_RECT_COPY (pSrcRect, &srcRect);
    pDestPoint->x = destRect.left;
    pDestPoint->y = destRect.top;

    /* Check result */
    if (UGL_RECT_WIDTH (destRect) <= 0 || UGL_RECT_HEIGHT (destRect) <= 0) {
        return (UGL_FALSE);
    }

    return (UGL_TRUE);
}

/******************************************************************************
 *
 * uglGenericClipDdbToDib - Generic clip of device dependent to independent
 *
 * RETURNS: UGL_TRUE or UGL_FALSE
 */

UGL_BOOL uglGenericClipDdbToDib (
    UGL_DEVICE_ID  devId,
    UGL_BMAP_ID *  pBmpId,
    UGL_RECT *     pSrcRect,
    UGL_DIB *      pDib,
    UGL_POINT *    pDestPoint
    ) {
    UGL_POS              x;
    UGL_POS              y;
    UGL_RECT             srcRect;
    UGL_RECT             srcClip;
    UGL_RECT             destRect;
    UGL_RECT             destClip;

    /* Setup geometry */
    UGL_RECT_COPY (&srcRect, pSrcRect);
    destRect.top   = pDestPoint->y;
    destRect.left  = pDestPoint->x;
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));

    /* Setup destination clip rect */
    destClip.top    = 0;
    destClip.bottom = pDib->height - 1;
    destClip.left   = 0;
    destClip.right  = pDib->width - 1;

    /* Setup source clip rect */
    if ((UGL_DDB_ID) *pBmpId == UGL_DISPLAY_ID) {
        srcClip.top    = 0;
        srcClip.bottom = devId->pMode->height - 1;
        srcClip.left   = 0;
        srcClip.right  = devId->pMode->width - 1;
    }
    else {
      srcClip.top    = 0;
      srcClip.bottom = (*pBmpId)->header.height - 1;
      srcClip.left   = 0;
      srcClip.right  = (*pBmpId)->header.width - 1;
    }

    /* Clip to source */
    UGL_RECT_INTERSECT (srcRect, srcClip, srcRect);
    UGL_RECT_SIZE_TO (destRect, UGL_RECT_WIDTH (srcRect),
                      UGL_RECT_HEIGHT (srcRect));

    /* Clip to destination */
    x = destRect.left;
    y = destRect.top;
    UGL_RECT_INTERSECT (destRect, destClip, destRect);
    UGL_RECT_SIZE_TO (srcRect, UGL_RECT_WIDTH (destRect),
                      UGL_RECT_HEIGHT (destRect));
    UGL_RECT_MOVE (srcRect, destRect.left - x, destRect.top - y);

    /* Copy the resulting geometry */
    UGL_RECT_COPY (pSrcRect, &srcRect);
    pDestPoint->x = destRect.left;
    pDestPoint->y = destRect.top;

    /* Check result */
    if (UGL_RECT_WIDTH (destRect) <= 0 || UGL_RECT_HEIGHT (destRect) <= 0) {
        return (UGL_FALSE);
    }

    return (UGL_TRUE);
}

