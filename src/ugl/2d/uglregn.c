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

/* uglregn.c - Universal graphics library region support */

#include <ugl/ugl.h>

/* Defines */

#define UGL_REGION_RECTS_IN_BLOCK       50

/* Locals */

UGL_LOCAL UGL_INT32          numRegions       = 0;
UGL_LOCAL UGL_LOCK_ID        uglRegionLockId  = UGL_NULL;
UGL_LOCAL UGL_REGION_RECT *  pFreeRectHead    = UGL_NULL;
UGL_LOCAL UGL_REGION_BLOCK * pRegionBlockHead = UGL_NULL;

UGL_LOCAL UGL_STATUS uglRegionBlockAlloc (
    void
    );

UGL_LOCAL UGL_VOID uglRegionRectAddTL2BR (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    );

UGL_LOCAL UGL_VOID uglRegionRectRemoveTL2BR (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    );

UGL_LOCAL UGL_VOID uglRegionRectAddTR2BL (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    );

UGL_LOCAL UGL_VOID uglRegionRectRemoveTR2BL (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    );

/******************************************************************************
 *
 * uglRegionCreate - Create a new region
 *
 * RETURNS: REGION_ID or UGL_NULL
 */

UGL_REGION_ID uglRegionCreate (
    void
    ) {
    UGL_REGION * pRegion;

    pRegion = (UGL_REGION *) UGL_MALLOC (sizeof (UGL_REGION));
    if (uglRegionInit (pRegion) != UGL_STATUS_OK) {
        UGL_FREE (pRegion);
        return (UGL_NULL);
    }

    return (pRegion);
}

