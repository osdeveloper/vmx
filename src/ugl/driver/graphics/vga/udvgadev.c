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

/* udvgadev.c - Vga graphics device */

#include <stdlib.h>

#include <ugl/ugl.h>
#include <ugl/driver/graphics/generic/udgen.h>

/*******************************************************************************
 * uglVgaDevDestroy - Free graphics device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 ******************************************************************************/

UGL_STATUS uglVgaDevDestroy(UGL_DEVICE_ID devId)
{
  /* Call os device deinit */
  if (uglUgiDevDeinit(devId) != UGL_STATUS_OK)
    return UGL_STATUS_ERROR;

  /* Destroy palette */
  uglGenericClutDestroy((UGL_GENERIC_DRIVER *) devId);

  /* Free memory */
  free(devId);

  return UGL_STATUS_OK;
}

