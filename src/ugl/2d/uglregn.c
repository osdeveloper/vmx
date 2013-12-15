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

#include <stdio.h>
#include <ugl/ugl.h>

/* Defines */

#define UGL_REGION_RECTS_IN_BLOCK       50

/* Types */

typedef struct ugl_region_rect {
    UGL_RECT                 rect;
    struct ugl_region_rect * pNextTL2BR;
    struct ugl_region_rect * pPrevTL2BR;
    struct ugl_region_rect * pNextTR2BL;
    struct ugl_region_rect * pPrevTR2BL;
} UGL_REGION_RECT;

typedef struct ugl_region {
    UGL_REGION_RECT * pFirstTL2BR;
    UGL_REGION_RECT * pLastTL2BR;
    UGL_REGION_RECT * pFirstTR2BL;
    UGL_REGION_RECT * pLastTR2BL;
} UGL_REGION;

typedef struct ugl_region_block {
    UGL_REGION_RECT *         pRectBlock;
    struct ugl_region_block * pNextBlock;
} UGL_REGION_BLOCK;

/* Locals */

UGL_LOCAL UGL_INT32          numRegions          = 0;
UGL_LOCAL UGL_LOCK_ID        uglRegionLockId     = UGL_NULL;
UGL_LOCAL UGL_REGION_RECT *  pFreeRegionRectHead = UGL_NULL;
UGL_LOCAL UGL_REGION_BLOCK * pRegionBlockHead    = UGL_NULL;

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

UGL_LOCAL UGL_VOID uglRegionRectRemove (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRegionRect
    );

