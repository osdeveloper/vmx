/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* sysLib.c - System dependent C code */

#include "config.h"

#include <sys/types.h>
#include <vmx.h>

#include <arch/sh/intArchLib.c>
#include <arch/sh/excArchLib.c>

#include <os/errnoLib.c>

#ifdef INCLUDE_VIDEO
#include <drv/video/pvr.c>
#endif

#ifdef   INCLUDE_LIBC
//#include <libc/stdio/puts.c>
//#include <libc/stdlib/itoa.c>
//#include <libc/stdlib/itox.c>
#include <libc/string/memset.c>
#include <libc/string/memcpy.c>
#include <libc/string/strlen.c>
#endif

/******************************************************************************
* sysHwInit - Initialize hardware
*
* RETURNS: N/A
******************************************************************************/

void sysHwInit(void)
{
#ifdef INCLUDE_VIDEO
  vid_init(vid_check_cable(), DM_640x480, PM_RGB565);
  vid_clear(0,0,0);
#endif
}