/******************************************************************************
 *
 * uglRegionInit - Initialize region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionInit (
    UGL_REGION_ID  regionId
    ) {
    UGL_REGION * pRegion;
    UGL_INT32    num;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    pRegion = (UGL_REGION *) regionId;

    /* Setup region struct */
    pRegion->pFirstTL2BR = UGL_NULL;
    pRegion->pLastTL2BR  = UGL_NULL;
    pRegion->pFirstTR2BL = UGL_NULL;
    pRegion->pLastTR2BL  = UGL_NULL;

    /* Increase number of regions */
    uglOSTaskLock ();
    num = numRegions++;
    uglOSTaskUnlock ();

    if (num == 0) {
        while (uglRegionLockId != UGL_NULL) {
            uglOSTaskDelay (10);
        }

        uglRegionLockId = uglOSLockCreate ();
        if (uglRegionLockId == UGL_NULL) {
            return (UGL_STATUS_ERROR);
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionDeinit - Deinitialize region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionDeinit (
    UGL_REGION_ID  regionId
    ) {
    UGL_INT32          num;
    UGL_REGION_BLOCK * pRegionBlock;
    UGL_REGION_BLOCK * pFreeBlock;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Clear region */
    uglRegionEmpty (regionId);

    /* Decrease region counter */
    uglOSTaskLock ();
    num = --numRegions;
    ulgOSTaskUnlock ();

    /* If no more regions */
    if (num == 0) {
        pRegionBlock = pRegionBlockHead;
        pFreeRectHead    = UGL_NULL;
        pRegionBlockHead = UGL_NULL;

        while (pRegionBlock != UGL_NULL) {
            pFreeBlock = pRegionBlock;
            pRegionBlock = pRegionBlock->pNextBlock;
            UGL_FREE (pFreeBlock);
        }

        uglOSLockDestroy (uglRegionLockId);
        uglRegionLockId = UGL_NULL;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionDestroy - Destroy region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionDestroy (
    UGL_REGION_ID  regionId
    ) {
    UGL_REGION * pRegion;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    uglRegionDeinit (regionId);

    pRegion = (UGL_REGION *) regionId;
    UGL_FREE (pRegion);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionEmpty - Clear region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionEmpty (
    UGL_REGION_ID  regionId
    ) {
    UGL_REGION *       pRegion;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    pRegion = (UGL_REGION *) regionId;

    if (pRegion->pLastTL2BR != UGL_NULL) {
        uglOSLock (uglRegionLockId);

        pRegion->pLastTL2BR->pNextTL2BR = pFreeRectHead;
        pFreeRectHead = pRegion->pFirstTL2BR;
        pRegion->pFirstTL2BR = UGL_NULL;
        pRegion->pLastTL2BR  = UGL_NULL;
        pRegion->pFirstTR2BL = UGL_NULL;
        pRegion->pLastTR2BL  = UGL_NULL;

        uglOSUnlock (uglRegionLockId);
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionRectAdd - Add rectangle to region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionRectAdd (
    UGL_REGION_ID  regionId,
    UGL_RECT *     pRect
    ) {
    UGL_REGION_RECT * pRegionRect;
    UGL_REGION *      pRegion;

    if (regionId == UGL_NULL || pRect == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    uglOSLock (uglRegionLockId);
    if (pFreeRectHead == UGL_NULL) {
        if (uglRegionBlockAlloc () != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }
    }

    /* Insert */
    pRegionRect = pFreeRectHead;
    pFreeRectHead = pRegionRect->pNextTL2BR;
    uglOsUlock (uglRegionLockId);

    /* Set geometry from rectangle */
    pRegionRect->left   = pRect->left;
    pRegionRect->top    = pRect->top;
    pRegionRect->right  = pRect->right;
    pRegionRect->bottom = pRect->bottom;

    pRegion = (UGL_REGION *) regionId;

    /* Add to lists */
    uglRegionRectAddTL2BR (pRegion, pRegionRect);
    uglRegionRectAddTR2RL (pRegion, pRegionRect);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionBlockAlloc - Allocate region block
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglRegionBlockAlloc (
    void
    ) {
    UGL_INT32          i;
    UGL_REGION_BLOCK * pRegionBlock;

    pRegionBlock = (UGL_REGION_BLOCK *) UGL_MALLOC (sizeof (UGL_REGION_BLOCK) +
                                                    UGL_REGION_RECTS_IN_BLOCK *
                                                    sizeof (UGL_REGION_RECT));
    if (pRegionBlock == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    pRegionBlock->pRectBlock = (UGL_REGION_RECT *) &pRegionBlock[1];
    pRegionBlock->pNextBlock = UGL_NULL;

    /* Initialize all rects */
    for (i = 0; i < UGL_REGION_RECTS_IN_BLOCK; i++) {
        pRegionBlock->pRectBlock[i].pNextTL2BR =
            &pRegionBlock->pRectBlock[i + 1];
    }

    /* Insert */
    uglOSLock (uglRegionLockId);
    pRegionBlock->pRectBlock[i].pNextTL2BR = pFreeRectHead;
    pFreeRectHead = pRegionBlock->pRectBlock;
    pRegionBlock->pNextBlock = pRegionBlockHead;
    pRegionBlockHead = pRegionBlock;
    uglOSUnlock (uglRegionLockId);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionRectAddTL2BR -
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglRegionRectAddTL2BR (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    ) {
    UGL_REGION_RECT * pNextRect;
    UGL_REGION_RECT * pPrevRect;
    UGL_REGION_RECT * pStoreRect;

    /* Find */
    pNextRect = pRegion->pFirstTL2BR;
    while (pNextRect != UGL_NULL && pRect->top > pNextRect->bottom) {
        pNextRect = pNextRect->pNextTL2BR;
    }

    while (pNextRect != UGL_NULL && (pRect->top > pNextRect->bottom ||
                                     (pRect->bottom >= pNextRect->top &&
                                      pRect->left > pNextRect->right))) {
        pNextRect = pNextRect->pNextTL2BR;
    }

    /* Insert */
    if (pNextRect != UGL_NULL) {
        pPrevRect = pNextRect->pPrevTL2BR;
        pNextRect->pPrevTL2BR = pRect;
    }
    else {
        pPrevRect = pRegion->pLastTL2BR;
        pRegion->pLastTL2BR = pRect;
    }

    pRect->pNextTL2BR = pNextRect;
    pRect->pPrevTL2BR = pPrevRect;
    if (pPrevRect != UGL_NULL) {
        pPrevRect->pNextTL2BR = pRect;
    }
    else {
        pRegion->pFirstTL2BR = pRect;
    }

    /* Update sorting */
    while (pNextRect != UGL_NULL) {
        if (pRect->bottom > pNextRect->top &&
            pRect->left > pNextRect->right) {
            pStoreRect = pNextRect->pNextTL2BR;
            uglRegionRectRemoveTL2BR (pRegion, pNextRect);
            uglRegionRectAddTL2BR (pRegion, pNextRect);
            pNextRect = pStoreRect;
        }
        else {
            pNextRect = pNextRect->pNextTL2BR;
        }
    }
}

/******************************************************************************
 *
 * uglRegionRectRemoveTL2BR -
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglRegionRectRemoveTL2BR (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    ) {
    UGL_REGION_RECT * pNextRect;
    UGL_REGION_RECT * pPrevRect;

    pNextRect = pRect->pNextTL2BR;
    pPrevRect = pRect->pPrevTL2BR;

    if (pPrevRect != UGL_NULL) {
        pPrevRect->pNextTL2BR = pNextRect;
    }
    else {
        pRegion->pFirstTL2BR = pNextRect;
    }

    if (pNextRect != UGL_NULL) {
        pNextRect->pPrevTL2BR = pPrevRect;
    }
    else {
        pRegion->pLastTL2BR = pPrevRect;
    }
}

/******************************************************************************
 *
 * uglRegionRectAddTR2BL -
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglRegionRectAddTR2BL (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    ) {
    UGL_REGION_RECT * pNextRect;
    UGL_REGION_RECT * pPrevRect;
    UGL_REGION_RECT * pStoreRect;

    /* Find */
    pNextRect = pRegion->pFirstTR2BL;
    while (pNextRect != UGL_NULL && pRect->top > pNextRect->bottom) {
        pNextRect = pNextRect->pNextTR2BL;
    }

    while (pNextRect != UGL_NULL && (pRect->top > pNextRect->bottom ||
                                     (pRect->bottom >= pNextRect->top &&
                                      pRect->right < pNextRect->left))) {
        pNextRect = pNextRect->pNextTR2BL;
    }

    /* Insert */
    if (pNextRect != UGL_NULL) {
        pPrevRect = pNextRect->pPrevTR2BL;
        pNextRect->pPrevTR2BL = pRect;
    }
    else {
        pPrevRect = pRegion->pLastTR2BL;
        pRegion->pLastTR2BL = pRect;
    }

    pRect->pNextTR2BL = pNextRect;
    pRect->pPrevTR2BL = pPrevRect;
    if (pPrevRect != UGL_NULL) {
        pPrevRect->pNextTR2BL = pRect;
    }
    else {
        pRegion->pFirstTR2BL = pRect;
    }

    /* Update sorting */
    while (pNextRect != UGL_NULL) {
        if (pRect->bottom > pNextRect->top &&
            pRect->right < pNextRect->left) {
            pStoreRect = pNextRect->pNextTR2BL;
            uglRegionRectRemoveTR2BL (pRegion, pNextRect);
            uglRegionRectAddTR2BL (pRegion, pNextRect);
            pNextRect = pStoreRect;
        }
        else {
            pNextRect = pNextRect->pNextTR2BL;
        }
    }
}

/******************************************************************************
 *
 * uglRegionRectRemoveTR2BL -
 *
 * RETURNS: N/A
 */

UGL_LOCAL UGL_VOID uglRegionRectRemoveTR2BL (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRect
    ) {
    UGL_REGION_RECT * pNextRect;
    UGL_REGION_RECT * pPrevRect;

    pNextRect = pRect->pNextTR2BL;
    pPrevRect = pRect->pPrevTR2BL;

    if (pPrevRect != UGL_NULL) {
        pPrevRect->pNextTR2BL = pNextRect;
    }
    else {
        pRegion->pFirstTR2BL = pNextRect;
    }

    if (pNextRect != UGL_NULL) {
        pNextRect->pPrevTR2BL = pPrevRect;
    }
    else {
        pRegion->pLastTR2BL = pPrevRect;
    }
}

