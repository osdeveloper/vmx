/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2014 Surplus Users Ham Society
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

/* uglccube.c - Color cube support for Universal Graphics Library */

#include "ugl.h"
#include "uglclr.h"
#include "uglugi.h"

/******************************************************************************
 *
 * uglColorCubeCreate - Create color cube without color allocation
 *
 * RETURNS: Pointer to color cube or UGL_NULL
 */

UGL_COLOR_CUBE * uglColorCubeCreate (
    UGL_ORD  nRed,
    UGL_ORD  nGreen,
    UGL_ORD  nBlue
    ) {
    UGL_COLOR_CUBE * pCube;
    UGL_INT32        i;
    UGL_INT32        j;
    UGL_INT32        k;
    UGL_INT32        nColors;
    UGL_INT32        index;

    /* Caclucate number of colors */
    nColors = nRed * nGreen * nBlue;

    if (nColors == 0) {
        return (UGL_NULL);
    }

    pCube = (UGL_COLOR_CUBE *) UGL_CALLOC (1, sizeof (UGL_COLOR_CUBE) +
                                           sizeof (UGL_ARGB) * nColors +
                                           sizeof (UGL_COLOR) * nColors +
                                           sizeof (UGL_ARGB) * nColors);
    if (pCube == UGL_NULL) {
        return (UGL_NULL);
    }

    /* Setup pointers */
    pCube->pArgbArray       = (UGL_ARGB *) &pCube[1];
    pCube->pUglColorArray   = (UGL_COLOR *) &pCube->pArgbArray[nColors];
    pCube->pActualArgbArray = (UGL_ARGB *) &pCube->pUglColorArray[nColors];
    pCube->nRedColors       = nRed;
    pCube->nGreenColors     = nGreen;
    pCube->nBlueColors      = nBlue;
    pCube->arraySize        = nColors;

    index = 0;
    for (i = 0; i < nRed; i++) {
        for (j = 0; j < nGreen; j++) {
            for (k = 0; k < nBlue; k++) {
                pCube->pArgbArray[index] =
                    UGL_MAKE_RGB (((i * 255) / (nRed - 1)),
                                  ((j * 255) / (nGreen - 1)),
                                  ((k * 255) / (nBlue - 1)));
                index++;
            }
        }
    }

    return (pCube);
}

/******************************************************************************
 *
 * uglColorCubeDestroy - Destroy color cube without color deallocation
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorCubeDestroy (
    UGL_COLOR_CUBE * pCube
    ) {

    if (pCube == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    UGL_FREE (pCube);

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglColorCubeAlloc - Allocate colors for color cube
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorCubeAlloc (
    UGL_DEVICE_ID    devId,
    UGL_COLOR_CUBE * pCube
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (devId == UGL_NULL || pCube == UGL_NULL || pCube->devId != UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Allocate colors */
    status = uglColorAllocExt (devId, pCube->pArgbArray, UGL_NULL,
                               pCube->pActualArgbArray, pCube->pUglColorArray,
                               pCube->arraySize);
    if (status != UGL_STATUS_ERROR) {
        pCube->devId = devId;
    }

    return (status);
}

/******************************************************************************
 *
 * uglColorCubeFree - Deallocate colors for color cube
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorCubeFree (
    UGL_COLOR_CUBE * pCube
    ) {
    UGL_STATUS  status;

    /* Check params */
    if (pCube == UGL_NULL || pCube->devId == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    status = uglColorFree (pCube->devId, pCube->pUglColorArray,
                           pCube->arraySize);

    return (status);
}

/******************************************************************************
 *
 * uglColorCubeLookupExt - Find nearst march for format using color cube
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorCubeLookupExt (
    UGL_COLOR_CUBE * pCube,
    UGL_ARGB *       pArgb,
    UGL_ARGB_SPEC *  pSpec,
    UGL_ARGB *       pUglColor,
    UGL_ARGB *       pActualArgb
    ) {
    UGL_UINT8  red;
    UGL_UINT8  green;
    UGL_UINT8  blue;
    UGL_INT32  index;

    /* Check params */
    if (pCube == UGL_NULL || pArgb == UGL_NULL || pUglColor == UGL_NULL) {
        return (UGL_STATUS_ERROR);
    }

    /* Get color components */
    red   = (UGL_UINT8) UGL_ARGB_RED (*pArgb);
    green = (UGL_UINT8) UGL_ARGB_GREEN (*pArgb);
    blue  = (UGL_UINT8) UGL_ARGB_BLUE (*pArgb);

    /* Calculate color index */
    index = ((((red * pCube->nRedColors) >> pSpec->nRedBits) *
                 (pCube->nGreenColors * pCube->nBlueColors)) +
             (((green * pCube->nGreenColors) >> pSpec->nGreenBits) *
                 (pCube->nBlueColors)) +
              ((blue * pCube->nBlueColors) >> pSpec->nBlueBits));

    /* Get color if requested */
    if (pUglColor != UGL_NULL) {
        *pUglColor = pCube->pUglColorArray[index];
    }

    /* Store actual color if requested */
    if (pActualArgb != UGL_NULL) {
        *pActualArgb = pCube->pActualArgbArray[index];
    }

    return (UGL_STATUS_OK);
}

/******************************************************************************
 *
 * uglColorCubeLookup - Find nearst march using color cube
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglColorCubeLookup (
    UGL_COLOR_CUBE * pCube,
    UGL_ARGB *       pArgb,
    UGL_ARGB *       pUglColor,
    UGL_ARGB *       pActualArgb
    ) {
    UGL_STATUS     status;
    UGL_ARGB_SPEC  spec;

    spec.nRedBits   = 8;
    spec.nGreenBits = 8;
    spec.nBlueBits  = 8;

    status = uglColorCubeLookupExt (pCube, pArgb, &spec,
                                    pUglColor, pActualArgb);

    return (status);
}

