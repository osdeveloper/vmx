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

/* udgmode.c - Universal graphics library generic */

#include <stdlib.h>
#include <ugl/ugl.h>

/*******************************************************************************
 * uglGenericModeFind- Find graphics mode
 *
 * RETURNS: Graphics mode index
 ******************************************************************************/

UGL_INT32 uglGenericModeFind(UGL_MODE *pList,
			     UGL_MODE *pReqMode,
			     UGL_UINT32 numModes)
{
  UGL_UINT32 i;
  int DrefreshRate;
  int DminRefreshRate;
  UGL_INT32 modeIndex;

  /* Setup locals */
  DminRefreshRate = 0x1fffffff;
  modeIndex = UGL_STATUS_ERROR;

  /* Loop tru all modes */
  for (i = 0; i < numModes; i++) {

    /* Check if mode is correct */
    if ( pList[i].width == pReqMode->width &&
	 pList[i].height == pReqMode->height &&
	 pList[i].depth == pReqMode->depth &&
	 pList[i].flags == pReqMode->flags) {

      /* Calculate refreshrate delta and store if smaller */
      DrefreshRate = abs(pList[i].refreshRate - pReqMode->refreshRate);
      if (DrefreshRate <= DminRefreshRate) {
        DminRefreshRate = DrefreshRate;
        modeIndex = i;

      } /* End delta lower */

    } /* End correct mode */

  } /* End all modes */

  return modeIndex;
}

