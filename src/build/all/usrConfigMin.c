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

/* usrConfigMin.c - Minimal usrInit() user code */

#include <sys/types.h>
#include <vmx.h>
#include <drv/video/pvr.h>
#include <arch/intArchLib.h>
#include <drv/timer/dreamcast/timer.h>
#include <vmx/logLib.h>

void usrInit(void)
{
  int i, j;
  sysHwInit();
  setLogFlags(LOG_ARCH_LIB);
  setLogLevel(LOG_LEVEL_ERROR|LOG_LEVEL_WARNING|LOG_LEVEL_INFO|LOG_LEVEL_CALLS);
  for (i = 0; i < 50; i++) { for (j = 0; j < 0xfffff; j++); puts("."); }
  irq_shutdown();
}

