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

/* udcclr.c - Common color support */

#include <stdlib.h>
#include <string.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/common/udcclr.h>

/******************************************************************************
 *
 * uglCommonClutCreate - Create palette
 *
 * RETURNS: Pointer to clut
 */

UGL_CLUT * uglCommonClutCreate (
    UGL_SIZE numColors
    ) {
    UGL_INT32  i;
    UGL_CLUT * pClut;

    /* Allocate memory for struct */
    pClut = (UGL_CLUT *) UGL_CALLOC (1, sizeof (UGL_CLUT) +
                                     sizeof (UGL_CLUT_ENTRY) * numColors);
    if (pClut == NULL) {
        return (UGL_NULL);
    }

    /* Setup struct */
    pClut->numColors      = numColors;
    pClut->firstFreeIndex = 0;
    pClut->firstUsedIndex = -1;
    pClut->clut           = (UGL_CLUT_ENTRY *) &pClut[1];

    /* Initailize reference entries */
    for (i = 0; i < numColors; i++) {
        pClut->clut[i].nextIndex = i + 1;
        pClut->clut[i].prevIndex = i - 1;
    }

    pClut->clut[0].prevIndex             = -1;
    pClut->clut[numColors - 1].nextIndex = -1;

    return (pClut);
}

/******************************************************************************
 *
 * uglCommonClutSet - Set palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglCommonClutSet (
    UGL_CLUT * pClut,
    UGL_ORD    offset,
    UGL_ARGB * pColors,
    UGL_SIZE numColors
    ) {
    UGL_INT32        i;
    UGL_CLUT_ENTRY * pClutArray;

    if (pClut == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Check offset and color number */
    if (offset < 0 || numColors < 0 || 
        (offset + numColors) > pClut->numColors) {
        return (UGL_STATUS_ERROR);
    }

    /* Set entries */
    pClutArray = pClut->clut;
    for (i = 0; i < numColors; i++) {
        pClutArray[i + offset].color = pColors[i];
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglCommonClutGet - Get software palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS
 */

UGL_STATUS uglCommonClutGet (
    UGL_CLUT * pClut,
    UGL_ORD    offset,
    UGL_ARGB * pColors,
    UGL_SIZE   numColors
    ) {
    UGL_CLUT_ENTRY * pClutArray;
    UGL_INT32        i;

    if (pClut == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Store clut */
    pClutArray = pClut->clut;

    /* Get entries */
    for (i = 0; i < numColors; i++) {
        pColors[i] = pClutArray[i + offset].color;
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglCommonClutMapNearest - Map to nearest match
 *
 * RETURNS: UGL_STATUS_OK
 */

UGL_STATUS uglCommonClutMapNearest (
    UGL_CLUT *  pClut,
    UGL_ARGB *  pMapColors,
    UGL_ARGB *  pActualColors,
    UGL_COLOR * pUglColors,
    UGL_SIZE    numColors
    ) {
    UGL_INT32        i;
    UGL_INT32        j;
    UGL_INT32        nearIndex;
    UGL_CLUT_ENTRY * clut;
    UGL_ARGB         r;
    UGL_ARGB         g;
    UGL_ARGB         b;
    unsigned long    minError;
    unsigned long    rError;
    unsigned long    gError;
    unsigned long    bError;
    unsigned long    errorVal;

    /* Store clut and maximize minimum error */
    clut     = pClut->clut;
    minError = 0xffffffffL;

    /* Store all rgb components of mapped colors */
    for (i = 0; i < numColors; i++) {
        r = UGL_ARGB_RED (pMapColors[i]);
        g = UGL_ARGB_GREEN (pMapColors[i]);
        b = UGL_ARGB_BLUE (pMapColors[i]);

        /* Calculate all component error deltas */
        j = pClut->firstUsedIndex;
        while (j != -1) {
            rError = UGL_ARGB_RED (clut[j].color) - r;
            gError = UGL_ARGB_GREEN (clut[j].color) - g;
            bError = UGL_ARGB_BLUE (clut[j].color) - b;

            /* Calculate total error absoule value */
            errorVal = rError * rError +
                       gError * gError +
                       bError * bError;

            /* Check if this is a better match */
            if (errorVal < minError) {
                nearIndex = j;

                /* If this is a prefect match we are done */
                if (errorVal == 0) {
                    break;
                }

                /* Store new min error */
                minError = errorVal;
            }

            /* Move on to the next color entry to test */
            j = clut[j].nextIndex;
        }

        /* Store actual color */
        if (pActualColors != UGL_NULL) {
            pActualColors[i] = clut[nearIndex].color;
        }

        /* Store palette index */
        if (pUglColors != UGL_NULL) {
            pUglColors[i] = (UGL_COLOR) nearIndex;
        }
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglCommonClutAlloc - Allocate color
 *
 * RETURNS: UGL_FALSE if no need to allocate a new color else UGL_TRUE
 */

UGL_BOOL uglCommonClutAlloc (
    UGL_CLUT * pClut,
    UGL_ORD *  pIndex,
    UGL_ARGB   reqColor,
    UGL_ARGB * pActualColor
    ) {
    UGL_CLUT_ENTRY * clut;
    UGL_INT32        i;
    UGL_INT32        index;

    /* Get index and struct */
    index = *pIndex;
    clut  = pClut->clut;

    /* Ignore alpha componenet */
    reqColor |= 0xff000000;

    /* No (< 0) index specified */
    if (index < 0) {
        index = pClut->firstUsedIndex;

        /* Check if any existing color match */
        while (index >= 0) {
            if (clut[index].color == reqColor) {
                clut[index].useCount++;
                *pIndex = index;
                *pActualColor = reqColor;
                return (UGL_FALSE);
            }

            /* Move on to next color */
            index = clut[index].nextIndex;
        }

        /* No match found, allocate new */

        /* Check if a new entry place is avilable */
        if (pClut->firstFreeIndex != -1) {
            index = pClut->firstFreeIndex;
            pClut->firstFreeIndex = clut[index].nextIndex;

            /* If none set prev index */
            if (pClut->firstFreeIndex != -1) {
                clut[pClut->firstFreeIndex].prevIndex = -1;
            }

            /* Set clut next index */
            clut[index].nextIndex = pClut->firstUsedIndex;

            /* If none set prev index */
            if (pClut->firstUsedIndex != -1) {
                clut[pClut->firstUsedIndex].prevIndex = index;
            }

            /* Set clut first used index */
            pClut->firstUsedIndex = index;

            /* Setup color */
            clut[index].useCount = 1;
            clut[index].color    = reqColor;

            /* Setup store arguments */
            *pIndex       = index;
            *pActualColor = reqColor;

            return (UGL_TRUE);
        }
        else {
            uglCommonClutMapNearest(pClut, &reqColor, pActualColor,
                                    (UGL_COLOR *) &index, 1);

            clut[index].useCount++;
            *pIndex = index;

            return (UGL_FALSE);
        }
    }
    else {
        /* Check if no users */
        if (clut[index].useCount == 0) {

            /* Check previous index */
            i = clut[index].prevIndex;
            if (i >= 0) {
                clut[i].nextIndex = clut[index].nextIndex;
            }

            /* Check next index */
            i = clut[index].nextIndex;
            if (i >= 0) {
                clut[i].prevIndex = clut[index].prevIndex;
            }

            /* Check if it is the first free */
            if (index == pClut->firstFreeIndex) {
                pClut->firstFreeIndex = i;
            }

            clut[index].nextIndex = pClut->firstUsedIndex;

            if (pClut->firstUsedIndex != -1) {
                clut[pClut->firstUsedIndex].prevIndex = index;
            }

            /* Store index as first */
            pClut->firstUsedIndex = index;
        }

        /* Store in arguments */
        clut[index].color = reqColor;
        clut[index].useCount++;
        *pIndex       = index;
        *pActualColor = reqColor;

        return (UGL_TRUE);
    }

    /* Should not arrive here */
    return (UGL_FALSE);
}

/******************************************************************************
 *
 * uglCommonClutFree - Free color users
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCommonClutFree (
    UGL_CLUT *  pClut,
    UGL_COLOR * pColors,
    UGL_SIZE    numColors,
    UGL_COLOR   mask
    ) {
    UGL_CLUT_ENTRY * clut;
    UGL_INT32        i;
    UGL_INT32        index;
    UGL_INT32        nextIndex;
    UGL_INT32        prevIndex;
    UGL_STATUS       status;

    /* Get clut */
    clut = pClut->clut;

    /* Set initial status */
    status = UGL_STATUS_OK;

    /* Loop trough all colors */
    for (i = 0; i < numColors; i++) {
        index = (UGL_INT32) (pColors[i] & mask);

        /* Check if no users */
        if (clut[index].useCount == 0) {
            status = UGL_STATUS_ERROR;
        }
        else if (--clut[index].useCount == 0) {
            nextIndex = clut[index].nextIndex;
            prevIndex = clut[index].prevIndex;

            if (nextIndex >= 0) {
                clut[nextIndex].prevIndex = clut[index].prevIndex;
            }

            if (prevIndex >= 0) {
                clut[prevIndex].nextIndex = clut[index].nextIndex;
            }
            else {
                pClut->firstUsedIndex = clut[index].nextIndex;
            }

            if (pClut->firstUsedIndex == index) {
                pClut->firstUsedIndex = clut[index].nextIndex;
            }

            /* Update */
            clut[index].nextIndex = pClut->firstFreeIndex;
            clut[index].prevIndex = -1;
            pClut->firstFreeIndex = index;
        }
    }

    return (status);
}

/******************************************************************************
 *
 * uglCommonClutDestroy - Free palette
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglCommonClutDestroy(
    UGL_CLUT * pClut
    ) {

    if (pClut == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    UGL_FREE (pClut);

    return (UGL_STATUS_OK);
}

