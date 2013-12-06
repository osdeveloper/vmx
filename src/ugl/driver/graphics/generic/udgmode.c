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

/* udgmode.c - Universal graphics library generic mode */

#include <stdlib.h>
#include <ugl/ugl.h>

/******************************************************************************
 *
 * uglGenericModeFind- Find graphics mode
 *
 * RETURNS: Graphics mode index or UGL_STATUS_ERROR
 */

UGL_INT32 uglGenericModeFind(
    UGL_MODE *  pList,
    UGL_MODE *  pReqMode,
    UGL_UINT32  numModes
    ) {
    UGL_UINT32  i;
    int         drefreshRate;
    int         dminRefreshRate;
    UGL_INT32   modeIndex;

    /* Setup locals */
    dminRefreshRate = 0x1fffffff;
    modeIndex       = UGL_STATUS_ERROR;

    /* Loop tru all modes */
    for (i = 0; i < numModes; i++) {

        /* Check if mode is correct */
        if (pList[i].width == pReqMode->width &&
            pList[i].height == pReqMode->height &&
            pList[i].depth == pReqMode->depth &&
            pList[i].flags == pReqMode->flags) {

            /* Calculate refreshrate delta and store if smaller */
            drefreshRate = abs(pList[i].refreshRate - pReqMode->refreshRate);
            if (drefreshRate <= dminRefreshRate) {
                dminRefreshRate = drefreshRate;
                modeIndex       = i;
            }
        }
    }

    return (modeIndex);
}