/* Functions */

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
    if (uglRegionInit ((UGL_REGION_ID) pRegion) != UGL_STATUS_OK) {
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
    uglOSTaskUnlock ();

    /* If no more regions */
    if (num == 0) {
        pRegionBlock = pRegionBlockHead;
        pFreeRegionRectHead = UGL_NULL;
        pRegionBlockHead    = UGL_NULL;

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
 * uglRegionIsEmpty - Check if region is empty
 *
 * RETURNS: UGL_TRUE or UGL_FALSE
 */

UGL_STATUS uglRegionIsEmpty (
    const UGL_REGION_ID  regionId
    ) {
    UGL_REGION * pRegion;

    if (regionId == UGL_NULL) {
        return (UGL_TRUE);
    }

    pRegion = (UGL_REGION *) regionId;
    if (pRegion->pFirstTL2BR == UGL_NULL) {
        return (UGL_TRUE);
    }

   return (UGL_FALSE);
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
    UGL_REGION * pRegion;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    pRegion = (UGL_REGION *) regionId;

    if (pRegion->pLastTL2BR != UGL_NULL) {
        uglOSLock (uglRegionLockId);

        pRegion->pLastTL2BR->pNextTL2BR = pFreeRegionRectHead;
        pFreeRegionRectHead = pRegion->pFirstTL2BR;
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
 * uglRegionCopy - Copy region from source to destination
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionCopy (
    const UGL_REGION_ID  srcRegionId,
    UGL_REGION_ID        destRegionId
    ) {
    UGL_REGION *      pSrcRegion;
    UGL_REGION_RECT * pRegionRect;

    /* Check params */
    if (srcRegionId == UGL_NULL || destRegionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Clear destination */
    uglRegionEmpty (destRegionId);

    pSrcRegion  = (UGL_REGION *) srcRegionId;

    /* Check if anything to copy */
    pRegionRect = pSrcRegion->pFirstTL2BR;
    if (pRegionRect == UGL_NULL) {
        return (UGL_STATUS_OK);
    }

    /* Copy rectangle list */
    while (pRegionRect != UGL_NULL) {
        uglRegionRectAdd (destRegionId, &pRegionRect->rect);
        pRegionRect = pRegionRect->pNextTL2BR;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionMove - Move region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionMove (
    const UGL_REGION_ID  regionId,
    const UGL_ORD        dx,
    const UGL_ORD        dy
    ) {
    UGL_REGION *      pRegion;
    UGL_REGION_RECT * pRegionRect;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Move all rectangles */
    pRegionRect = pRegion->pFirstTL2BR;
    while (pRegionRect != UGL_NULL) {
        UGL_RECT_MOVE (pRegionRect->rect, dx, dy);
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
    UGL_REGION_ID    regionId,
    const UGL_RECT * pRect
    ) {
    UGL_REGION_RECT * pRegionRect;
    UGL_REGION *      pRegion;

    if (regionId == UGL_NULL || pRect == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    uglOSLock (uglRegionLockId);
    if (pFreeRegionRectHead == UGL_NULL) {
        if (uglRegionBlockAlloc () != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }
    }

    /* Insert */
    pRegionRect = pFreeRegionRectHead;
    pFreeRegionRectHead = pRegionRect->pNextTL2BR;
    uglOSUnlock (uglRegionLockId);

    /* Set geometry from rectangle */
    UGL_RECT_COPY (&pRegionRect->rect, pRect);

    pRegion = (UGL_REGION *) regionId;

    /* Add to lists */
    uglRegionRectAddTL2BR (pRegion, pRegionRect);
    uglRegionRectAddTR2BL (pRegion, pRegionRect);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionRectInclude - Include rectangle in region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionRectInclude (
    UGL_REGION_ID     regionId,
    const UGL_RECT  * pRect
    ) {
    UGL_REGION *      pRegion;
    UGL_REGION_RECT * pRegionRect;
    UGL_RECT          includeRect;
    UGL_RECT          includeRect2;
    UGL_RECT          intersectRect;
    UGL_POS           coord;

    /* Check params */
    if (regionId == UGL_NULL || pRect == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Store internal variables */
    pRegion = (UGL_REGION *) regionId;
    UGL_RECT_COPY (&includeRect, pRect);

    /* Check if trivial */
    if (UGL_RECT_WIDTH (includeRect) <= 0 ||
        UGL_RECT_HEIGHT (includeRect) <= 0) {
        return (UGL_STATUS_OK);
    }

    /* Loop to add intermediate rectangles */
    pRegionRect = pRegion->pFirstTL2BR;
    while (pRegionRect != UGL_NULL) {

        /* Check trivial for intersections */
        if (pRect->bottom < pRegionRect->rect.top &&
            pRect->right <= pRegionRect->rect.right) {
            break;
        }

        /* Create intersection rectangle */
        UGL_RECT_INTERSECT (*pRect, pRegionRect->rect, intersectRect);

        /* Check if creating new rectangles around intersection */
        if (intersectRect.right >= intersectRect.left &&
            intersectRect.bottom >= intersectRect.top) {

            /* Check if rectangle shall be added above intersection */
            if (intersectRect.top != includeRect.top) {
                includeRect.bottom = intersectRect.top - 1;

                if (uglRegionRectInclude (regionId,
                                          &includeRect) != UGL_STATUS_OK) {
                    return (UGL_STATUS_ERROR);
                }

                includeRect.top    = intersectRect.top;
                includeRect.bottom = pRect->bottom;
            }

            /* Check if rectangle shall be added below intersection */
            if (intersectRect.bottom != includeRect.bottom) {
                coord = includeRect.top;
                includeRect.top = intersectRect.bottom + 1;

                if (uglRegionRectInclude (regionId,
                                          &includeRect) != UGL_STATUS_OK) {
                    return (UGL_STATUS_ERROR);
                }

                includeRect.top    = coord;
                includeRect.bottom = intersectRect.bottom;
            }

            /* Check if rectangle shall be added to the left of intersection */
            if (intersectRect.left != includeRect.left) {
                includeRect.right = intersectRect.left - 1;

                if (uglRegionRectInclude (regionId,
                                          &includeRect) != UGL_STATUS_OK) {
                    return (UGL_STATUS_ERROR);
                }

                includeRect.left  = intersectRect.left;
                includeRect.right = pRect->right;
            }

            /* Check if rectangle shall be added to the right of intersection */
            if (intersectRect.right != includeRect.right) {
                includeRect.left = intersectRect.right + 1;

                if (uglRegionRectInclude (regionId,
                                          &includeRect) != UGL_STATUS_OK) {
                    return (UGL_STATUS_ERROR);
                }
            }

            /* Done */
            return (UGL_STATUS_OK);
        }
        else {

            /* Check if included rectangle can be combined with extisting */
            if (((includeRect.top == pRegionRect->rect.top ||
                  includeRect.bottom == pRegionRect->rect.bottom) &&
                 (includeRect.left == pRegionRect->rect.right + 1 ||
                  includeRect.right == pRegionRect->rect.left - 1)) ||
                (includeRect.left == pRegionRect->rect.left &&
                 includeRect.right == pRegionRect->rect.right &&
                 (includeRect.top == pRegionRect->rect.top + 1 ||
                  includeRect.bottom == pRegionRect->rect.top - 1))) {

                /* Remove rectangle beeing combined with */
                uglRegionRectRemove (pRegion, pRegionRect);

                if (includeRect.top == pRegionRect->rect.top &&
                    includeRect.bottom == pRegionRect->rect.bottom) {

                    if (intersectRect.left > pRegionRect->rect.right) {
                        includeRect.left = pRegionRect->rect.left;
                    }
                    else {
                        includeRect.right = pRegionRect->rect.right;
                    }
                }
                else if (includeRect.top == pRegionRect->rect.top) {

                    if (intersectRect.left < pRegionRect->rect.left) {
                        UGL_RECT_COPY (&includeRect2, &pRegionRect->rect);
                    }
                    else {
                        UGL_RECT_COPY (&includeRect, &pRegionRect->rect);
                        UGL_RECT_COPY (&includeRect2, &includeRect);
                    }

                    if (includeRect.bottom < includeRect2.bottom) {
                        includeRect.right = includeRect2.right;
                        includeRect2.top = includeRect.bottom + 1;
                    }
                    else {
                        includeRect2.left = includeRect.left;
                        includeRect.top = includeRect2.bottom + 1;
                    }

                    /* Add extra rectangle to region */
                    if (uglRegionRectInclude (regionId,
                                              &includeRect2) != UGL_STATUS_OK) {
                        return (UGL_STATUS_ERROR);
                    }
                }
                else if (includeRect.bottom == pRegionRect->rect.bottom) {

                    if (includeRect.left < pRegionRect->rect.left) {
                        UGL_RECT_COPY (&includeRect2, &pRegionRect->rect);
                    }
                    else {
                        UGL_RECT_COPY (&includeRect, &pRegionRect->rect);
                        UGL_RECT_COPY (&includeRect2, &includeRect);
                    }

                    if (includeRect.top > includeRect2.top) {
                        includeRect.right = includeRect2.right;
                        includeRect2.bottom = includeRect.top - 1;
                    }
                    else {
                        includeRect2.left = includeRect.left;
                        includeRect.bottom = includeRect2.top - 1;
                    }

                    /* Add extra rectangle to region */
                    if (uglRegionRectInclude (regionId,
                                              &includeRect2) != UGL_STATUS_OK) {
                        return (UGL_STATUS_ERROR);
                    }
                }
                else {

                    if (includeRect.top > pRegionRect->rect.bottom) {
                        includeRect.top = pRegionRect->rect.top;
                    }
                    else {
                        includeRect.bottom = pRegionRect->rect.bottom;
                    }
                }

                /* Add rectangle and done */
                return uglRegionRectInclude (regionId, &includeRect);
            }

            /* Advance */
            pRegionRect = pRegionRect->pNextTL2BR;
        }
    }

    /* Add rectangle and done */
    return uglRegionRectAdd (regionId, &includeRect);
}

/******************************************************************************
 *
 * uglRegionIntersect - Calculate intersection between two regions
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionIntersect (
    UGL_REGION_ID  regionAId,
    UGL_REGION_ID  regionBId,
    UGL_REGION_ID  intersectRegionId
    ) {
    UGL_REGION *      pRegionA;
    UGL_REGION *      pRegionB;
    UGL_REGION        region;
    UGL_REGION_RECT * pRegionRectA;
    UGL_REGION_RECT * pRegionRectB;
    UGL_RECT          intersectRect;

    /* Check params */
    if (regionAId == UGL_NULL || regionBId == UGL_NULL ||
        intersectRegionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    pRegionA = (UGL_REGION *) regionAId;
    pRegionB = (UGL_REGION *) regionBId;

    /* Initialize local region variable */
    uglRegionInit (&region);

    /* Check if same as any of input regions */
    if (regionAId == intersectRegionId || regionBId == intersectRegionId) {
        if (uglRegionCopy (intersectRegionId, &region) != UGL_STATUS_OK) {
            return (UGL_STATUS_ERROR);
        }

        if (regionAId == intersectRegionId) {
            pRegionA = &region;
        }
        else {
            pRegionB = &region;
        }
    }

    /* Clear destination */
    uglRegionEmpty (intersectRegionId);

    /* Add all intersection rectangles */
    pRegionRectA = pRegionA->pFirstTL2BR;
    while (pRegionRectA != UGL_NULL) {

        pRegionRectB = pRegionB->pFirstTL2BR;
        while (pRegionRectB != UGL_NULL) {

            /* Check if trivial */
            if (pRegionRectA->rect.bottom < pRegionRectB->rect.top &&
                pRegionRectA->rect.right <= pRegionRectB->rect.right) {
                break;
            }

            /* Calculate intersection */
            UGL_RECT_INTERSECT (pRegionRectA->rect, pRegionRectB->rect,
                                intersectRect);

            if (intersectRect.right >= intersectRect.left &&
                intersectRect.bottom >= intersectRect.top) {

                /* Add intersection rectangle to region */
                if (uglRegionRectInclude (intersectRegionId,
                                          &intersectRect) != UGL_STATUS_OK) {
                }
            }

            /* Advance */
            pRegionRectB = pRegionRectB->pNextTL2BR;
        }

        /* Advance */
        pRegionRectA = pRegionRectA->pNextTL2BR;
    }

    /* Remove local region variable */
    uglRegionDeinit (&region);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglRegionShow - Display region info as text to console
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglRegionShow (
    UGL_REGION_ID  regionId
    )
{
    UGL_REGION *      pRegion;
    UGL_REGION_RECT * pRegionRect;

    if (regionId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    pRegion = (UGL_REGION *) regionId;

    /* Show rectangle list */
    pRegionRect = pRegion->pFirstTL2BR;
    while (pRegionRect != UGL_NULL) {
        printf("Rectangle:\t%d\t%d\t%d\t%d\n",
               pRegionRect->rect.left, pRegionRect->rect.top,
               pRegionRect->rect.right, pRegionRect->rect.bottom);
        pRegionRect = pRegionRect->pNextTL2BR;
    }

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
    for (i = 0; i < UGL_REGION_RECTS_IN_BLOCK - 1; i++) {
        pRegionBlock->pRectBlock[i].pNextTL2BR =
            &pRegionBlock->pRectBlock[i + 1];
    }

    /* Insert */
    uglOSLock (uglRegionLockId);
    pRegionBlock->pRectBlock[i].pNextTL2BR = pFreeRegionRectHead;
    pFreeRegionRectHead = pRegionBlock->pRectBlock;
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

    /* Find next */
    pNextRect = pRegion->pFirstTL2BR;
    while (pNextRect != UGL_NULL && pRect->rect.top > pNextRect->rect.bottom) {
        pNextRect = pNextRect->pNextTL2BR;
    }

    while (pNextRect != UGL_NULL &&
           (pRect->rect.top > pNextRect->rect.bottom ||
            (pRect->rect.bottom >= pNextRect->rect.top &&
             pRect->rect.left > pNextRect->rect. right))) {
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
        if (pRect->rect.bottom > pNextRect->rect.top &&
            pRect->rect.left > pNextRect->rect.right) {
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

    /* Find next */
    pNextRect = pRegion->pFirstTR2BL;
    while (pNextRect != UGL_NULL && pRect->rect.top > pNextRect->rect.bottom) {
        pNextRect = pNextRect->pNextTR2BL;
    }

    while (pNextRect != UGL_NULL &&
           (pRect->rect.top > pNextRect->rect.bottom ||
            (pRect->rect.bottom >= pNextRect->rect.top &&
             pRect->rect.right < pNextRect->rect.left))) {
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
        if (pRect->rect.bottom > pNextRect->rect.top &&
            pRect->rect.right < pNextRect->rect.left) {
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

/******************************************************************************
 *
 * uglRegionRectRemove - Remove rectangle from region
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_VOID uglRegionRectRemove (
    UGL_REGION *      pRegion,
    UGL_REGION_RECT * pRegionRect
    ) {

    /* Remove from lists */
    uglRegionRectRemoveTL2BR (pRegion, pRegionRect);
    uglRegionRectRemoveTR2BL (pRegion, pRegionRect);

    /* Put on free list */
    uglOSLock (uglRegionLockId);
    pRegionRect->pNextTL2BR = pFreeRegionRectHead;
    pFreeRegionRectHead = pRegionRect;
    uglOSUnlock (uglRegionLockId);
}

